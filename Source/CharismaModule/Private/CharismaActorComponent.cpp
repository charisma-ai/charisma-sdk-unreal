// Fill out your copyright notice in the Description page of Project Settings.


#include "CharismaActorComponent.h"

// Sets default values for this component's properties
UCharismaActorComponent::UCharismaActorComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UCharismaActorComponent::BeginPlay()
{
	Super::BeginPlay();

	AGameModeBase* GameMode = UGameplayStatics::GetGameMode(this);
	GameModeComponent = GameMode->FindComponentByClass<UCharismaGameModeComponent>();

	Playthrough = GameModeComponent->Playthrough;

	if (!Playthrough)
	{
		GetWorld()->GetTimerManager().SetTimer(Timer, this, &UCharismaActorComponent::BindToPlaythrough, 0.2f, false);
	}
	else
	{
		Playthrough->OnMessage.AddDynamic(this, &UCharismaActorComponent::UpdateCharacterEmotion);
	}
}

void UCharismaActorComponent::BindToPlaythrough()
{
	
	if (!Playthrough)
	{
		Playthrough = GameModeComponent->Playthrough;
		if (Playthrough)
		{
			Playthrough->OnMessage.AddDynamic(this, &UCharismaActorComponent::UpdateCharacterEmotion);
		}
		else
		{
			GetWorld()->GetTimerManager().SetTimer(Timer, this, &UCharismaActorComponent::BindToPlaythrough, 0.2f, false);
		}
	}
}

void UCharismaActorComponent::UpdateCharacterEmotion(const FCharismaMessageEvent& Message)
{
	 
	if (CharacterName == Message.Message.Character.Name)
	{
		
		CharacterEmotions = Message.Emotions;

	}

}