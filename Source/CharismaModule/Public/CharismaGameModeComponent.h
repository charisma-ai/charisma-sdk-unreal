// Copyright 2022 Charisma Entertainment Ltd

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "Playthrough.h"

#include "CharismaGameModeComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CHARISMAMODULE_API UCharismaGameModeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category = "Charisma|GameMode")
	UPlaythrough* Playthrough;
};
