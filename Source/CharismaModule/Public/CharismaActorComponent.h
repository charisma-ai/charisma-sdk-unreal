#pragma once

#include "CharismaAudio.h"
#include "CharismaGameModeComponent.h"
#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/GameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Playthrough.h"

#include "CharismaActorComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
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

	UPROPERTY()
	UAudioComponent* CharacterAudioComponent;

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
