// Copyright 2022 Charisma Entertainment Ltd

#pragma once

#include "CharismaAPI.h"
#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"

#include "GetPlaythroughInfo.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FGetPlaythroughInfoDelegate, FCharismaPlaythroughInfoResponse, PlaythroughInfo, FString, Error);

UCLASS()
class CHARISMAMODULE_API UGetPlaythroughInfo : public UBlueprintAsyncActionBase
{
	GENERATED_UCLASS_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"),
		Category = "Charisma API")
	static UGetPlaythroughInfo* GetPlaythroughInfo(UObject* WorldContextObject, const FString Token);

	virtual void Activate() override;

	UPROPERTY()
	FString Token;

private:
	UPROPERTY(BlueprintAssignable)
	FGetPlaythroughInfoDelegate Success;

	UPROPERTY(BlueprintAssignable)
	FGetPlaythroughInfoDelegate Failure;

	UObject* WorldContextObject;
};