// Copyright 2022 Charisma Entertainment Ltd

#pragma once

#include "Engine/Engine.h"

DECLARE_LOG_CATEGORY_EXTERN(LogCharisma, Log, All);

namespace CharismaLogger
{
	enum ELogSeverity
	{
		Info,
		Warning,
		Error
	};

	static void Log(const int32 Key, const FString& Message, const ELogSeverity Severity, const float Duration = 5.0f)
	{
		FColor MessageColor;
		switch (Severity)
		{
			case Info:
				MessageColor = FColor::Green;
				UE_LOG(LogCharisma, Log, TEXT("Charisma: %s"), *Message);
				break;
			case Warning:
				MessageColor = FColor::Orange;
				UE_LOG(LogCharisma, Warning, TEXT("Charisma: %s"), *Message);
				break;
			case Error:
				MessageColor = FColor::Red;
				UE_LOG(LogCharisma, Error, TEXT("Charisma: %s"), *Message);
				break;
			default:
				MessageColor = FColor::White;
				UE_LOG(LogCharisma, Log, TEXT("Charisma: %s"), *Message);
		}

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(Key, Duration, MessageColor, FString("Charisma: " + Message));
		}
	}
}