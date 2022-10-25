// Copyright 2022 Charisma Entertainment Ltd

#include "APIBlueprints/GetPlaythroughInfo.h"

UGetPlaythroughInfo::UGetPlaythroughInfo(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer), WorldContextObject(nullptr)
{
}

UGetPlaythroughInfo* UGetPlaythroughInfo::GetPlaythroughInfo(UObject* WorldContextObject, const FString Token)
{
	UGetPlaythroughInfo* BlueprintNode = NewObject<UGetPlaythroughInfo>();
	BlueprintNode->WorldContextObject = WorldContextObject;
	BlueprintNode->Token = Token;
	return BlueprintNode;
}

void UGetPlaythroughInfo::Activate()
{
	UCharismaAPI::GetPlaythroughInfo(
		Token, [&](const FCharismaPlaythroughInfoResponse PlaythroughInfo) { Success.Broadcast(PlaythroughInfo, TEXT("")); },
		[&](const FString Error) { Failure.Broadcast(FCharismaPlaythroughInfoResponse(), Error); });
}
