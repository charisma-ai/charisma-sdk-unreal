// Copyright 2022 Charisma Entertainment Ltd

#include "Playthrough.h"

#include "CharismaAPI.h"
#include "CharismaLogger.h"
#include "Json.h"
#include "JsonUtilities.h"
#include "TimerManager.h"

#include <nlohmann/json.hpp>
#include <sstream>

UPlaythrough::UPlaythrough()
{
}

UPlaythrough::~UPlaythrough()
{
	Disconnect();
}

UPlaythrough* UPlaythrough::NewPlaythroughObject(UObject* WorldContextObject, const FString& Token, const FString& PlaythroughUuid)
{
	UPlaythrough* Playthrough = NewObject<UPlaythrough>(WorldContextObject);
	Playthrough->CurWorldContextObject = WorldContextObject;
	Playthrough->CurToken = Token;
	Playthrough->CurPlaythroughUuid = PlaythroughUuid;
	return Playthrough;
}

const nlohmann::json UPlaythrough::SdkInfo = {{"sdkId", "unreal"}, {"sdkVersion", "0.9.0"}, {"protocolVersion", 1}};

void UPlaythrough::FirePing()
{
	this->RoomInstance->Send("ping");

	PingCount++;
	if (PingCount >= MinimumPingsToConsiderFailed)
	{
		CharismaLogger::Log(1, "Ping timed out", CharismaLogger::Error);

		ChangeConnectionState(ECharismaPlaythroughConnectionState::Reconnecting);

		OnPingFailure.Broadcast();
	}
}

void UPlaythrough::OnRoomJoined(TSharedPtr<Room<void>> Room)
{
	ReconnectionTryCount = 0;

	this->RoomInstance = Room;

	this->RoomInstance->OnMessage("status",
		[this](const msgpack::object& message) { ChangeConnectionState(ECharismaPlaythroughConnectionState::Connected); });

	this->RoomInstance->OnMessage("message",
		[this](const msgpack::object& message)
		{
			FCharismaMessageEvent Event = message.as<FCharismaMessageEvent>();

			if (Event.Message.Character_Optional.IsSet())
			{
				Event.Message.Character = Event.Message.Character_Optional.GetValue();
			}

			if (Event.Message.Speech_Optional.IsSet())
			{
				Event.Message.Speech = Event.Message.Speech_Optional.GetValue();
			}

			for (FCharismaMemory& Memory : Event.Memories)
			{
				if (Memory.SaveValue_Optional.IsSet())
				{
					Memory.SaveValue = Memory.SaveValue_Optional.GetValue();
				}
			}

			CharismaLogger::Log(-1, Event.Message.Character.Name + TEXT(": ") + Event.Message.Text, CharismaLogger::Info);

			this->PlaythroughEmotions = Event.Emotions;
			this->PlaythroughMemories = Event.Memories;

			OnMessage.Broadcast(Event);
		});

	this->RoomInstance->OnMessage("start-typing", [this](const msgpack::object& message) { OnTyping.Broadcast(true); });
	this->RoomInstance->OnMessage("stop-typing", [this](const msgpack::object& message) { OnTyping.Broadcast(false); });

	this->RoomInstance->OnMessage("problem",
		[this](const msgpack::object& message)
		{
			FCharismaErrorEvent Event = message.as<FCharismaErrorEvent>();

			CharismaLogger::Log(-1, Event.Error, CharismaLogger::Error);

			OnError.Broadcast(Event);
		});

	this->RoomInstance->OnMessage("pong",
		[this](const msgpack::object& message)
		{
			PingCount = 0;
			OnPingSuccess.Broadcast();
			ChangeConnectionState(ECharismaPlaythroughConnectionState::Connected);
		});

	this->RoomInstance->OnMessage("speech-recognition-error",
		[this](const msgpack::object& message)
		{
			FSpeechRecognitionErrorResult Event = message.as<FSpeechRecognitionErrorResult>();
			CharismaLogger::Log(-1,
				TEXT("Speech recognition error: <") + StdStringToFString(Event.Message) + TEXT(">, error when: ") +
					StdStringToFString(Event.ErrorOccuredWhen),
				CharismaLogger::Error);
		});

	this->RoomInstance->OnMessage("speech-recognition-result",
		[this](const msgpack::object& message)
		{
			FCharismaSpeechRecognitionResultEvent Event = message.as<FCharismaSpeechRecognitionResultEvent>();

			if (!Event.Text.IsEmpty())
			{
				OnSpeechRecognitionResult.Broadcast(Event.Text, Event.IsFinal);
			}
		});

	this->RoomInstance->OnLeave = ([this](int32 StatusCode) { this->ReconnectionFlow(); });

	this->RoomInstance->OnError =
		([this](int32 StatusCode, const FString& Error) { CharismaLogger::Log(-1, Error, CharismaLogger::Error); });

	UWorld* World = GEngine->GetWorldFromContextObject(CurWorldContextObject, EGetWorldErrorMode::ReturnNull);
	if (World != nullptr)
	{
		World->GetTimerManager().SetTimer(PingTimerHandle, this, &UPlaythrough::FirePing, TimeBetweenPings, true);
	}
}

void UPlaythrough::Connect()
{
	// Only start the connection flow if we're disconnected
	if (ConnectionState != ECharismaPlaythroughConnectionState::Disconnected)
	{
		CharismaLogger::Log(1, TEXT("Playthrough is already connecting or connected."), CharismaLogger::Warning);
		return;
	}

	ChangeConnectionState(ECharismaPlaythroughConnectionState::Connecting);

	ClientInstance = MakeShared<Client>(UCharismaAPI::SocketURL);
	ClientInstance->JoinOrCreate<void>("chat",
		{{"token", FStringToStdString(CurToken)}, {"playthroughId", FStringToStdString(CurPlaythroughUuid)},
			{"sdkInfo", UPlaythrough::SdkInfo}},
		[this](TSharedPtr<MatchMakeError> Error, TSharedPtr<Room<void>> Room)
		{
			if (Error)
			{
				this->ReconnectionFlowCreate();
				return;
			}

			this->OnRoomJoined(Room);
		});
}

void UPlaythrough::Disconnect()
{
	ChangeConnectionState(ECharismaPlaythroughConnectionState::Disconnected);

	bCalledByDisconnect = true;

	if (RoomInstance.IsValid())
	{
		RoomInstance->Leave();
		RoomInstance = nullptr;
	}
	if (ClientInstance.IsValid())
	{
		ClientInstance = nullptr;
	}
}

void UPlaythrough::Action(const FString& ConversationUuid, const FString& ActionName) const
{
	if (!RoomInstance || ConnectionState != ECharismaPlaythroughConnectionState::Connected)
	{
		CharismaLogger::Log(6, "Charisma must be connected to before sending events.", CharismaLogger::Warning);
		return;
	}

	ActionPayload payload;
	payload.conversationUuid = FStringToStdString(ConversationUuid);
	payload.action = FStringToStdString(ActionName);

	if (SpeechAudioFormat.Num() > 0)
	{
		payload.speechConfig = GetSpeechConfig(SpeechAudioFormat);
	}

	RoomInstance->Send("action", payload);
}

void UPlaythrough::Start(const FString& ConversationUuid, const int32 SceneIndex, const int32 StartGraphId,
	const FString& StartGraphReferenceId, const TArray<ECharismaSpeechAudioFormat> AudioFormat)
{
	if (!RoomInstance || ConnectionState != ECharismaPlaythroughConnectionState::Connected)
	{
		CharismaLogger::Log(6, "Charisma must be connected to before sending events.", CharismaLogger::Warning);
		return;
	}

	SpeechAudioFormat = AudioFormat;

	StartPayload payload;
	payload.conversationUuid = FStringToStdString(ConversationUuid);

	if (SceneIndex)
	{
		payload.sceneIndex = SceneIndex;
	}

	if (StartGraphId)
	{
		payload.startGraphId = StartGraphId;
	}

	if (!StartGraphReferenceId.IsEmpty())
	{
		payload.startGraphReferenceId = FStringToStdString(StartGraphReferenceId);
	}

	if (SpeechAudioFormat.Num() > 0)
	{
		payload.speechConfig = GetSpeechConfig(SpeechAudioFormat);
	}

	RoomInstance->Send("start", payload);
}

void UPlaythrough::Tap(const FString& ConversationUuid) const
{
	if (!RoomInstance || ConnectionState != ECharismaPlaythroughConnectionState::Connected)
	{
		CharismaLogger::Log(6, "Charisma must be connected to before sending events.", CharismaLogger::Warning);
		return;
	}

	TapPayload payload;
	payload.conversationUuid = FStringToStdString(ConversationUuid);

	if (SpeechAudioFormat.Num() > 0)
	{
		payload.speechConfig = GetSpeechConfig(SpeechAudioFormat);
	}

	RoomInstance->Send("tap", payload);
}

void UPlaythrough::Reply(const FString& ConversationUuid, const FString& Message) const
{
	if (!RoomInstance || ConnectionState != ECharismaPlaythroughConnectionState::Connected)
	{
		CharismaLogger::Log(6, "Charisma must be connected to before sending events.", CharismaLogger::Warning);
		return;
	}

	ReplyPayload payload;
	payload.conversationUuid = FStringToStdString(ConversationUuid);
	payload.text = FStringToStdString(Message);

	if (SpeechAudioFormat.Num() > 0)
	{
		payload.speechConfig = GetSpeechConfig(SpeechAudioFormat);
	}

	RoomInstance->Send("reply", payload);
}

void UPlaythrough::Resume(const FString& ConversationUuid) const
{
	if (!RoomInstance || ConnectionState != ECharismaPlaythroughConnectionState::Connected)
	{
		CharismaLogger::Log(6, "Charisma must be connected to before sending events.", CharismaLogger::Warning);
		return;
	}

	ResumePayload payload;
	payload.conversationUuid = FStringToStdString(ConversationUuid);

	if (SpeechAudioFormat.Num() > 0)
	{
		payload.speechConfig = GetSpeechConfig(SpeechAudioFormat);
	}

	RoomInstance->Send("resume", payload);
}

void UPlaythrough::ToggleSpeech(const TArray<ECharismaSpeechAudioFormat> AudioFormat)
{
	SpeechAudioFormat = AudioFormat;
}

SpeechConfig UPlaythrough::GetSpeechConfig(const TArray<ECharismaSpeechAudioFormat> AudioFormat) const
{
	SpeechConfig speechConfig;
	for (const ECharismaSpeechAudioFormat& AudioFormatEntry : AudioFormat)
	{
		if (AudioFormatEntry == ECharismaSpeechAudioFormat::Mp3)
		{
			speechConfig.encoding.push_back("mp3");
		}
		else if (AudioFormatEntry == ECharismaSpeechAudioFormat::Wav)
		{
			speechConfig.encoding.push_back("wav");
		}
		else if (AudioFormatEntry == ECharismaSpeechAudioFormat::Pcm)
		{
			speechConfig.encoding.push_back("pcm");
		}
		else if (AudioFormatEntry == ECharismaSpeechAudioFormat::Ogg)
		{
			speechConfig.encoding.push_back("ogg");
		}
	}
	speechConfig.output = "buffer";
	return speechConfig;
}

void UPlaythrough::Play() const
{
	if (!RoomInstance || ConnectionState != ECharismaPlaythroughConnectionState::Connected)
	{
		CharismaLogger::Log(6, "Charisma must be connected to before sending events.", CharismaLogger::Warning);
		return;
	}

	RoomInstance->Send("play");
}

void UPlaythrough::Pause() const
{
	if (!RoomInstance || ConnectionState != ECharismaPlaythroughConnectionState::Connected)
	{
		CharismaLogger::Log(6, "Charisma must be connected to before sending events.", CharismaLogger::Warning);
		return;
	}

	RoomInstance->Send("pause");
}

void UPlaythrough::StartSpeechRecognition(bool& bWasSuccessful, const ECharismaSpeechRecognitionService service,
	const FString languageCode, const FString encoding, const int32 sampleRate)
{
	if (!RoomInstance || ConnectionState != ECharismaPlaythroughConnectionState::Connected)
	{
		CharismaLogger::Log(6, "Charisma must be connected to before sending events.", CharismaLogger::Warning);
		return;
	}

	if (!MicrophoneCaptureInstance.IsValid())
	{
		MicrophoneCaptureInstance = MakeShared<UMicrophoneCapture>();
	}

	MicrophoneCaptureInstance->OnSpeechAudio = [this](const TArray<uint8>& Audio, uint32 AudioLength)
	{ RoomInstance->Send("speech-recognition-chunk", Audio); };

	bWasSuccessful = MicrophoneCaptureInstance->StartCapture(sampleRate);

	if (bWasSuccessful)
	{
		SpeechRecognitionStartPayload payload;
		payload.service = FStringToStdString(GetSpeechRecognitionServiceString(service));
		payload.languageCode = FStringToStdString(languageCode);
		payload.encoding = FStringToStdString(encoding);
		payload.sampleRate = sampleRate;

		RoomInstance->Send("speech-recognition-start", payload);
	}
}

FString UPlaythrough::GetSpeechRecognitionServiceString(const ECharismaSpeechRecognitionService Service)
{
	switch (Service)
	{
		case ECharismaSpeechRecognitionService::Unified:
			return "unified";
		case ECharismaSpeechRecognitionService::Google:
			return "unified:google";
		case ECharismaSpeechRecognitionService::AWS:
			return "unified:aws";
		case ECharismaSpeechRecognitionService::Deepgram:
			return "unified:deepgram";
	}

	return "";
}

void UPlaythrough::StopSpeechRecognition()
{
	if (!RoomInstance)
	{
		CharismaLogger::Log(6, "Charisma must be connected to before sending events.", CharismaLogger::Warning);
		return;
	}

	if (MicrophoneCaptureInstance)
	{
		MicrophoneCaptureInstance->StopCapture();
	}

	RoomInstance->Send("speech-recognition-stop");
}

void UPlaythrough::ReconnectionFlow()
{
	if (bCalledByDisconnect)
	{
		ChangeConnectionState(ECharismaPlaythroughConnectionState::Disconnected);
		return;
	}

	ChangeConnectionState(ECharismaPlaythroughConnectionState::Reconnecting);

	ClientInstance->JoinById<void>(RoomInstance->Id, {{"sessionId", FStringToStdString(RoomInstance->SessionId)}},
		[this](TSharedPtr<MatchMakeError> Error, TSharedPtr<Room<void>> Room)
		{
			if (Error)
			{
				this->ReconnectionFlowCreate();
				return;
			}

			ChangeConnectionState(ECharismaPlaythroughConnectionState::Connected);
			this->OnRoomJoined(Room);
		});
}

void UPlaythrough::ReconnectionFlowCreate()
{
	ClientInstance->JoinOrCreate<void>("chat",
		{{"token", FStringToStdString(CurToken)}, {"playthroughId", FStringToStdString(CurPlaythroughUuid)},
			{"sdkInfo", UPlaythrough::SdkInfo}},
		[this](TSharedPtr<MatchMakeError> Error, TSharedPtr<Room<void>> Room)
		{
			if (Error)
			{
				ReconnectionTryCount++;

				if (ReconnectionTryCount <= 60)
				{
					this->ReconnectionDelay();
				}
				else
				{
					ChangeConnectionState(ECharismaPlaythroughConnectionState::Disconnected);
				}
				return;
			}

			this->OnRoomJoined(Room);
		});
}

void UPlaythrough::ReconnectionDelay()
{
	UWorld* World = GEngine->GetWorldFromContextObject(CurWorldContextObject, EGetWorldErrorMode::ReturnNull);
	if (World != nullptr)
	{
		float DelayTime = ReconnectionTryDelay + FMath::RandRange(0.0f, ReconnectionTryJitter);
		FTimerHandle ReconnectionTimer;
		World->GetTimerManager().SetTimer(ReconnectionTimer, this, &UPlaythrough::ReconnectionFlowCreate, DelayTime, false);
	}
}

void UPlaythrough::ChangeConnectionState(ECharismaPlaythroughConnectionState NewConnectionState)
{
	if (NewConnectionState != ConnectionState)
	{
		CharismaLogger::Log(1, TEXT("Connection state change: ") + UEnum::GetDisplayValueAsText(NewConnectionState).ToString(),
			CharismaLogger::Info);
		ECharismaPlaythroughConnectionState PreviousConnectionState = ConnectionState;
		ConnectionState = NewConnectionState;
		OnChangeConnectionState.Broadcast(PreviousConnectionState, NewConnectionState);
	}
}