#pragma once

#include "CharismaAPI.h"
#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"

#include "RestartFromEventId.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRestartFromEventIdDelegate, FString, Error);

UCLASS()
class CHARISMAMODULE_API URestartFromEventId : public UBlueprintAsyncActionBase
{
	GENERATED_UCLASS_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"),
		Category = "Charisma API")
	static URestartFromEventId* RestartFromEventId(UObject* WorldContextObject, const FString Token, const int64 EventId);

	virtual void Activate() override;

	UPROPERTY()
	FString Token;

	UPROPERTY()
	int64 EventId;

private:
	UPROPERTY(BlueprintAssignable)
	FRestartFromEventIdDelegate Success;

	UPROPERTY(BlueprintAssignable)
	FRestartFromEventIdDelegate Failure;

	UObject* WorldContextObject;
};