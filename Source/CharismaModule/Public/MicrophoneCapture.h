#pragma once

#include "CoreMinimal.h"
#include "Interfaces/VoiceCapture.h"

class UMicrophoneCapture
{
public:
	UMicrophoneCapture();
	~UMicrophoneCapture();

	bool bIsCaptureActive = false;

	bool StartCapture(int32 SampleRate = 16000);

	void StopCapture();

	bool Tick(float DeltaTime);

	TFunction<void(const TArray<uint8>& Audio, const uint32 AudioLength)> OnSpeechAudio;

private:
#if ENGINE_MAJOR_VERSION < 5
	FDelegateHandle TickerHandle;
#else
	FTSTicker::FDelegateHandle TickerHandle;
#endif
	TSharedPtr<class IVoiceCapture> VoiceCapture;
};