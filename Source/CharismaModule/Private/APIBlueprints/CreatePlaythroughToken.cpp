// Copyright 2022 Charisma Entertainment Ltd

#include "APIBlueprints/CreatePlaythroughToken.h"

UCreatePlaythroughToken::UCreatePlaythroughToken(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer), WorldContextObject(nullptr)
{
}

UCreatePlaythroughToken* UCreatePlaythroughToken::CreatePlaythroughToken(
	UObject* WorldContextObject, const int32 StoryId, const int32 StoryVersion, const FString& ApiKey)
{
	UCreatePlaythroughToken* BlueprintNode = NewObject<UCreatePlaythroughToken>();
	BlueprintNode->WorldContextObject = WorldContextObject;
	BlueprintNode->StoryId = StoryId;
	BlueprintNode->StoryVersion = StoryVersion;
	BlueprintNode->ApiKey = ApiKey;
	return BlueprintNode;
}

void UCreatePlaythroughToken::Activate()
{
	UCharismaAPI::CreatePlaythroughToken(
		StoryId, StoryVersion, ApiKey,
		[&](const FString Token, const FString PlaythroughUuid) {
			Success.Broadcast(
				UPlaythrough::NewPlaythroughObject(WorldContextObject, Token, PlaythroughUuid), Token, PlaythroughUuid, TEXT(""));
		},
		[&](const FString Error) { Failure.Broadcast(nullptr, TEXT(""), TEXT(""), Error); });
}
