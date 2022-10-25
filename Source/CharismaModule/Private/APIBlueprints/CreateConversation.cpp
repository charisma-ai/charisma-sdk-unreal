// Copyright 2022 Charisma Entertainment Ltd

#include "APIBlueprints/CreateConversation.h"

UCreateConversation::UCreateConversation(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer), WorldContextObject(nullptr)
{
}

UCreateConversation* UCreateConversation::CreateConversation(UObject* WorldContextObject, const FString Token)
{
	UCreateConversation* BlueprintNode = NewObject<UCreateConversation>();
	BlueprintNode->WorldContextObject = WorldContextObject;
	BlueprintNode->Token = Token;
	return BlueprintNode;
}

void UCreateConversation::Activate()
{
	UCharismaAPI::CreateConversation(
		Token, [&](const FString ConversationUuid) { Success.Broadcast(ConversationUuid, TEXT("")); },
		[&](const FString Error) { Failure.Broadcast(TEXT(""), Error); });
}
