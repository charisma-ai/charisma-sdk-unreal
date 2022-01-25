// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "Playthrough.h"

#include "CharismaGameModeComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CHARISMAMODULE_API UCharismaGameModeComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

	UPROPERTY(BlueprintReadWrite)
	UPlaythrough* Playthrough;
		
};
