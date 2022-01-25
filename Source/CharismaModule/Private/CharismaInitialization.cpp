// Fill out your copyright notice in the Description page of Project Settings.


#include "CharismaInitialization.h"

UCharismaInitialization::UCharismaInitialization(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer), WorldContextObject(nullptr)
{
}

UCharismaInitialization* UCharismaInitialization::CharismaInitialization(UObject* WorldContextObject, const int32 StoryId, const int32 StoryVersion, const FString& ApiKey)
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

	UPlaythrough::CreateCharismaPlaythroughObject(RefPlaythrough);

	RefPlaythrough->OnTokenCreationSuccess.AddDynamic(this, &UCharismaInitialization::BroadcastSuccess);
	RefPlaythrough->OnTokenCreationFailure.AddDynamic(this, &UCharismaInitialization::BroadcastFailure);

	UCharismaAPI::InitPlaythoughObject(InitStoryId, InitStoryVersion, InitApiKey, RefPlaythrough);
	

}

void UCharismaInitialization::BroadcastSuccess(const FString Token, const int32 PlaythroughId, const UPlaythrough* Playthrough)
{
	Success.Broadcast(Playthrough, PlaythroughId, Token);
}

void UCharismaInitialization::BroadcastFailure(const FString Token, const int32 PlaythroughId, const UPlaythrough* Playthrough)
{
	Failure.Broadcast(Playthrough, PlaythroughId, Token);
}