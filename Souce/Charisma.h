#pragma once

#include "CoreMinimal.h"

#include "Http.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Json.h"
#include "JsonUtilities.h"
#include "SocketIONative.h"

#include "Audio.h"
#include "Developer/TargetPlatform/Public/Interfaces/IAudioFormat.h"

#include "Charisma.generated.h"

/**
 * 
 */

// Response params

USTRUCT(BlueprintType)
struct FCharismaMemory
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadWrite)
	FString MemoryRecallValue;

	UPROPERTY(BlueprintReadWrite)
	FString SaveValue;
};

USTRUCT(BlueprintType)
struct FMood
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadWrite)
	int32 Anger;
	
	UPROPERTY(BlueprintReadWrite)
	int32 Trust;
	
	UPROPERTY(BlueprintReadWrite)
	int32 Patience;
	
	UPROPERTY(BlueprintReadWrite)
	int32 Happiness;
	
	UPROPERTY(BlueprintReadWrite)
	int32 Fearlessness;
};

USTRUCT(BlueprintType)
struct FCharacterMood
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 Id;
	
	UPROPERTY(BlueprintReadOnly)
	FString Name;
	
	UPROPERTY(BlueprintReadOnly)
	FMood Mood;
};

USTRUCT(BlueprintType)
struct FSpeechAudio
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadOnly)
	TArray<uint8> Data;
	
	UPROPERTY(BlueprintReadOnly)
	FString Type;
	
	UPROPERTY(BlueprintReadOnly)
	FString Url;
};

USTRUCT(BlueprintType)
struct FSpeech
{
	GENERATED_USTRUCT_BODY()
	
	UPROPERTY(BlueprintReadOnly)
	FSpeechAudio Audio;
	
	UPROPERTY(BlueprintReadOnly)
	float Duration;
};

USTRUCT(BlueprintType)
struct FCharismaCharacter
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 Id;
	
	UPROPERTY(BlueprintReadOnly)
	FString Name;
	
	UPROPERTY(BlueprintReadOnly)
	FString Avatar;	
};

USTRUCT(BlueprintType)
struct FMessage
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString Text;
	
	UPROPERTY(BlueprintReadOnly)
	TMap<FString, FString> Metadata;
	
	UPROPERTY(BlueprintReadOnly)
	FCharismaCharacter Character;

	UPROPERTY(BlueprintReadOnly)
	FSpeech Speech;
};

UENUM(BlueprintType)
enum EMessageType
{
	Media,
	Character
};

// Response

USTRUCT(BlueprintType)
struct FCharismaResponse
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 ConversationId;
	
	UPROPERTY(BlueprintReadOnly)
	TEnumAsByte<EMessageType> MessageType;
	
	UPROPERTY(BlueprintReadOnly)
	FMessage Message;
	
	UPROPERTY(BlueprintReadOnly)
	bool EndStory;
	
	UPROPERTY(BlueprintReadOnly)
	bool TapToContinue;
	
	UPROPERTY(BlueprintReadOnly)
	TArray<FCharacterMood> CharacterMoods;
	
	UPROPERTY(BlueprintReadOnly)
	TArray<FCharismaMemory> Memories;
	
	UPROPERTY(BlueprintReadOnly)
	int64 EventId;
	
	uint64 Timestamp;
};

// Charisma

USTRUCT(BlueprintType)
struct FCharismaError
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString Error;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTokenDelegate, FString, Token);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FConversationDelegate, int32, ConversationId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInteractionDelegate, FCharismaResponse, Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FConnectionDelegate, bool, IsConnected);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTypingDelegate, bool, IsTyping);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCharismaErrorDelegate, FCharismaError, CharismaError);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FReadyDelegate);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CHARISMA_API UCharisma : public UObject
{
	GENERATED_BODY()

public:

	UCharisma();
	~UCharisma();

	// Static
	
	static const FString Base_URL;
	static const FString Namespace;

	UFUNCTION(BlueprintCallable, Category=Charisma)
	static UCharisma* CreateCharismaObject(UObject* Owner);

	UFUNCTION(BlueprintCallable, Category = Charisma)
	static USoundWave* CreateSoundFromBytes(TArray<uint8> RawBytes);
	
	enum EMessage_Severity {
		Info,
		Warning,
		Error
	};

	static void Log(int32 Key, FString Message, EMessage_Severity Severity, float Duration = 5.f);

	// Member

	UFUNCTION(BlueprintCallable, Category = Setup)
	void CreatePlayThroughToken(int32 StoryId, int32 StoryVersion = 0);

	UFUNCTION(BlueprintCallable, Category = Setup)
	void CreateConversation(FString TokenForConversation);

	UFUNCTION(BlueprintCallable, Category = Interaction)
	void SetMemory(FString TokenForSetMemory, FString RecallValue, FString SaveValue);

	UFUNCTION(BlueprintCallable, Category = Interaction)
	void SetMood(FString TokenForSetMood, FString CharacterName, FMood Mood);

    	UFUNCTION(BlueprintCallable, Category = Connection)
	void Connect(FString TokenForPlay, bool VerboseLogging = false);

	UFUNCTION(BlueprintCallable, Category = Connection)
	void Disconnect();

	UFUNCTION(BlueprintCallable, Category = Play)
	void Start(int32 ConversationId, int32 StartFromScene, bool UseSpeech = false);

	UFUNCTION(BlueprintCallable, Category = Play)
	void Tap(int32 ConversationIdForTap) const;

	UFUNCTION(BlueprintCallable, Category = Play)
	void Reply(int32 ConversationIdForReply, FString Message) const;

	UFUNCTION(BlueprintCallable, Category = Play)
	void ToggleSpeechOn();

	UFUNCTION(BlueprintCallable, Category = Play)
	void ToggleSpeechOff();
	
	// Properties
	
	UPROPERTY(BlueprintReadWrite)
	FString PlayThroughToken;

	UPROPERTY(BlueprintReadWrite)
	FString DebugToken;

	UPROPERTY(BlueprintReadWrite)
	int32 ConversationId;

	// Events

	UPROPERTY(BlueprintAssignable, Category = Events)
	FTokenDelegate OnTokenCreated;

	UPROPERTY(BlueprintAssignable, Category = Events)
	FConversationDelegate OnConversationCreated;

	UPROPERTY(BlueprintAssignable, Category = Events)
	FConnectionDelegate OnConnected;

	UPROPERTY(BlueprintAssignable, Category = Events)
	FReadyDelegate OnReady;

	UPROPERTY(BlueprintAssignable, Category = Events)
	FTypingDelegate OnTyping;

	UPROPERTY(BlueprintAssignable, Category = Events)
	FInteractionDelegate OnReceivedMessage;

	UPROPERTY(BlueprintAssignable, Category = Events)
	FCharismaErrorDelegate OnError;

private:

	// Static

	static bool FillSoundWaveInfo(USoundWave* SoundWave, TArray<uint8> RawData);

	// Member
	
	void OnTokenRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool WasSuccessFull) const;
	
	void OnConversationRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool WasSuccessFull) const;
	
	void OnSetMemory(FHttpRequestPtr Request, FHttpResponsePtr Response, bool WasSuccessFull) const;

	void OnSetMood(FHttpRequestPtr Request, FHttpResponsePtr Response, bool WasSuccessFull) const;
	
	FCharismaResponse GenerateResponse(TSharedPtr<FJsonObject> ResponseData);
	
	FCharismaError GenerateError(TSharedPtr<FJsonObject> ErrorData) const;
	
	TSharedPtr<FJsonObject> GetSpeechForEmit() const;

	// Properties
	
	TSharedPtr<FSocketIONative> Socket;

	bool bUseSpeech = false;
	
	bool bIsPlaying = false;
	
	bool bVerboseLog = false;
};

UCLASS()
class USoundWave_InMemoryOgg : public USoundWave
{
	GENERATED_BODY()
	
public:

	virtual bool HasCompressedData(FName Format, ITargetPlatform* TargetPlatform) const override
	{
		// Default implementation appends a hash to the format name which is specific to the platform,
		// This is what was causing OGG data to not be found. E.g. on my system it was OGG10000-1-1-1-1-1
		if (Format != OggFormatName)
		{
			return false;
		}

		return CompressedFormatData.Contains(Format);
	}

	virtual void BeginGetCompressedData(FName Format, const FPlatformAudioCookOverrides* CompressionOverrides) override
	{
		// Do nothing, data is already in memory
	}

	virtual FByteBulkData* GetCompressedData(FName Format, const FPlatformAudioCookOverrides* CompressionOverrides = nullptr) override
	{
		if (Format == OggFormatName)
		{
			return &CompressedFormatData.GetFormat(Format);
		}
		return nullptr;
	}

	virtual void InitAudioResource(FByteBulkData& CompressedData) override
	{
		// Should never be pushing additional compressed data to a this
		check(false);
	}

private:
	FName OggFormatName = FName(TEXT("OGG"));
};
