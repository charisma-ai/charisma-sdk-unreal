#include "Charisma.h"
#include "SocketIOClient.h"

#include "Runtime/Engine/Classes/Sound/SoundWave.h"
#include "Runtime/Engine/Public/VorbisAudioInfo.h"
#include "Developer/TargetPlatform/Public/Interfaces/IAudioFormat.h"
#include "AudioDevice.h"

const FString UCharisma::Base_URL = "https://api.charisma.ai";
const FString UCharisma::Namespace = "/play";

UCharisma::UCharisma()
{
	
}


UCharisma::~UCharisma()
{
	Disconnect();
}

UCharisma* UCharisma::CreateCharismaObject(UObject* Owner)
{
	return NewObject<UCharisma>(Owner);
}

USoundWave* UCharisma::CreateSoundFromBytes(const TArray<uint8> RawBytes)
{
	USoundWave_InMemoryOgg* SoundWaveRef = NewObject<USoundWave_InMemoryOgg>(USoundWave::StaticClass());

	if(!SoundWaveRef)
	{
		Log(-4, "Failed to create new SoundWave object", Error);
		return nullptr;
	}

	if(RawBytes.Num() > 0)
	{
		if(!FillSoundWaveInfo(SoundWaveRef, RawBytes))
		{
			Log(-4, "Something went wrong when loading sound data", Error);
			return nullptr;
		}
	}else
	{
		Log(-4, "Couldn't load sound. Raw Data is empty! Did you toggle speech off?", Error);
		return nullptr;
	}

	FByteBulkData* BulkData = &SoundWaveRef->CompressedFormatData.GetFormat(FName("OGG"));
	
	BulkData->Lock(LOCK_READ_WRITE);
	FMemory::Memmove(BulkData->Realloc(RawBytes.Num()), RawBytes.GetData(), RawBytes.Num());
	BulkData->Unlock();

	SoundWaveRef->SetPrecacheState(ESoundWavePrecacheState::Done);
	return SoundWaveRef;
}

bool UCharisma::FillSoundWaveInfo(USoundWave* SoundWave, TArray<uint8> RawData)
{
	FSoundQualityInfo SoundQualityInfo;
	FVorbisAudioInfo VorbisAudioInfo;
	
	if (!VorbisAudioInfo.ReadCompressedInfo(RawData.GetData(), RawData.Num(), &SoundQualityInfo))
	{
		return false;
	}

	SoundWave->DecompressionType = EDecompressionType::DTYPE_RealTime;
	SoundWave->SoundGroup = ESoundGroup::SOUNDGROUP_Default;
	SoundWave->NumChannels = SoundQualityInfo.NumChannels;
	SoundWave->Duration = SoundQualityInfo.Duration;
	SoundWave->RawPCMDataSize = SoundQualityInfo.SampleDataSize;
	SoundWave->SetSampleRate(SoundQualityInfo.SampleRate);

	return true;
}

void UCharisma::CreatePlayThroughToken(const int32 StoryId, const int32 StoryVersion)
{
	TSharedPtr<FJsonObject> RequestData = MakeShareable(new FJsonObject);
	RequestData->SetNumberField("storyId", StoryId);

	if(StoryVersion != 0)
	{
		RequestData->SetNumberField("version", StoryVersion);
	}

	FString OutputString;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(RequestData.ToSharedRef(), Writer);

	FHttpModule* HttpModule = &FHttpModule::Get();
	TSharedRef<IHttpRequest> HttpRequest = HttpModule->CreateRequest();

	if (StoryVersion == -1 && !DebugToken.IsEmpty())
	{
		HttpRequest->SetHeader("Authorization", "Bearer " + DebugToken);
	}

	if (StoryVersion == -1 && DebugToken.IsEmpty())
	{
		Log(-3, "Please provide a valid debug token", Error);
	}

	HttpRequest->SetVerb("POST");
	HttpRequest->AppendToHeader("Content-Type", "application/json");
	HttpRequest->SetURL(Base_URL + "/play/token/");
	HttpRequest->SetContentAsString(OutputString);
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UCharisma::OnTokenRequestComplete);
	
	HttpRequest->ProcessRequest();
}



void UCharisma::CreateConversation(const FString TokenForConversation)
{
	FHttpModule* HttpModule = &FHttpModule::Get();
	TSharedRef<IHttpRequest> HttpRequest = HttpModule->CreateRequest();

	HttpRequest->SetVerb("POST");
	HttpRequest->SetHeader("Authorization", "Bearer " + TokenForConversation);
	HttpRequest->SetURL(Base_URL + "/play/conversation/");
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UCharisma::OnConversationRequestComplete);
	
	HttpRequest->ProcessRequest();
}

void UCharisma::SetMemory(const FString TokenForSetMemory, const FString RecallValue, const FString SaveValue)
{
	TSharedPtr<FJsonObject> RequestData = MakeShareable(new FJsonObject);
	RequestData->SetStringField("memoryRecallValue", RecallValue);
	RequestData->SetStringField("saveValue", SaveValue);

	FString OutputString;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(RequestData.ToSharedRef(), Writer);

	FHttpModule* HttpModule = &FHttpModule::Get();
	TSharedRef<IHttpRequest> HttpRequest = HttpModule->CreateRequest();

	HttpRequest->SetVerb("POST");
	HttpRequest->SetHeader("Authorization", "Bearer " + TokenForSetMemory);
	HttpRequest->AppendToHeader("Content-Type", "application/json");
	HttpRequest->SetURL(Base_URL + "/play/set-memory/");
	HttpRequest->SetContentAsString(OutputString);
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UCharisma::OnSetMemory);
	
	HttpRequest->ProcessRequest();
}

void UCharisma::SetMood(const FString TokenForSetMood, const FString CharacterName, FMood Mood)
{
	TSharedPtr<FJsonObject> RequestData = MakeShareable(new FJsonObject);
	RequestData->SetStringField("characterName", CharacterName);	
	RequestData->SetObjectField("modifier", FJsonObjectConverter::UStructToJsonObject(Mood, 0, 0));

	FString OutputString;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(RequestData.ToSharedRef(), Writer);
	
	FHttpModule* HttpModule = &FHttpModule::Get();
	TSharedRef<IHttpRequest> HttpRequest = HttpModule->CreateRequest();

	HttpRequest->SetVerb("POST");
	HttpRequest->SetHeader("Authorization", "Bearer " + TokenForSetMood);
	HttpRequest->SetURL(Base_URL + "/play/set-mood/");
	HttpRequest->AppendToHeader("Content-Type", "application/json");
	HttpRequest->SetContentAsString(OutputString);
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UCharisma::OnSetMood);
	
	HttpRequest->ProcessRequest();
}

void UCharisma::OnTokenRequestComplete(const FHttpRequestPtr Request, const FHttpResponsePtr Response, const bool WasSuccessFull) const
{
	if(WasSuccessFull)
	{
		const int32 ResponseCode = Response->GetResponseCode();
		const FString Content = Response->GetContentAsString();

		if(ResponseCode == 200)
		{
			TSharedPtr<FJsonObject> ResponseData = MakeShareable(new FJsonObject);
			
			const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Content);
			if(FJsonSerializer::Deserialize(Reader, ResponseData))
			{
				const FString Token = ResponseData->GetStringField(TEXT("token"));
				
				OnTokenCreated.Broadcast(Token);
			}else
			{
				Log(-2, "Failed to deserialize response data.", Error, 5.f);
			}	
		}else
		{
			if (GEngine)
			{
				TArray<FStringFormatArg> Args;
				Args.Add(FStringFormatArg(FString::FromInt(ResponseCode)));
				Args.Add(FStringFormatArg(Content));
				Log(-2, FString::Format(TEXT("{0}, {1}."), Args), Error, 5.f);
			}
		}
	}
}

void UCharisma::OnConversationRequestComplete(const FHttpRequestPtr Request, const FHttpResponsePtr Response, const bool WasSuccessFull) const
{
	if(WasSuccessFull)
	{
		const int32 ResponseCode = Response->GetResponseCode();
		const FString Content = Response->GetContentAsString();

		if (ResponseCode == 200)
		{
			TSharedPtr<FJsonObject> ResponseData = MakeShareable(new FJsonObject);

			const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Content);
			if (FJsonSerializer::Deserialize(Reader, ResponseData))
			{
				const int32 Conversation = ResponseData->GetNumberField(TEXT("conversationId"));

				OnConversationCreated.Broadcast(Conversation);
			}
			else
			{
				Log(-1, "Failed to deserialize response data.", Error, 5.f);
			}
		}
		else
		{
			TArray<FStringFormatArg> Args;
			Args.Add(FStringFormatArg(FString::FromInt(ResponseCode)));
			Args.Add(FStringFormatArg(Content));
			Log(-1, FString::Format(TEXT("{0}, {1}."), Args), Error, 5.f);
		}
	}
}

void UCharisma::OnSetMemory(FHttpRequestPtr Request, FHttpResponsePtr Response, bool WasSuccessFull) const
{
	if(WasSuccessFull)
	{
		const int32 ResponseCode = Response->GetResponseCode();
		const FString Content = Response->GetContentAsString();

		if(ResponseCode == 200)
		{
			Log(0, Response.Get()->GetContentAsString(), Info);
		}else
		{
			TArray<FStringFormatArg> Args;
			Args.Add(FStringFormatArg(FString::FromInt(ResponseCode)));
			Args.Add(FStringFormatArg(Content));
			Log(0, FString::Format(TEXT("{0}, {1}."), Args), Error, 5.f);
		}
	}
}

void UCharisma::OnSetMood(FHttpRequestPtr Request, FHttpResponsePtr Response, bool WasSuccessFull) const
{
	if (WasSuccessFull)
	{
		const int32 ResponseCode = Response->GetResponseCode();
		const FString Content = Response->GetContentAsString();

		if (ResponseCode == 200)
		{
			Log(0, Response.Get()->GetContentAsString(), Info);
		}
		else
		{
			TArray<FStringFormatArg> Args;
			Args.Add(FStringFormatArg(FString::FromInt(ResponseCode)));
			Args.Add(FStringFormatArg(Content));
			Log(0, FString::Format(TEXT("{0}, {1}."), Args), Error, 5.f);
		}
	}
}

void UCharisma::Connect(const FString TokenForPlay, const bool VerboseLogging)
{
	if(bIsPlaying) return;

	bVerboseLog = VerboseLogging;
        
	Socket = ISocketIOClientModule::Get().NewValidNativePointer();

	if (!Socket.IsValid())
	{
		return;
	}

	TSharedPtr<FJsonObject> QueryParams = MakeShareable(new FJsonObject);
	QueryParams->SetStringField("token", TokenForPlay);

	Socket->VerboseLog = bVerboseLog;
	Socket->Connect(Base_URL, QueryParams, nullptr);
	
	Socket->OnConnectedCallback = [this](const FString& InSessionId)
	{
		Log(1, "Connected.", Info);
		
		Socket->JoinNamespace(Namespace);
		
		OnConnected.Broadcast(true);
	};

	Socket->OnDisconnectedCallback = [this](ESIOConnectionCloseReason Reason)
	{
		OnConnected.Broadcast(false);

		Socket->LeaveNamespace(Namespace);

		if(Socket.IsValid())
		{
			ISocketIOClientModule::Get().ReleaseNativePointer(Socket);
			Socket = nullptr;
		}
	};

	Socket->OnReconnectionCallback = [this](const uint32 AttemptCount, const uint32 DelayInMs)
	{
		Log(2, FString("Reconnecting. Attempt: " + FString::FromInt(AttemptCount)), Warning);
	};

	Socket->OnFailCallback = [this]()
	{
		Log(3, "Failed to connect.", Error);
	};
	
	Socket->OnEvent(TEXT("status"), [this](const FString& Event, const TSharedPtr<FJsonValue>& Message)
	{
		Log(4, "Ready to begin play.", Info);

		OnReady.Broadcast();
		
	}, Namespace);

	Socket->OnEvent(TEXT("start-typing"), [this](const FString& Event, const TSharedPtr<FJsonValue>& Message)
	{
		
		OnTyping.Broadcast(true);
		
	}, Namespace);

	Socket->OnEvent(TEXT("stop-typing"), [this](const FString& Event, const TSharedPtr<FJsonValue>& Message)
	{

		OnTyping.Broadcast(false);

	}, Namespace);
	
	Socket->OnEvent(TEXT("message"), [this](const FString& Event, const TSharedPtr<FJsonValue>& Message)
	{
		TSharedPtr<FJsonObject> ResponseData = MakeShareable(new FJsonObject);
		ResponseData = Message->AsObject();

		OnReceivedMessage.Broadcast(GenerateResponse(ResponseData));
		
	}, Namespace);

	Socket->OnEvent(TEXT("error"), [this](const FString& Event, const TSharedPtr<FJsonValue>& Message)
	{
		Log(5, Message->AsString(), Error);
		
	}, Namespace);

	Socket->OnEvent(TEXT("problem"), [this](const FString& Event, const TSharedPtr<FJsonValue>& Message)
	{
		TSharedPtr<FJsonObject> ErrorData = MakeShareable(new FJsonObject);
		ErrorData = Message->AsObject();

		OnError.Broadcast(GenerateError(ErrorData));
		
	}, Namespace);
}

void UCharisma::Disconnect()
{	
	bIsPlaying = false;

	if (Socket.IsValid())
	{
		if (!Socket->bIsConnected)
		{
			return;
		}
		
		Socket->Disconnect();
	}
}

void UCharisma::Start(const int32 ConversationIdForPlay, const int32 StartFromScene, const bool UseSpeech)
{
	if (!Socket.IsValid())
	{
		return;
	}
	
	if(!Socket->bIsConnected)
	{
		Log(6, "Connect before starting play.", Warning, 5.f);
		return;
	}

	bIsPlaying = true;
	bUseSpeech = UseSpeech;

	TSharedPtr<FJsonObject> StartOptions = MakeShareable(new FJsonObject);
	StartOptions->SetNumberField("conversationId", ConversationIdForPlay);
	StartOptions->SetNumberField("sceneIndex", StartFromScene);
	
	if(bUseSpeech)
	{
		StartOptions->SetObjectField("speechConfig", GetSpeechForEmit());
	}
	
	Socket->Emit("start", StartOptions, nullptr, Namespace);
}

void UCharisma::Tap(const int32 ConversationIdForTap) const
{
	if (!Socket.IsValid())
	{
		return;
	}
	
	if (!Socket->bIsConnected)
	{
		Log(6, "Connect before interacting with charisma.", Warning, 5.f);
		return;
	}

	TSharedPtr<FJsonObject> TapOptions = MakeShareable(new FJsonObject);
	TapOptions->SetNumberField("conversationId", ConversationIdForTap);
	
	if(bUseSpeech)
	{
		TapOptions->SetObjectField("speechConfig", GetSpeechForEmit());
	}
	
	Socket->Emit("tap", TapOptions, nullptr, Namespace);
}

void UCharisma::Reply(const int32 ConversationIdForReply, const FString Message) const
{
	if(!Socket.IsValid())
	{
		return;
	}
	
	if (!Socket->bIsConnected)
	{
		Log(6, "Connect before interacting with charisma.", Warning, 5.f);
		return;
	}

	TSharedPtr<FJsonObject> PlayerMessage = MakeShareable(new FJsonObject);
	PlayerMessage->SetNumberField("conversationId", ConversationIdForReply);
	PlayerMessage->SetStringField("text", Message);

	if(bUseSpeech)
	{
		PlayerMessage->SetObjectField("speechConfig", GetSpeechForEmit());
	}	

	Socket->Emit("reply", PlayerMessage, nullptr, Namespace);
}

void UCharisma::ToggleSpeechOn()
{
	bUseSpeech = true;
}

void UCharisma::ToggleSpeechOff()
{
	bUseSpeech = false;
}


FCharismaResponse UCharisma::GenerateResponse(const TSharedPtr<FJsonObject> ResponseData)
{
	FCharismaResponse Response;
	FJsonObjectConverter::JsonObjectToUStruct<FCharismaResponse>(ResponseData.ToSharedRef(), &Response, 0 ,0);

	if(Response.EndStory) {
	  bIsPlaying = false;
	}

	return Response;
}

FCharismaError UCharisma::GenerateError(const TSharedPtr<FJsonObject> ErrorData) const
{
	FCharismaError Error;
	FJsonObjectConverter::JsonObjectToUStruct<FCharismaError>(ErrorData.ToSharedRef(), &Error, 0, 0);
	
	return Error;
}

TSharedPtr<FJsonObject> UCharisma::GetSpeechForEmit() const
{
	TSharedPtr<FJsonObject> SpeechOptionsForStart = MakeShareable(new FJsonObject);
	SpeechOptionsForStart->SetStringField("encoding", "ogg" );
	SpeechOptionsForStart->SetStringField("audioOutput", "buffer");
	
	return SpeechOptionsForStart;
}


void UCharisma::Log(const int32 Key, const FString Message, const EMessage_Severity Severity, const float Duration)
{
	
	FColor MessageColor;
        switch (Severity)
		{
        case Info:
			MessageColor = FColor::Green;
          break;
        case Warning:
			MessageColor = FColor::Orange;
          break;
        case Error:
			MessageColor = FColor::Red;
          break;
        default: ;
			MessageColor = FColor::White;
        }

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(Key, Duration, MessageColor, FString("Charisma: " + Message));
	}
}
