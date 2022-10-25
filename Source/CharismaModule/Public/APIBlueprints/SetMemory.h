// Copyright 2022 Charisma Entertainment Ltd

#pragma once

#include "CharismaAPI.h"
#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"

#include "SetMemory.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSetMemoryDelegate, FString, Error);

UCLASS()
class CHARISMAMODULE_API USetMemory : public UBlueprintAsyncActionBase
{
	GENERATED_UCLASS_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"),
		Category = "Charisma API")
	static USetMemory* SetMemory(
		UObject* WorldContextObject, const FString Token, const FString RecallValue, const FString SaveValue);

	virtual void Activate() override;

	UPROPERTY()
	FString Token;

	UPROPERTY()
	FString RecallValue;

	UPROPERTY()
	FString SaveValue;

private:
	UPROPERTY(BlueprintAssignable)
	FSetMemoryDelegate Success;

	UPROPERTY(BlueprintAssignable)
	FSetMemoryDelegate Failure;

	UObject* WorldContextObject;
};