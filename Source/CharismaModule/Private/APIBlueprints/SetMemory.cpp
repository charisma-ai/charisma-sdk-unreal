// Copyright 2022 Charisma Entertainment Ltd

#include "APIBlueprints/SetMemory.h"

USetMemory::USetMemory(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer), WorldContextObject(nullptr)
{
}

USetMemory* USetMemory::SetMemory(
	UObject* WorldContextObject, const FString Token, const FString RecallValue, const FString SaveValue)
{
	USetMemory* BlueprintNode = NewObject<USetMemory>();
	BlueprintNode->WorldContextObject = WorldContextObject;
	BlueprintNode->Token = Token;
	BlueprintNode->RecallValue = RecallValue;
	BlueprintNode->SaveValue = SaveValue;
	return BlueprintNode;
}

void USetMemory::Activate()
{
	UCharismaAPI::SetMemory(
		Token, RecallValue, SaveValue, [&]() { Success.Broadcast(TEXT("")); },
		[&](const FString Error) { Failure.Broadcast(Error); });
}
