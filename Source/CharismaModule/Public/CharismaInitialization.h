// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"

#include "CharismaAPI.h"
#include "Playthrough.h"

#include "CharismaInitialization.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FCharismaInitializationDelegate, const UPlaythrough*, Playthrough, FString, Token, FString, PlaythroughUuid);

UCLASS()
class CHARISMAMODULE_API UCharismaInitialization : public UBlueprintAsyncActionBase
{
	GENERATED_UCLASS_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"), Category = "CharismaInitialization")
	static UCharismaInitialization* CharismaInitialization(UObject* WorldContextObject, const int32 StoryId, const int32 StoryVersion, const FString& ApiKey);

	virtual void Activate() override;

	UPROPERTY()
	int32 InitStoryId;
	UPROPERTY()
	int32 InitStoryVersion;
	UPROPERTY()
	FString InitApiKey;


private:

	UPROPERTY(BlueprintAssignable)
	FCharismaInitializationDelegate Success;
	
	UPROPERTY(BlueprintAssignable)
	FCharismaInitializationDelegate Failure;

	UObject* WorldContextObject;
	
};
