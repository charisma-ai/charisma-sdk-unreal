// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"

#include "PLaythrough.h"

#include "PlaythroughStartConversation.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPlaythroughMessagesManagerDelegate, const FCharismaMessageEvent&, MessageEvent);
UCLASS()
class CHARISMAMODULE_API UPlaythroughStartConversation : public UBlueprintAsyncActionBase
{
	GENERATED_UCLASS_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"), Category = "")
	static UPlaythroughStartConversation* StartConversation(UObject* WorldContextObject, UPlaythrough* Playthrough,	const int32 ConversationId, const int32 SceneIndex, const int32 StartGraphId, const FString& StartGraphReferenceId,
		const bool UseSpeech);

	virtual void Activate() override;

	UPROPERTY()
	UPlaythrough* CurPlaythrough;
	UPROPERTY()
	int32 CurConversationId;
	UPROPERTY()
	int32 CurSceneIndex;
	UPROPERTY()
	int32 CurStartGraphId;
	UPROPERTY()
	FString CurStartGraphReferenceId;
	UPROPERTY()
	bool CurUseSpeech;

private:

	UFUNCTION()
	void BroadcastMessage(const FCharismaMessageEvent& MessageEvent);

	UPROPERTY(BlueprintAssignable)
	FPlaythroughMessagesManagerDelegate OnMessage;

	UObject* WorldContextObject;

};
