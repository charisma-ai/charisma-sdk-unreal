// Copyright 2022 Charisma Entertainment Ltd

#include "APIBlueprints/ResetPlaythrough.h"

UResetPlaythrough::UResetPlaythrough(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer), WorldContextObject(nullptr)
{
}

UResetPlaythrough* UResetPlaythrough::ResetPlaythrough(UObject* WorldContextObject, const FString Token, const FString EventId)
{
	UResetPlaythrough* BlueprintNode = NewObject<UResetPlaythrough>();
	BlueprintNode->WorldContextObject = WorldContextObject;
	BlueprintNode->Token = Token;
	BlueprintNode->EventId = EventId;
	return BlueprintNode;
}

void UResetPlaythrough::Activate()
{
	UCharismaAPI::ResetPlaythrough(
		Token, EventId, [&]() { Success.Broadcast(TEXT("")); }, [&](const FString Error) { Failure.Broadcast(Error); });
}
