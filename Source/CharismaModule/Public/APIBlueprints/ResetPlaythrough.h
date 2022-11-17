// Copyright 2022 Charisma Entertainment Ltd

#pragma once

#include "CharismaAPI.h"
#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"

#include "ResetPlaythrough.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FResetPlaythroughDelegate, FString, Error);

UCLASS()
class CHARISMAMODULE_API UResetPlaythrough : public UBlueprintAsyncActionBase
{
	GENERATED_UCLASS_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"),
		Category = "Charisma API")
	static UResetPlaythrough* ResetPlaythrough(UObject* WorldContextObject, const FString Token, const FString EventId);

	virtual void Activate() override;

	UPROPERTY()
	FString Token;

	UPROPERTY()
	FString EventId;

private:
	UPROPERTY(BlueprintAssignable)
	FResetPlaythroughDelegate Success;

	UPROPERTY(BlueprintAssignable)
	FResetPlaythroughDelegate Failure;

	UObject* WorldContextObject;
};