// Fill out your copyright notice in the Description page of Project Settings.

#include "CharismaInitialization.h"

UCharismaInitialization::UCharismaInitialization(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer), WorldContextObject(nullptr)
{
}

UCharismaInitialization* UCharismaInitialization::CharismaInitialization(
	UObject* WorldContextObject, const int32 StoryId, const int32 StoryVersion, const FString& ApiKey)
{
	UCharismaInitialization* BlueprintNode = NewObject<UCharismaInitialization>();
	BlueprintNode->WorldContextObject = WorldContextObject;
	BlueprintNode->InitStoryId = StoryId;
	BlueprintNode->InitStoryVersion = StoryVersion;
	BlueprintNode->InitApiKey = ApiKey;
	return BlueprintNode;
}

void UCharismaInitialization::Activate()
{
	UPlaythrough* RefPlaythrough;

	UPlaythrough::CreateCharismaPlaythroughObject(WorldContextObject, RefPlaythrough);

	UCharismaAPI::CreatePlaythroughToken(InitStoryId, InitStoryVersion, InitApiKey,
		[&](const FString Token, const FString PlaythroughUuid) { Success.Broadcast(RefPlaythrough, Token, PlaythroughUuid); },
		[&](const FString Error) { Failure.Broadcast(RefPlaythrough, TEXT(""), TEXT("")); });
}
