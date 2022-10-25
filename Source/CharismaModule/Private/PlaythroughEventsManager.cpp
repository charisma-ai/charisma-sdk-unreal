// Copyright 2022 Charisma Entertainment Ltd

#include "PlaythroughEventsManager.h"

UPlaythroughEventsManager::UPlaythroughEventsManager(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer), WorldContextObject(nullptr)
{
}

UPlaythroughEventsManager* UPlaythroughEventsManager::AttachPlaythroughHandlers(
	UObject* WorldContextObject, UPlaythrough* Playthrough)
{
	UPlaythroughEventsManager* BlueprintNode = NewObject<UPlaythroughEventsManager>();
	BlueprintNode->CurPlaythrough = Playthrough;
	return BlueprintNode;
}

void UPlaythroughEventsManager::Activate()
{
	CurPlaythrough->OnConnected.AddDynamic(this, &UPlaythroughEventsManager::OnConnectedFUNC);
	CurPlaythrough->OnReady.AddDynamic(this, &UPlaythroughEventsManager::OnReadyFUNC);
	CurPlaythrough->OnTyping.AddDynamic(this, &UPlaythroughEventsManager::OnTypingFUNC);
	CurPlaythrough->OnMessage.AddDynamic(this, &UPlaythroughEventsManager::OnMessageFUNC);
}

void UPlaythroughEventsManager::OnConnectedFUNC(bool IsConnected)
{
	CurIsConnected = IsConnected;
	OnConnected.Broadcast(CurConversationId, CurIsConnected, CurIsTyping, CurMessageEvent);
}

void UPlaythroughEventsManager::OnReadyFUNC()
{
	OnReady.Broadcast(CurConversationId, CurIsConnected, CurIsTyping, CurMessageEvent);
}

void UPlaythroughEventsManager::OnTypingFUNC(bool IsTyping)
{
	CurIsTyping = IsTyping;
	OnTyping.Broadcast(CurConversationId, CurIsConnected, CurIsTyping, CurMessageEvent);
}

void UPlaythroughEventsManager::OnMessageFUNC(const FCharismaMessageEvent& MessageEvent)
{
	CurMessageEvent = MessageEvent;
	OnMessage.Broadcast(CurConversationId, CurIsConnected, CurIsTyping, CurMessageEvent);
}