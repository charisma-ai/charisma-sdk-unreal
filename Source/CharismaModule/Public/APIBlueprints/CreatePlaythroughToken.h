// Copyright 2022 Charisma Entertainment Ltd

#pragma once

#include "CharismaAPI.h"
#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Playthrough.h"

#include "CreatePlaythroughToken.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(
	FCreatePlaythroughTokenDelegate, const UPlaythrough*, Playthrough, FString, Token, FString, PlaythroughUuid, FString, Error);

UCLASS()
class CHARISMAMODULE_API UCreatePlaythroughToken : public UBlueprintAsyncActionBase
{
	GENERATED_UCLASS_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"),
		Category = "Charisma API")
	static UCreatePlaythroughToken* CreatePlaythroughToken(
		UObject* WorldContextObject, const int32 StoryId, const int32 StoryVersion, const FString& ApiKey);

	virtual void Activate() override;

	UPROPERTY()
	int32 StoryId;
	UPROPERTY()
	int32 StoryVersion;
	UPROPERTY()
	FString ApiKey;

private:
	UPROPERTY(BlueprintAssignable)
	FCreatePlaythroughTokenDelegate Success;

	UPROPERTY(BlueprintAssignable)
	FCreatePlaythroughTokenDelegate Failure;

	UObject* WorldContextObject;
};
