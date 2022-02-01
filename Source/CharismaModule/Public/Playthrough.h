#pragma once

#include "CharismaEvents.h"
#include "Client.h"
#include "CoreMinimal.h"
#include "Interfaces/IHttpResponse.h"
#include "Math/UnrealMathUtility.h"
#include "Room.h"
#include "UObject/NoExportTypes.h"

#include "Playthrough.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMessageHistoryDelegate, const FCharismaMessageHistoryResponse&, MessageHistory);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPlaythroughInfoDelegate, const FCharismaPlaythroughInfoResponse&, PlaythroughInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FConnectionDelegate, bool, IsConnected);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTypingDelegate, bool, IsTyping);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMessageDelegate, const FCharismaMessageEvent&, MessageEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FErrorDelegate, const FCharismaErrorEvent&, ErrorEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FReadyDelegate);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CHARISMAMODULE_API UPlaythrough : public UObject
{
	GENERATED_BODY()

public:
	UPlaythrough();
	~UPlaythrough();

	// Member

	UFUNCTION(BlueprintCallable, Category = Playthrough)
	static UPlaythrough* NewPlaythroughObject(UObject* WorldContextObject, const FString& Token, const FString& PlaythroughUuid);

	UFUNCTION(BlueprintCallable, Category = Connection)
	void Connect();

	UFUNCTION(BlueprintCallable, Category = Connection)
	void Disconnect();

	UFUNCTION(BlueprintCallable, Category = Play)
	void Action(const FString& ConversationUuid, const FString& ActionName) const;

	UFUNCTION(BlueprintCallable, Category = Play)
	void Start(const FString& ConversationUuid, const int32 SceneIndex, const int32 StartGraphId,
		const FString& StartGraphReferenceId, const bool UseSpeech = false);

	UFUNCTION(BlueprintCallable, Category = Play)
	void Reply(const FString& ConversationUuid, const FString& Message) const;

	UFUNCTION(BlueprintCallable, Category = Play)
	void Resume(const FString& ConversationUuid) const;

	UFUNCTION(BlueprintCallable, Category = Play)
	void Tap(const FString& ConversationUuid) const;

	UFUNCTION(BlueprintCallable, Category = Play)
	void ToggleSpeechOn();

	UFUNCTION(BlueprintCallable, Category = Play)
	void ToggleSpeechOff();

	UFUNCTION()
	void SaveEmotionsMemories(const TArray<FCharismaEmotion>& Emotions, const TArray<FCharismaMemory>& Memories);

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

	UPROPERTY(BlueprintReadWrite)
	TArray<FCharismaEmotion> PlaythroughEmotions;

	UPROPERTY(BlueprintReadWrite)
	TArray<FCharismaMemory> PlaythroughMemories;

private:
	// Member

	SpeechConfig GetSpeechConfig() const;

	// Properties

	TSharedPtr<Client> ClientInstance;

	TSharedPtr<Room<void>> RoomInstance;

	FString CurToken;

	FString CurPlaythroughUuid;

	UObject* CurWorldContextObject;

	int ReconnectionTryCount = 0;

	bool bCalledByDisconnect = false;

	bool bUseSpeech = false;

	bool bIsPlaying = false;
};
