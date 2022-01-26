// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "Playthrough.h"
#include "Kismet/GameplayStatics.h"
#include "CharismaGameModeComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/GameModeBase.h"
#include "CharismaAudio.h"

#include "CharismaActorComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CHARISMAMODULE_API UCharismaActorComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCharismaActorComponent();

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString CharacterName;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool PlayAudio = false;

	UPROPERTY(BlueprintReadWrite)
	TArray<FCharismaEmotion> CharacterEmotions;

	UPROPERTY()
	UPlaythrough* Playthrough;

	UPROPERTY()
	FTimerHandle Timer;

	UPROPERTY()
	bool LoopInit = true;

	UPROPERTY()
	UCharismaGameModeComponent* GameModeComponent;

	UFUNCTION()
	void MessageReceived(const FCharismaMessageEvent& Message);

	UFUNCTION()
	void BindToPlaythrough();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

		
};
