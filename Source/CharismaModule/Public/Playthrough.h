#pragma once

#include "CharismaEvents.h"
#include "Client.h"
#include "CoreMinimal.h"
#include "Interfaces/IHttpResponse.h"
#include "Math/UnrealMathUtility.h"
#include "MicrophoneCapture.h"
#include "Room.h"
#include "UObject/NoExportTypes.h"
#include "Engine/EngineTypes.h"

#include "Playthrough.generated.h"

UENUM(BlueprintType, Category = "Charisma|Playthrough")
enum class ECharismaSpeechAudioFormat : uint8
{
	None UMETA(DisplayName = "No speech"),
	Mp3 UMETA(DisplayName = "mp3"),
	Wav UMETA(DisplayName = "wav"),
	Pcm UMETA(DisplayName = "pcm"),
	Ogg UMETA(DisplayName = "ogg")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMessageHistoryDelegate, const FCharismaMessageHistoryResponse&, MessageHistory);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPlaythroughInfoDelegate, const FCharismaPlaythroughInfoResponse&, PlaythroughInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FConnectionDelegate, bool, IsConnected);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTypingDelegate, bool, IsTyping);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMessageDelegate, const FCharismaMessageEvent&, MessageEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FErrorDelegate, const FCharismaErrorEvent&, ErrorEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FReadyDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPingSuccessDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPingFailureDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSpeechRecognitionResultDelegate, const FString&, Transcript, bool, IsFinal);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CHARISMAMODULE_API UPlaythrough : public UObject
{
	GENERATED_BODY()

public:
	UPlaythrough();
	~UPlaythrough();

	// Member

	UFUNCTION(BlueprintCallable, Category = "Charisma|Playthrough")
	static UPlaythrough* NewPlaythroughObject(UObject* WorldContextObject, const FString& Token, const FString& PlaythroughUuid);

	static const nlohmann::json SdkInfo;

	UFUNCTION(BlueprintCallable, Category = "Charisma|Playthrough")
	void Connect();

	UFUNCTION(BlueprintCallable, Category = "Charisma|Playthrough")
	void Disconnect();

	UFUNCTION(BlueprintCallable, Category = "Charisma|Playthrough Events")
	void Action(const FString& ConversationUuid, const FString& ActionName) const;

	UFUNCTION(BlueprintCallable, Category = "Charisma|Playthrough Events")
	void Start(const FString& ConversationUuid, const int32 SceneIndex, const int32 StartGraphId,
		const FString& StartGraphReferenceId,
		const ECharismaSpeechAudioFormat SpeechAudioFormat = ECharismaSpeechAudioFormat::None);

	UFUNCTION(BlueprintCallable, Category = "Charisma|Playthrough Events")
	void Reply(const FString& ConversationUuid, const FString& Message) const;

	UFUNCTION(BlueprintCallable, Category = "Charisma|Playthrough Events")
	void Resume(const FString& ConversationUuid) const;

	UFUNCTION(BlueprintCallable, Category = "Charisma|Playthrough Events")
	void Tap(const FString& ConversationUuid) const;

	UFUNCTION(BlueprintCallable, Category = "Charisma|Playthrough Events")
	void ToggleSpeech(const ECharismaSpeechAudioFormat AudioFormat);

	UFUNCTION(BlueprintCallable, Category = "Charisma|Playthrough Events")
	void StartSpeechRecognition(bool& bWasSuccessful);

	UFUNCTION(BlueprintCallable, Category = "Charisma|Playthrough Events")
	void StopSpeechRecognition();

	// Events

	UPROPERTY(BlueprintAssignable, Category = "Charisma|Playthrough Events")
	FConnectionDelegate OnConnected;

	UPROPERTY(BlueprintAssignable, Category = "Charisma|Playthrough Events")
	FMessageDelegate OnMessage;

	UPROPERTY(BlueprintAssignable, Category = "Charisma|Playthrough Events")
	FErrorDelegate OnError;

	UPROPERTY(BlueprintAssignable, Category = "Charisma|Playthrough Events")
	FReadyDelegate OnReady;

	UPROPERTY(BlueprintAssignable, Category = "Charisma|Playthrough Events")
	FTypingDelegate OnTyping;

	UPROPERTY(BlueprintAssignable, Category = "Charisma|Playthrough Events")
	FPingSuccessDelegate OnPingSuccess;

	UPROPERTY(BlueprintAssignable, Category = "Charisma|Playthrough Events")
	FPingFailureDelegate OnPingFailure;

	UPROPERTY(BlueprintAssignable, Category = "Charisma|Playthrough Events")
	FSpeechRecognitionResultDelegate OnSpeechRecognitionResult;

	UPROPERTY(BlueprintReadWrite, Category = "Charisma|Playthrough")
	TArray<FCharismaEmotion> PlaythroughEmotions;

	UPROPERTY(BlueprintReadWrite, Category = "Charisma|Playthrough")
	TArray<FCharismaMemory> PlaythroughMemories;

	UPROPERTY(BlueprintReadWrite, Category = "Charisma|Playthrough")
	float TimeBetweenPings = 2.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Charisma|Playthrough")
	uint8 MinimumPingsToConsiderFailed = 3;

private:
	UFUNCTION()
	void ReconnectionFlow();

	UFUNCTION()
	void ReconnectionFlowCreate();

	UFUNCTION()
	void ReconnectionDelay();

	UFUNCTION()
	void FirePing();

	// Member

	SpeechConfig GetSpeechConfig(const ECharismaSpeechAudioFormat AudioFormat) const;

	void OnRoomJoined(TSharedPtr<Room<void>> Room);

	// Properties

	TSharedPtr<Client> ClientInstance;

	TSharedPtr<Room<void>> RoomInstance;

	FString CurToken;

	FString CurPlaythroughUuid;

	UObject* CurWorldContextObject;

	uint8 PingCount = 0;

	FTimerHandle PingTimerHandle;

	uint8 ReconnectionTryCount = 0;

	bool bCalledByDisconnect = false;

	ECharismaSpeechAudioFormat SpeechAudioFormat = ECharismaSpeechAudioFormat::None;

	bool bIsPlaying = false;

	TSharedPtr<UMicrophoneCapture> MicrophoneCaptureInstance;
};
