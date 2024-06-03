#pragma once

#include "CoreMinimal.h"
#include "CharismaConnectionParams.generated.h"

USTRUCT(BlueprintType)
struct CHARISMAMODULE_API FCharismaConnectionParams
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Charisma|GameMode")
	int StoryId;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Charisma|GameMode")
	int StoryVersion;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Charisma|GameMode")
	FString ApiKey;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Charisma|GameMode")
	FString StartGraphRefId;
};
