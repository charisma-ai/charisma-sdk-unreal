// Copyright 2022 Charisma Entertainment Ltd

#pragma once

#include "CharismaAPI.h"
#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"

#include "GetMessageHistory.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FGetMessageHistoryDelegate, FCharismaMessageHistoryResponse, MessageHistory, FString, Error);

UCLASS()
class CHARISMAMODULE_API UGetMessageHistory : public UBlueprintAsyncActionBase
{
	GENERATED_UCLASS_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"),
		Category = "Charisma API")
	static UGetMessageHistory* GetMessageHistory(
		UObject* WorldContextObject, const FString Token, const FString ConversationUuid, const FString EventId);

	virtual void Activate() override;

	UPROPERTY()
	FString Token;

	UPROPERTY()
	FString ConversationUuid;

	UPROPERTY()
	FString MinEventId;

private:
	UPROPERTY(BlueprintAssignable)
	FGetMessageHistoryDelegate Success;

	UPROPERTY(BlueprintAssignable)
	FGetMessageHistoryDelegate Failure;

	UObject* WorldContextObject;
};