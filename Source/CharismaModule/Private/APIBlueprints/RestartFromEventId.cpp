// Copyright 2022 Charisma Entertainment Ltd

#include "APIBlueprints/RestartFromEventId.h"

URestartFromEventId::URestartFromEventId(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer), WorldContextObject(nullptr)
{
}

URestartFromEventId* URestartFromEventId::RestartFromEventId(
	UObject* WorldContextObject, const FString Token, const FString EventId)
{
	URestartFromEventId* BlueprintNode = NewObject<URestartFromEventId>();
	BlueprintNode->WorldContextObject = WorldContextObject;
	BlueprintNode->Token = Token;
	BlueprintNode->EventId = EventId;
	return BlueprintNode;
}

void URestartFromEventId::Activate()
{
	UCharismaAPI::RestartFromEventId(
		Token, EventId, [&]() { Success.Broadcast(TEXT("")); }, [&](const FString Error) { Failure.Broadcast(Error); });
}
