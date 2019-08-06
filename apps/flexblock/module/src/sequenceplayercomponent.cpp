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
RTTI_PROPERTY("TimelineParameters", &nap::timeline::SequencePlayerComponent::mParameterGroup, nap::rtti::EPropertyMetaData::Required)
RTTI_PROPERTY("StartShowFile", &nap::timeline::SequencePlayerComponent::mDefaultShow, nap::rtti::EPropertyMetaData::FileLink)

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


		bool SequencePlayerComponentInstance::init(utility::ErrorState& errorState)
		{
			SequencePlayerComponent* resource = getComponent<SequencePlayerComponent>();

			mSequenceContainer = std::make_unique<SequenceContainer>();
			if (!mSequenceContainer->init(errorState))
				return false;

			const auto& parameterGroup = resource->mParameterGroup;
			for (const auto& parameter : parameterGroup->mParameters)
			{
				mParameters.emplace_back(parameter.get());
			}

			if (!load(resource->mDefaultShow, errorState))
				return false;

			if( mSequenceContainer->getSequences().size() > 0 )
				mDuration = mSequenceContainer->getSequences().back()->getStartTime() + mSequenceContainer->getSequences().back()->getDuration();

			return true;
		}


		void SequencePlayerComponentInstance::update(double deltaTime)
		{
			if (mIsPlaying)
			{
				// advance time
				if (!mIsPaused)
				{
					mTime += deltaTime * mSpeed;
				}

				// iterate trough sequences
				for (int i = 0; i < mSequenceContainer->getSequences().size(); i++)
				{
					// get result of process
					// -1 is we should move backwards ( time is smaller then start time of current sequence )
					// 1 is we should move forward ( time is bigger then start time + duration of current sequence )
					// 0 is we are in the right sequence ( time is bigger then start time and smaller then start time + duration of sequence )
					int result = mSequenceContainer->getSequences()[mCurrentSequenceIndex]->process(mTime, mParameters);
					
					if (result != 0)
					{
						// move backwards or forward in index according to result
						mCurrentSequenceIndex += result;

						size_t size = mSequenceContainer->getSequences().size();
						if (mCurrentSequenceIndex >= size)
						{
							// if we have reached the end of the sequence container, 
							// stop or start again depending on whether loop is true or not
							if (mIsLooping)
							{
								mCurrentSequenceIndex = 0;
								i = 0;
								mTime = deltaTime;
							}
							else
							{
								mIsFinished = true;
								mIsPlaying = false;
								mCurrentSequenceIndex = 0;
							}
						}
						else if (mCurrentSequenceIndex < 0)
						{
							// if we have reached the beginning of the sequence container ( time is smaller then zero ), 
							// stop or start again from the end depending on whether loop is true or not
							if (mIsLooping)
							{
								mCurrentSequenceIndex = mSequenceContainer->getSequences().size() - 1;
								i = 0;
								mTime = mDuration - deltaTime;
							}
							else
							{
								mIsFinished = true;
								mIsPlaying = false;
								mCurrentSequenceIndex = 0;
							}
						}
					}
					else
					{
						break;
					}
				}
			}
		}


		void SequencePlayerComponentInstance::reconstruct()
		{
			mCurrentSequenceIndex = 0;
			mSequenceContainer->reconstruct();
			mDuration = mSequenceContainer->getSequences().back()->getStartTime() + mSequenceContainer->getSequences().back()->getDuration();
		}


		const void SequencePlayerComponentInstance::evaluate(double time, std::vector<Parameter*> &output) const
		{
			int currentSequenceIndex = 0;

			for (int i = 0; i < mSequenceContainer->getSequences().size(); i++)
			{
				int result = mSequenceContainer->getSequences()[currentSequenceIndex]->process(time, output);

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


		void SequencePlayerComponentInstance::play()
		{
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
			mTime = math::clamp<double>(time, 0.0, mDuration);
			mCurrentSequenceIndex = 0;
		}


		void SequencePlayerComponentInstance::pause()
		{
			mIsPaused = true;
		}


		void SequencePlayerComponentInstance::stop()
		{
			mIsPlaying = false;
			mIsPaused = false;
			mIsFinished = false;
			mCurrentSequenceIndex = 0;
			mTime = 0.0;
		}


		bool SequencePlayerComponentInstance::save(std::string showName, utility::ErrorState& errorState)
		{
			//
			// Ensure the presets directory exists
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


		void SequencePlayerComponentInstance::insertSequence(std::unique_ptr<Sequence> sequence)
		{
			mSequenceContainer->insertSequence(std::move(sequence));

			reconstruct();
		}


		void SequencePlayerComponentInstance::removeSequence(const Sequence* sequence)
		{
			mSequenceContainer->removeSequence(sequence);

			reconstruct();
		}


		void SequencePlayerComponentInstance::removeSequenceElement(const Sequence* sequence, const SequenceElement* element)
		{
			mSequenceContainer->removeSequenceElement(sequence, element);
			reconstruct();
		}
	}
}