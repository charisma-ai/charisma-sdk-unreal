// Copyright 2022 Charisma Entertainment Ltd

#pragma once

#include "CharismaAPI.h"
#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"

#include "CreateConversation.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCreateConversationDelegate, FString, ConversationUuid, FString, Error);

UCLASS()
class CHARISMAMODULE_API UCreateConversation : public UBlueprintAsyncActionBase
{
	GENERATED_UCLASS_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"),
		Category = "Charisma API")
	static UCreateConversation* CreateConversation(UObject* WorldContextObject, const FString Token);

	virtual void Activate() override;

	UPROPERTY()
	FString Token;

private:
	UPROPERTY(BlueprintAssignable)
	FCreateConversationDelegate Success;

	UPROPERTY(BlueprintAssignable)
	FCreateConversationDelegate Failure;

	UObject* WorldContextObject;
};