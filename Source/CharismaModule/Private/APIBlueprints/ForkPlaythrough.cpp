// Copyright 2022 Charisma Entertainment Ltd

#include "APIBlueprints/ForkPlaythrough.h"

UForkPlaythrough::UForkPlaythrough(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer), WorldContextObject(nullptr)
{
}

UForkPlaythrough* UForkPlaythrough::ForkPlaythrough(UObject* WorldContextObject, const FString Token)
{
	UForkPlaythrough* BlueprintNode = NewObject<UForkPlaythrough>();
	BlueprintNode->WorldContextObject = WorldContextObject;
	BlueprintNode->Token = Token;
	
	return BlueprintNode;
}

void UForkPlaythrough::Activate()
{
	UCharismaAPI::ForkPlaythrough(
		Token, [&](const FString Token, const FString PlaythroughUuid) { Success.Broadcast(Token, PlaythroughUuid, TEXT("")); },
		[&](const FString Error) { Failure.Broadcast(TEXT(""),TEXT(""), Error); });
}
