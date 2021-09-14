#pragma once

#include "Audio.h"
#include "AudioDevice.h"
#include "Developer/TargetPlatform/Public/Interfaces/IAudioFormat.h"
#include "Runtime/Engine/Classes/Sound/SoundWave.h"
#include "Runtime/Engine/Public/VorbisAudioInfo.h"

#include "CharismaAudio.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CHARISMAMODULE_API UCharismaAudio : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = Charisma)
	static USoundWave* CreateSoundFromBytes(const TArray<uint8>& RawBytes);

private:
	static bool FillSoundWaveInfo(USoundWave* SoundWave, const TArray<uint8>& RawData);
};

UCLASS()
class USoundWave_InMemoryOgg : public USoundWave
{
	GENERATED_BODY()

public:
	virtual bool HasCompressedData(FName Format, ITargetPlatform* TargetPlatform) const override
	{
		// Default implementation appends a hash to the format name which is specific to the platform,
		// This is what was causing OGG data to not be found. E.g. on my system it was OGG10000-1-1-1-1-1
		if (Format != OggFormatName)
		{
			return false;
		}

		return CompressedFormatData.Contains(Format);
	}

	virtual void BeginGetCompressedData(FName Format, const FPlatformAudioCookOverrides* CompressionOverrides) override
	{
		// Do nothing, data is already in memory
	}

	virtual FByteBulkData* GetCompressedData(
		FName Format, const FPlatformAudioCookOverrides* CompressionOverrides = nullptr) override
	{
		if (Format == OggFormatName)
		{
			return &CompressedFormatData.GetFormat(Format);
		}
		return nullptr;
	}

	virtual void InitAudioResource(FByteBulkData& CompressedData) override
	{
		// Should never be pushing additional compressed data to a this
		check(false);
	}

private:
	FName OggFormatName = FName(TEXT("OGG"));
};
