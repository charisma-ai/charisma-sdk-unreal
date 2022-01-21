// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"

#include "Playthrough.h"

#include "PlaythroughCreateCharismaConversation.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPlaythroughMessagesManagerReadyDelegate, const int32, ConversationId);

UCLASS()
class CHARISMAMODULE_API UPlaythroughCreateCharismaConversation : public UBlueprintAsyncActionBase
{
	GENERATED_UCLASS_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"), Category = "")
	static UPlaythroughCreateCharismaConversation* CreateCharismaConversation(UObject* WorldContextObject, UPlaythrough* Playthrough, const FString Token, const int32 PlaythroughId);

	virtual void Activate() override;

	UPROPERTY()
	UPlaythrough* CurPlaythrough;
	UPROPERTY()
	FString CurToken;
	UPROPERTY()
	int32 CurPlaythroughId;
	int32 CurConversationId;

private:
	UFUNCTION()
	void BroadcastOnCreated(const int32 ConversationId);

	UFUNCTION()
	void BroadcastReady();

	UPROPERTY(BlueprintAssignable)
	FPlaythroughMessagesManagerReadyDelegate OnReady;

	UObject* WorldContextObject;
};