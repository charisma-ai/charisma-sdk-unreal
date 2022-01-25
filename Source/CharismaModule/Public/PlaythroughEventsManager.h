// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"

#include "Playthrough.h"

#include "PlaythroughEventsManager.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FPEMDelegate, int32, ConversationId, bool, IsConnected, bool, IsTyping, const FCharismaMessageEvent&, MessageEvent);


UCLASS()
class CHARISMAMODULE_API UPlaythroughEventsManager : public UBlueprintAsyncActionBase
{
	GENERATED_UCLASS_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"), Category = "")
	static UPlaythroughEventsManager* AttachPlaythroughHandlers(
		UObject* WorldContextObject, UPlaythrough* Playthrough);
	virtual void Activate() override;

	UPROPERTY()
	UPlaythrough* CurPlaythrough;
	UPROPERTY()
	int32 CurConversationId;
	UPROPERTY()
	bool CurIsConnected = false;
	UPROPERTY()
	bool CurIsTyping = false;
	UPROPERTY()
	FCharismaMessageEvent CurMessageEvent;

private:

	UPROPERTY(BlueprintAssignable, Category = Events)
	FPEMDelegate OnConversationCreated;
	UPROPERTY(BlueprintAssignable, Category = Events)
	FPEMDelegate OnConnected;
	UPROPERTY(BlueprintAssignable, Category = Events)
	FPEMDelegate OnReady;
	UPROPERTY(BlueprintAssignable, Category = Events)
	FPEMDelegate OnTyping;
	UPROPERTY(BlueprintAssignable, Category = Events)
	FPEMDelegate OnMessage;

	UFUNCTION()
	void OnConversationCreatedFUNC(int32 ConversationId);
	UFUNCTION()
	void OnConnectedFUNC(bool IsConnected);
	UFUNCTION()
	void OnReadyFUNC();
	UFUNCTION()
	void OnTypingFUNC(bool IsTyping);
	UFUNCTION()
	void OnMessageFUNC(const FCharismaMessageEvent& MessageEvent);

	UObject* WorldContextObject;
};
