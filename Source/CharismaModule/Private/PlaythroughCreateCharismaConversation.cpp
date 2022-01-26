// Fill out your copyright notice in the Description page of Project Settings.


#include "PlaythroughCreateCharismaConversation.h"

UPlaythroughCreateCharismaConversation::UPlaythroughCreateCharismaConversation(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer), WorldContextObject(nullptr)
{
}

UPlaythroughCreateCharismaConversation* UPlaythroughCreateCharismaConversation::CreateCharismaConversation(UObject* WorldContextObject, UPlaythrough* Playthrough, const FString Token, const int32 PlaythroughId)
{
	UPlaythroughCreateCharismaConversation* BlueprintNode = NewObject<UPlaythroughCreateCharismaConversation>();
	BlueprintNode->WorldContextObject = WorldContextObject;
	BlueprintNode->CurPlaythrough = Playthrough;
	BlueprintNode->CurToken = Token;
	BlueprintNode->CurPlaythroughId = PlaythroughId;
	return BlueprintNode;
}

void UPlaythroughCreateCharismaConversation::Activate()
{
	UCharismaAPI::CreateConversationAPI(CurPlaythrough ,CurToken);
	CurPlaythrough->Connect(CurToken, CurPlaythroughId);
}                                                             