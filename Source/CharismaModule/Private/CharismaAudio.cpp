#include "CharismaAudio.h"

USoundWave* UCharismaAudio::CreateSoundFromBytes(const TArray<uint8>& RawBytes)
{
	USoundWave_InMemoryOgg* SoundWaveRef = NewObject<USoundWave_InMemoryOgg>(USoundWave::StaticClass());

	if (!SoundWaveRef)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Failed to create new SoundWave object."));
		}
		return nullptr;
	}

	if (RawBytes.Num() > 0)
	{
		if (!FillSoundWaveInfo(SoundWaveRef, RawBytes))
		{
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Something went wrong when loading SoundWave data."));
			}
			return nullptr;
		}
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,
				TEXT("Could not load SoundWave since raw audio data is empty. Make sure speech is toggled on in the Charisma "
					 "component."));
		}
		return nullptr;
	}

#if ENGINE_MAJOR_VERSION < 5
	FByteBulkData* BulkData = &SoundWaveRef->CompressedFormatData.GetFormat(FName("OGG"));
#else
	FByteBulkData* BulkData = SoundWaveRef->GetCompressedData(FName("OGG"));
#endif

	BulkData->Lock(LOCK_READ_WRITE);
	FMemory::Memmove(BulkData->Realloc(RawBytes.Num()), RawBytes.GetData(), RawBytes.Num());
	BulkData->Unlock();

	SoundWaveRef->SetPrecacheState(ESoundWavePrecacheState::Done);
	return SoundWaveRef;
}

bool UCharismaAudio::FillSoundWaveInfo(USoundWave* SoundWave, const TArray<uint8>& RawData)
{
	FSoundQualityInfo SoundQualityInfo;
	FVorbisAudioInfo VorbisAudioInfo;

	if (!VorbisAudioInfo.ReadCompressedInfo(RawData.GetData(), RawData.Num(), &SoundQualityInfo))
	{
		return false;
	}

	SoundWave->DecompressionType = EDecompressionType::DTYPE_RealTime;
	SoundWave->SoundGroup = ESoundGroup::SOUNDGROUP_Default;
	SoundWave->NumChannels = SoundQualityInfo.NumChannels;
	SoundWave->Duration = SoundQualityInfo.Duration;
	SoundWave->RawPCMDataSize = SoundQualityInfo.SampleDataSize;
	SoundWave->SetSampleRate(SoundQualityInfo.SampleRate);

	return true;
}
