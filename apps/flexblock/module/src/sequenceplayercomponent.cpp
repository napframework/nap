#include "sequenceplayercomponent.h"

// External Includes
#include <entity.h>
#include <utility/fileutils.h>
#include <rtti/jsonwriter.h>
#include <rtti/jsonreader.h>
#include <rtti/defaultlinkresolver.h>
#include <fstream>


// nap::flexblocksequenceplayer run time class definition 
RTTI_BEGIN_CLASS(nap::timeline::SequencePlayerComponent)
// Put additional properties here
RTTI_PROPERTY("TimelineParameters", &nap::timeline::SequencePlayerComponent::mParameterGroup, nap::rtti::EPropertyMetaData::Required)
RTTI_PROPERTY("SequenceContainer", &nap::timeline::SequencePlayerComponent::mSequenceContainer, nap::rtti::EPropertyMetaData::Required)

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
			for (const auto* sequence_ : mSequenceContainer->mSequences)
			{
				if (sequence_ == sequence)
				{
					mSequenceContainer->mSequences[mCurrentSequenceIndex]->reset();
					mTime = sequence->getStartTime();
					mCurrentSequenceIndex = 0;
					mSequenceContainer->mSequences[mCurrentSequenceIndex]->reset();
					break;
				}
			}
		}

		bool SequencePlayerComponentInstance::init(utility::ErrorState& errorState)
		{
			SequencePlayerComponent* resource = getComponent<SequencePlayerComponent>();
			mSequenceContainer = resource->mSequenceContainer.get();

			const auto& parameterGroup = resource->mParameterGroup;
			for (const auto& parameter : parameterGroup->mParameters)
			{
				mParameters.emplace_back(parameter.get());
			}

			if( mSequenceContainer->mSequences.size() > 0 )
				mDuration = mSequenceContainer->mSequences.back()->getStartTime() + mSequenceContainer->mSequences.back()->getDuration();

			return true;
		}

		void SequencePlayerComponentInstance::update(double deltaTime)
		{
			if (mIsPlaying)
			{
				if (!mIsPaused)
				{
					mTime += deltaTime * mSpeed;
				}

				for (int i = 0; i < mSequenceContainer->mSequences.size(); i++)
				{
					int result = mSequenceContainer->mSequences[mCurrentSequenceIndex]->process(mTime, mParameters);
					
					if (result != 0)
					{
						mCurrentSequenceIndex += result;

						int size = mSequenceContainer->mSequences.size();
						if (mCurrentSequenceIndex >= size)
						{
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
							if (mIsLooping)
							{
								mCurrentSequenceIndex = mSequenceContainer->mSequences.size() - 1;
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
			mSequenceContainer->reconstruct();
			mDuration = mSequenceContainer->mSequences.back()->getStartTime() + mSequenceContainer->mSequences.back()->getDuration();
		}

		const void SequencePlayerComponentInstance::evaluate(double time, std::vector<Parameter*> &output) const
		{
			int currentSequenceIndex = 0;
			//if (mIsPlaying)
			//{
				for (int i = 0; i < mSequenceContainer->mSequences.size(); i++)
				{
					int result = mSequenceContainer->mSequences[currentSequenceIndex]->process(time, output);

					if (result != 0)
					{
						currentSequenceIndex += result;

						int size = mSequenceContainer->mSequences.size();
						if (currentSequenceIndex >= size)
						{
							currentSequenceIndex = 0;
							i = 0;
						}
						else if (currentSequenceIndex < 0)
						{
							currentSequenceIndex = mSequenceContainer->mSequences.size() - 1;
							i = 0;
						}
					}
					else
					{
						break;
					}
				}
			//}
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

		void SequencePlayerComponentInstance::setTime(const double time)
		{
			mTime = math::clamp<double>(time, 0.0, mDuration);
			mCurrentSequenceIndex = 0;

			for (const auto& sequence : mSequenceContainer->mSequences)
			{
				sequence->reset();
			}
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
			for (const auto& sequence : mSequenceContainer->mSequences)
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

		bool SequencePlayerComponentInstance::load(std::string showName, utility::ErrorState& errorState)
		{
			//
			mDeserializeResult.mFileLinks.clear();
			mDeserializeResult.mReadObjects.clear();
			mDeserializeResult.mUnresolvedPointers.clear();

			// Ensure the presets directory exists
			const std::string dir = utility::getAbsolutePath("shows");

			std::string show_path = dir + "/" + showName;

			// Load the parameters from the preset
			rtti::Factory factory;
			if (!rtti::readJSONFile(show_path, rtti::EPropertyValidationMode::DisallowMissingProperties, factory, mDeserializeResult, errorState))
				return false;

			// Resolve links
			if (!rtti::DefaultLinkResolver::sResolveLinks(mDeserializeResult.mReadObjects, mDeserializeResult.mUnresolvedPointers, errorState))
				return false;

			std::map<int, Sequence*> sequenceMap;
			// Find the root parameter group in the preset file and apply parameters
			for (auto& object : mDeserializeResult.mReadObjects)
			{
				if (!object->init(errorState))
					return false;

				if (object->get_type().is_derived_from<Sequence>())
				{
					Sequence* sequence = static_cast<Sequence*>(object.get());
					sequenceMap.insert(std::pair<int, Sequence*>(sequence->mIndexInSequenceContainer, sequence));

				}
			}

			mSequenceContainer->mSequences.clear();

			mSequenceContainer->mSequences = std::vector<Sequence*>(sequenceMap.size());
			for (int i = 0 ; i < sequenceMap.size(); i++)
			{
				mSequenceContainer->mSequences[i] = sequenceMap[i];
			}

			mSequenceContainer->reinit();
			reconstruct();

			return true;
		}
	}
}