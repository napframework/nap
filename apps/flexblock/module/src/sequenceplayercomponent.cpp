// Local includes
#include "sequenceplayercomponent.h"

// External Includes
#include <entity.h>
#include <utility/fileutils.h>
#include <rtti/jsonwriter.h>
#include <rtti/jsonreader.h>
#include <rtti/defaultlinkresolver.h>
#include <fstream>


//////////////////////////////////////////////////////////////////////////

// nap::flexblocksequenceplayer run time class definition 
RTTI_BEGIN_CLASS(nap::timeline::SequencePlayerComponent)
// Put additional properties here
RTTI_PROPERTY("TimelineParameters", &nap::timeline::SequencePlayerComponent::mParameterGroups, nap::rtti::EPropertyMetaData::Required)
RTTI_PROPERTY("StartShowFile", &nap::timeline::SequencePlayerComponent::mDefaultShow, nap::rtti::EPropertyMetaData::FileLink)
RTTI_PROPERTY("Flex Device", &nap::timeline::SequencePlayerComponent::mFlexDevice, nap::rtti::EPropertyMetaData::Required)
RTTI_PROPERTY("Frequency", &nap::timeline::SequencePlayerComponent::mFrequency, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

// nap::flexblocksequenceplayerInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::timeline::SequencePlayerComponentInstance)
RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////

namespace nap
{
	namespace timeline
	{
		void SequencePlayerComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
		{

		}


		void SequencePlayerComponentInstance::skipToSequence(const Sequence * sequence)
		{
			for (const auto* sequence_ : mSequenceContainer->getSequences())
			{
				if (sequence_ == sequence)
				{
					mTime = sequence->getStartTime();
					mCurrentSequenceIndex = 0;
					break;
				}
			}
		}

		SequencePlayerComponentInstance::~SequencePlayerComponentInstance()
		{
			stopThread();
		}

		bool SequencePlayerComponentInstance::init(utility::ErrorState& errorState)
		{
			SequencePlayerComponent* resource = getComponent<SequencePlayerComponent>();

			//
			mFrequency = resource->mFrequency;
			mFlexDevice = resource->mFlexDevice.get();

			//
			mSequenceContainer = std::make_unique<SequenceContainer>();
			if (!mSequenceContainer->init(errorState))
				return false;

			const auto& parameterGroups = resource->mParameterGroups;
			for (const auto& parameterGroup : parameterGroups)
			{
				for (const auto& parameter : parameterGroup->mParameters)
				{
					mParameters.emplace_back(parameter.get());
				}
			}

			if (!load(resource->mDefaultShow, errorState))
				return false;

			if( mSequenceContainer->getSequences().size() > 0 )
				mDuration = mSequenceContainer->getSequences().back()->getStartTime() + mSequenceContainer->getSequences().back()->getDuration();

			return true;
		}


		void SequencePlayerComponentInstance::onUpdate()
		{
			// Compute sleep time in microseconds 
			float sleep_time_microf = 1000.0f / static_cast<float>(mFrequency);
			long  sleep_time_micro = static_cast<long>(sleep_time_microf * 1000.0f);

			// declare input struct to copy parameters during computation
			SequencePlayerThreadInput input;

			while (mUpdateThreadRunning)
			{
				if (mIsPlaying)
				{
					// copy paramters
					{
						std::lock_guard<std::mutex> l(mThreadInputMutex);

						input.mCurrentSequenceIndex = mCurrentSequenceIndex;
						input.mIsFinished = mIsFinished;
						input.mIsLooping = mIsLooping;
						input.mIsPaused = mIsPaused;
						input.mIsPlaying = mIsPlaying;
						input.mSpeed = mSpeed;
						input.mTime = mTime;
						input.mDuration = mDuration;
					}

					// calc delta time
					auto now = std::chrono::steady_clock::now();
					auto elapsed = now - mBefore;
					float deltaTime = std::chrono::duration<float, std::milli>(elapsed).count() / 1000.0f;
					mBefore = now;

					// are we not paused ? then advance time
					if (!input.mIsPaused)
					{
						input.mTime += deltaTime * input.mSpeed;
					}

					// iterate trough sequences
					for (int i = 0; i < mSequenceContainer->getSequences().size(); i++)
					{
						// get result of process
						// -1 is we should move backwards ( time is smaller then start time of current sequence )
						// 1 is we should move forward ( time is bigger then start time + duration of current sequence )
						// 0 is we are in the right sequence ( time is bigger then start time and smaller then start time + duration of sequence )
						int result = mSequenceContainer->getSequences()[input.mCurrentSequenceIndex]->process(input.mTime, mParameters);

						if (result != 0)
						{
							// move backwards or forward in index according to result
							input.mCurrentSequenceIndex += result;

							size_t size = mSequenceContainer->getSequences().size();
							if (input.mCurrentSequenceIndex >= size)
							{
								// if we have reached the end of the sequence container, 
								// stop or start again depending on whether loop is true or not
								if (input.mIsLooping)
								{
									input.mCurrentSequenceIndex = 0;
									i = 0;
									input.mTime = deltaTime;
								}
								else
								{
									input.mIsFinished = true;
									input.mIsPlaying = false;
									input.mCurrentSequenceIndex = 0;
								}
							}
							else if (input.mCurrentSequenceIndex < 0)
							{
								// if we have reached the beginning of the sequence container ( time is smaller then zero ), 
								// stop or start again from the end depending on whether loop is true or not
								if (input.mIsLooping)
								{
									input.mCurrentSequenceIndex = mSequenceContainer->getSequences().size() - 1;
									i = 0;
									input.mTime = input.mDuration - deltaTime;
								}
								else
								{
									input.mIsFinished = true;
									input.mIsPlaying = false;
									input.mCurrentSequenceIndex = 0;
								}
							}
						}
						else
						{
							break;
						}
					}

					// copy parameters back
					{
						std::lock_guard<std::mutex> l(mThreadInputMutex);
						mCurrentSequenceIndex = input.mCurrentSequenceIndex;
						mIsFinished = input.mIsFinished;
						mIsLooping = input.mIsLooping;
						mIsPaused = input.mIsPaused;
						mIsPlaying = input.mIsPlaying;
						mSpeed = input.mSpeed;
						mTime = input.mTime;
						mDuration = input.mDuration;
					}

					// set the return time
					mReturnTime = mTime;

					// declare struct for input for of flex device
					FlexInput flexInput;

					// motor inputs
					for (int i = 0; i < 8; i++)
					{
						flexInput.setInput(i, static_cast<ParameterFloat*>(mParameters[i])->mValue);
					}

					// slack
					flexInput.mSlack = static_cast<ParameterFloat*>(mParameters[8])->mValue;

					// overrides
					for (int i = 9; i < 17; i++)
					{
						flexInput.setOverride(i - 9, static_cast<ParameterFloat*>(mParameters[i])->mValue);
					}

					// sinus
					flexInput.mSinusAmplitude = static_cast<ParameterFloat*>(mParameters[17])->mValue;
					flexInput.mSinusFrequency = static_cast<ParameterFloat*>(mParameters[18])->mValue;

					// give input
					mFlexDevice->setInput(flexInput);
				}
				else
				{
					mUpdateThreadRunning = false;
				}

				std::this_thread::sleep_for(std::chrono::microseconds(sleep_time_micro));
			}
		}


		void SequencePlayerComponentInstance::reconstruct()
		{
			mCurrentSequenceIndex = 0;
			mSequenceContainer->reconstruct();
			mDuration = mSequenceContainer->getSequences().back()->getStartTime() + mSequenceContainer->getSequences().back()->getDuration();
		}


		const void SequencePlayerComponentInstance::evaluate(double time, std::vector<Parameter*> &output, int offset) const
		{
			int currentSequenceIndex = 0;

			for (int i = 0; i < mSequenceContainer->getSequences().size(); i++)
			{
				int result = mSequenceContainer->getSequences()[currentSequenceIndex]->evaluate(time, output);

				if (result != 0)
				{
					currentSequenceIndex += result;

					int size = mSequenceContainer->getSequences().size();
					if (currentSequenceIndex >= size)
					{
						currentSequenceIndex = 0;
						i = 0;
					}
					else if (currentSequenceIndex < 0)
					{
						currentSequenceIndex = mSequenceContainer->getSequences().size() - 1;
						i = 0;
					}
				}
				else
				{
					break;
				}
			}
		}


		void SequencePlayerComponentInstance::stopThread()
		{
			// stop running thread
			mUpdateThreadRunning = false;
			if (mUpdateTask.valid())
			{
				mUpdateTask.wait();
			}
		}


		void SequencePlayerComponentInstance::play()
		{
			stopThread();

			mBefore = mTimer.now();

			//
			if (mIsPlaying)
			{
				mIsPaused = false;
			}
			else
			{
				mCurrentSequenceIndex = 0;
				mIsPlaying = true;
				mIsFinished = false;
				mTime = 0.0;
			}

			startThread();
		}

		void SequencePlayerComponentInstance::startThread()
		{
			//
			mUpdateThreadRunning = true;
			mUpdateTask = std::async(std::launch::async, std::bind(&SequencePlayerComponentInstance::onUpdate, this));
		}


		Sequence* SequencePlayerComponentInstance::getSequenceAtTime(double time) const
		{
			for (int i = 0; i < mSequenceContainer->getSequences().size(); i++)
			{
				if (time >= mSequenceContainer->getSequences()[i]->getStartTime() &&
					time < mSequenceContainer->getSequences()[i]->getStartTime() + mSequenceContainer->getSequences()[i]->getDuration())
				{
					return mSequenceContainer->getSequences()[i];
				}
			}

			return nullptr;
		}


		void SequencePlayerComponentInstance::setTime(const double time)
		{
			std::lock_guard<std::mutex> l(mThreadInputMutex);

			mTime = math::clamp<double>(time, 0.0, mDuration);
			mReturnTime = mTime;
			mCurrentSequenceIndex = 0;
		}


		void SequencePlayerComponentInstance::pause()
		{
			stopThread();

			if (!mIsPaused)
			{
				mIsPaused = true;
			}

			startThread();
		}


		void SequencePlayerComponentInstance::stop()
		{
			// stop running thread
			stopThread();

			mIsPlaying = false;
			mIsPaused = false;
			mIsFinished = false;
			mCurrentSequenceIndex = 0;
			mTime = 0.0;
			mReturnTime = mTime;
		}


		bool SequencePlayerComponentInstance::save(std::string showName, utility::ErrorState& errorState)
		{
			std::lock_guard<std::mutex> l(mThreadInputMutex);

			if (errorState.check(mIsPlaying, "Cannot save when playing!"))
				return false;

			//
			// Ensure the shows directory exists
			const std::string dir = "shows";
			utility::makeDirs(utility::getAbsolutePath(dir));

			std::string show_path = dir + "/" + showName;

			// Serialize current set of parameters to json
			rtti::JSONWriter writer;
			rtti::ObjectList sequenceList;
			for (const auto& sequence : mSequenceContainer->getSequences())
			{
				sequenceList.emplace_back(sequence);
			}

			// serialize the list
			if (!rtti::serializeObjects(sequenceList, writer, errorState))
				return false;

			// Open output file
			std::ofstream output(show_path, std::ios::binary | std::ios::out); 
			if (!errorState.check(output.is_open() && output.good(), "Failed to open %s for writing", show_path.c_str()))
				return false;

			// Write to disk
			std::string json = writer.GetJSON();
			output.write(json.data(), json.size());

			return true;
		}


		bool SequencePlayerComponentInstance::load(std::string showPath, utility::ErrorState& errorState)
		{
			std::lock_guard<std::mutex> l(mThreadInputMutex);

			if (errorState.check(mIsPlaying, "Cannot load when playing!"))
				return false;

			//
			rtti::DeserializeResult result;

			//
			mShowName = utility::getFileNameWithoutExtension(showPath);

			//
			std::string show_path = showPath;

			// Load the parameters from the preset
			rtti::Factory factory;
			if (!rtti::readJSONFile(showPath, rtti::EPropertyValidationMode::DisallowMissingProperties, rtti::EPointerPropertyMode::NoRawPointers, factory, result, errorState))
				return false;

			// Resolve links
			if (!rtti::DefaultLinkResolver::sResolveLinks(result.mReadObjects, result.mUnresolvedPointers, errorState))
				return false;

			std::map<int, std::unique_ptr<Sequence>> sequenceMap;

			// Find the root parameter group in the preset file and apply parameters
			for (auto& object : result.mReadObjects)
			{
				if (!object->init(errorState))
					return false;

				// make map of sequences, transfer ownership, we will move the ownership to the sequence container later on
				if (object->get_type().is_derived_from<Sequence>())
				{
					std::unique_ptr<Sequence> sequence(dynamic_cast<Sequence*>(object.release()));
					sequenceMap.insert(std::pair<int, std::unique_ptr<Sequence>>(sequence->mIndexInSequenceContainer, std::move(sequence)));
				}
			}

			// transfer ownership of read objects
			mOwnedObjects.clear();
			for (int i = 0; i < result.mReadObjects.size(); i++)
			{
				if (result.mReadObjects[i] != nullptr)
				{
					mOwnedObjects.emplace_back(std::move(result.mReadObjects[i]));
				}
			}

			// fill the sequencecontainer with the right sequences
			mSequenceContainer->clearSequences();
			std::vector<std::unique_ptr<Sequence>> sequences(sequenceMap.size());
			for (int i = 0 ; i < sequenceMap.size(); i++)
			{
				sequences[i] = std::move(sequenceMap[i]);
			}

			mSequenceContainer->setSequences(sequences);

			// reconstruct the sequence
			reconstruct();

			return true;
		}


		bool SequencePlayerComponentInstance::insertSequence(std::unique_ptr<Sequence> sequence, utility::ErrorState& errorState)
		{
			std::lock_guard<std::mutex> l(mThreadInputMutex);

			if (errorState.check(mIsPlaying, "Cannot insert sequence when playing!"))
				return false;

			mSequenceContainer->insertSequence(std::move(sequence));

			reconstruct();

			return true;
		}


		bool SequencePlayerComponentInstance::removeSequence(const Sequence* sequence, utility::ErrorState& errorState)
		{
			std::lock_guard<std::mutex> l(mThreadInputMutex);

			if (errorState.check(!mIsPlaying, "Cannot remove sequence when playing!"))
				return false;

			mSequenceContainer->removeSequence(sequence);

			reconstruct();

			return true;
		}


		bool SequencePlayerComponentInstance::removeSequenceElement(const Sequence* sequence, const SequenceElement* element, utility::ErrorState& errorState)
		{
			std::lock_guard<std::mutex> l(mThreadInputMutex);

			if (errorState.check(mIsPlaying, "Cannot remove sequence element when playing!"))
				return false;

			mSequenceContainer->removeSequenceElement(sequence, element);
			reconstruct();

			return true;
		}


		void SequencePlayerComponentInstance::setSpeed(const float speed)
		{
			std::lock_guard<std::mutex> l(mThreadInputMutex);
			mSpeed = speed;
		}


		void SequencePlayerComponentInstance::setIsLooping(const bool isLooping)
		{
			std::lock_guard<std::mutex> l(mThreadInputMutex);
			mIsLooping = isLooping;
		}
	}
}