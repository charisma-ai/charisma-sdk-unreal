// Copyright 2022 Charisma Entertainment Ltd

#include "MicrophoneCapture.h"

#include "Voice.h"

UMicrophoneCapture::UMicrophoneCapture()
{
}

UMicrophoneCapture::~UMicrophoneCapture()
{
	if (bIsCaptureActive)
	{
		StopCapture();
	}
}

bool UMicrophoneCapture::StartCapture(int32 SampleRate)
{
	if (bIsCaptureActive)
	{
		return false;
	}

	if (!FVoiceModule::Get().DoesPlatformSupportVoiceCapture())
	{
		UE_LOG(LogTemp, Log, TEXT("VoiceCapture is not supported on this platform!"));
	}

	if (!VoiceCapture.IsValid())
	{
		VoiceCapture = FVoiceModule::Get().CreateVoiceCapture("");
		if (VoiceCapture.IsValid())
		{
			FString DeviceName;
			if (!VoiceCapture->Init(DeviceName, SampleRate, 1))
			{
				return false;
			}
			UE_LOG(LogTemp, Log, TEXT("VoiceCapture initialized"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to obtain VoiceCapture, no voice available!"));
			return false;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Capture started"));
	VoiceCapture->Start();
	bIsCaptureActive = true;

#if ENGINE_MAJOR_VERSION < 5
	TickerHandle =
		FTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([this](float TimeDelta) { return Tick(TimeDelta); }));
#else
	TickerHandle =
		FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([this](float TimeDelta) { return Tick(TimeDelta); }));
#endif

	return true;
}

void UMicrophoneCapture::StopCapture()
{
	bIsCaptureActive = false;

	if (VoiceCapture.IsValid())
	{
		VoiceCapture->Stop();
	}

	if (TickerHandle.IsValid())
	{
#if ENGINE_MAJOR_VERSION < 5
		FTicker::GetCoreTicker().RemoveTicker(TickerHandle);
#else
		FTSTicker::GetCoreTicker().RemoveTicker(TickerHandle);
#endif
	}
}

bool UMicrophoneCapture::Tick(float DeltaTime)
{
	if (VoiceCapture.IsValid() && bIsCaptureActive)
	{
		uint32 VoiceCaptureBytesAvailable = 0;
		EVoiceCaptureState::Type CaptureState = VoiceCapture->GetCaptureState(VoiceCaptureBytesAvailable);

		if (CaptureState == EVoiceCaptureState::Ok && VoiceCaptureBytesAvailable > 0)
		{
			uint32 VoiceCaptureReadBytes = 0;
			TArray<uint8> VoiceData;
			VoiceData.AddUninitialized(VoiceCaptureBytesAvailable);
			VoiceCapture->GetVoiceData(VoiceData.GetData(), VoiceCaptureBytesAvailable, VoiceCaptureReadBytes);
			if (OnSpeechAudio)
			{
				OnSpeechAudio(VoiceData, VoiceCaptureReadBytes);
			}
		}
	}

	return true;
}
