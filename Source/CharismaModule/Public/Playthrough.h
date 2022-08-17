#pragma once

#include "CharismaEvents.h"
#include "Client.h"
#include "CoreMinimal.h"
#include "Interfaces/IHttpResponse.h"
#include "Math/UnrealMathUtility.h"
#include "MicrophoneCapture.h"
#include "Room.h"
#include "UObject/NoExportTypes.h"

#include "Playthrough.generated.h"

UENUM(BlueprintType, Category = "Charisma Playthrough")
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

	UFUNCTION(BlueprintCallable, Category = "Charisma Playthrough")
	static UPlaythrough* NewPlaythroughObject(UObject* WorldContextObject, const FString& Token, const FString& PlaythroughUuid);

	static const nlohmann::json SdkInfo;

	UFUNCTION(BlueprintCallable, Category = "Charisma Playthrough")
	void Connect();

	UFUNCTION(BlueprintCallable, Category = "Charisma Playthrough")
	void Disconnect();

	UFUNCTION(BlueprintCallable, Category = "Charisma Playthrough")
	void Action(const FString& ConversationUuid, const FString& ActionName) const;

	UFUNCTION(BlueprintCallable, Category = "Charisma Playthrough")
	void Start(const FString& ConversationUuid, const int32 SceneIndex, const int32 StartGraphId,
		const FString& StartGraphReferenceId,
		const ECharismaSpeechAudioFormat SpeechAudioFormat = ECharismaSpeechAudioFormat::None);

	UFUNCTION(BlueprintCallable, Category = "Charisma Playthrough")
	void Reply(const FString& ConversationUuid, const FString& Message) const;

	UFUNCTION(BlueprintCallable, Category = "Charisma Playthrough")
	void Resume(const FString& ConversationUuid) const;

	UFUNCTION(BlueprintCallable, Category = "Charisma Playthrough")
	void Tap(const FString& ConversationUuid) const;

	UFUNCTION(BlueprintCallable, Category = "Charisma Playthrough")
	void ToggleSpeech(const ECharismaSpeechAudioFormat AudioFormat);

	UFUNCTION()
	void SaveEmotionsMemories(const TArray<FCharismaEmotion>& Emotions, const TArray<FCharismaMemory>& Memories);

	UFUNCTION(BlueprintCallable, Category = "Charisma Playthrough")
	void StartSpeechRecognition(bool& bWasSuccessful);

	UFUNCTION(BlueprintCallable, Category = "Charisma Playthrough")
	void StopSpeechRecognition();

	UFUNCTION()
	void ReconnectionFlow();

	UFUNCTION()
	void ReconnectionFlowCreate();

	UFUNCTION()
	void ReconnectionDelay();

	// Events

	UPROPERTY(BlueprintAssignable, Category = Events)
	FConnectionDelegate OnConnected;

	UPROPERTY(BlueprintAssignable, Category = Events)
	FMessageDelegate OnMessage;

	UPROPERTY(BlueprintAssignable, Category = Events)
	FErrorDelegate OnError;

	UPROPERTY(BlueprintAssignable, Category = Events)
	FReadyDelegate OnReady;

	UPROPERTY(BlueprintAssignable, Category = Events)
	FTypingDelegate OnTyping;

	UPROPERTY(BlueprintAssignable, Category = Events)
	FPingSuccessDelegate OnPingSuccess;

	UPROPERTY(BlueprintAssignable, Category = Events)
	FPingFailureDelegate OnPingFailure;

	UPROPERTY(BlueprintAssignable, Category = Events)
	FSpeechRecognitionResultDelegate OnSpeechRecognitionResult;

	UPROPERTY(BlueprintReadWrite)
	TArray<FCharismaEmotion> PlaythroughEmotions;

	UPROPERTY(BlueprintReadWrite)
	TArray<FCharismaMemory> PlaythroughMemories;

	UPROPERTY(BlueprintReadWrite)
	float TimeBetweenPings = 2.0f;

	UPROPERTY(BlueprintReadWrite)
	uint8 MinimumPingsToConsiderFailed = 3;

private:
	// Member

	SpeechConfig GetSpeechConfig(const ECharismaSpeechAudioFormat AudioFormat) const;

	void OnRoomJoined(TSharedPtr<Room<void>> Room);

	UFUNCTION()
	void FirePing();

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
