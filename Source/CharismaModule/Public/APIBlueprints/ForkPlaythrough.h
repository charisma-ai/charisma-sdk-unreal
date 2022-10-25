// Copyright 2022 Charisma Entertainment Ltd

#pragma once

#include "CharismaAPI.h"
#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"

#include "ForkPlaythrough.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FForkPlaythroughDelegate,FString, Token, FString, PlaythroughUuid, FString, Error);

UCLASS()
class CHARISMAMODULE_API UForkPlaythrough : public UBlueprintAsyncActionBase
{
	GENERATED_UCLASS_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"),
		Category = "Charisma API")
	static UForkPlaythrough* ForkPlaythrough(UObject* WorldContextObject, const FString Token);

	virtual void Activate() override;

	UPROPERTY()
	FString Token;

private:
	UPROPERTY(BlueprintAssignable)
	FForkPlaythroughDelegate Success;

	UPROPERTY(BlueprintAssignable)
	FForkPlaythroughDelegate Failure;

	UObject* WorldContextObject;
};