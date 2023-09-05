// Copyright 2022 Charisma Entertainment Ltd

#include "CharismaActorComponent.h"

#include "Engine/World.h"
#include "TimerManager.h"

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

	CharacterAudioComponent = GetOwner()->FindComponentByClass<UAudioComponent>();

	AGameModeBase* GameMode = UGameplayStatics::GetGameMode(this);
	GameModeComponent = GameMode->FindComponentByClass<UCharismaGameModeComponent>();

	BindToPlaythrough();

	RuntimeAudioImporterLibrary = URuntimeAudioImporterLibrary::CreateRuntimeAudioImporter();
	RuntimeAudioImporterLibrary->OnResult.AddDynamic(this, &UCharismaActorComponent::OnAudioDecoded);
}

void UCharismaActorComponent::BindToPlaythrough()
{
	Playthrough = GameModeComponent->Playthrough;
	if (Playthrough)
	{
		Playthrough->OnMessage.AddDynamic(this, &UCharismaActorComponent::OnMessageReceived);
	}
	else
	{
		GetWorld()->GetTimerManager().SetTimer(Timer, this, &UCharismaActorComponent::BindToPlaythrough, 0.2f, false);
	}
}

void UCharismaActorComponent::OnMessageReceived(const FCharismaMessageEvent& Message)
{
	if (CharacterName == Message.Message.Character.Name)
	{
		CharacterEmotions = Message.Emotions;

		if (bPlayAudio && CharacterAudioComponent && Message.Message.Speech.Audio.Num() > 0)
		{
			RuntimeAudioImporterLibrary->ImportAudioFromBuffer(Message.Message.Speech.Audio, ERuntimeAudioFormat::Auto);
		}
	}
}

void UCharismaActorComponent::OnAudioDecoded(
	URuntimeAudioImporterLibrary* RuntimeAudioImporterLibraryRef, UImportedSoundWave* SoundWaveRef, ERuntimeImportStatus Status)
{
	if (Status == ERuntimeImportStatus::SuccessfulImport)
	{
		CharacterAudioComponent->SetSound(SoundWaveRef);
		CharacterAudioComponent->Play();
	}
}