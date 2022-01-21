// Fill out your copyright notice in the Description page of Project Settings.


#include "PlaythroughStartConversation.h"

UPlaythroughStartConversation::UPlaythroughStartConversation(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer), WorldContextObject(nullptr)
{
}

UPlaythroughStartConversation* UPlaythroughStartConversation::StartConversation(UObject* WorldContextObject, UPlaythrough* Playthrough, const int32 ConversationId, const int32 SceneIndex, const int32 StartGraphId,
	const FString& StartGraphReferenceId,
	const bool UseSpeech)
{
	UPlaythroughStartConversation* BlueprintNode = NewObject<UPlaythroughStartConversation>();
	BlueprintNode->WorldContextObject = WorldContextObject;
	BlueprintNode->CurPlaythrough = Playthrough;
	BlueprintNode->CurConversationId = ConversationId;
	BlueprintNode->CurPlaythrough = Playthrough;
	BlueprintNode->CurConversationId = ConversationId;
	BlueprintNode->CurSceneIndex = SceneIndex;
	BlueprintNode->CurStartGraphId = StartGraphId;
	BlueprintNode->CurStartGraphReferenceId = StartGraphReferenceId;
	BlueprintNode->CurUseSpeech = UseSpeech;
	return BlueprintNode;
}

void UPlaythroughStartConversation::Activate()
{

	CurPlaythrough->Start(CurConversationId, CurSceneIndex, CurStartGraphId, CurStartGraphReferenceId, CurUseSpeech);

	CurPlaythrough->OnMessage.AddDynamic(this, &UPlaythroughStartConversation::BroadcastMessage);
}

void UPlaythroughStartConversation::BroadcastMessage(const FCharismaMessageEvent& MessageEvent)
{
	OnMessage.Broadcast(MessageEvent);
}