#include "Playthrough.h"

#include "CharismaAPI.h"
#include "Json.h"
#include "JsonUtilities.h"

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

void UPlaythrough::Connect()
{
	if (bIsPlaying)
	{
		return;
	}

	ClientInstance = MakeShared<Client>(UCharismaAPI::SocketURL);
	UCharismaAPI::Log(1, "Connecting...", Info);
	ClientInstance->JoinOrCreate<void>("chat",
		{{"token", FStringToStdString(CurToken)}, {"playthroughId", FStringToStdString(CurPlaythroughUuid)}},
		[this](TSharedPtr<MatchMakeError> Error, TSharedPtr<Room<void>> Room) {
			if (Error)
			{
				UPlaythrough::ReconnectionFlowCreate();
				return;
			}

			this->RoomInstance = Room;

			UCharismaAPI::Log(1, "Connected.", Info);

			OnConnected.Broadcast(true);
			OnReady.Broadcast();

			this->RoomInstance->OnMessage(
				"status", [](const msgpack::object& message) { UCharismaAPI::Log(-1, TEXT("Ready to begin playing."), Info); });

			this->RoomInstance->OnMessage("message", [this](const msgpack::object& message) {
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

				if (Event.EndStory)
				{
					this->bIsPlaying = false;
				}

				UCharismaAPI::Log(-1, Event.Message.Character.Name + TEXT(": ") + Event.Message.Text, Info);

				UPlaythrough::SaveEmotionsMemories(Event.Emotions, Event.Memories);
				OnMessage.Broadcast(Event);
			});

			this->RoomInstance->OnMessage("start-typing", [this](const msgpack::object& message) { OnTyping.Broadcast(true); });
			this->RoomInstance->OnMessage("stop-typing", [this](const msgpack::object& message) { OnTyping.Broadcast(false); });

			this->RoomInstance->OnMessage("problem", [this](const msgpack::object& message) {
				FCharismaErrorEvent Event = message.as<FCharismaErrorEvent>();

				UCharismaAPI::Log(-1, Event.Error, ECharismaLogSeverity::Error);

				OnError.Broadcast(Event);
			});

			this->RoomInstance->OnLeave = ([this]() {
				OnConnected.Broadcast(false);
				UPlaythrough::ReconnectionFlow();
			});

			this->RoomInstance->OnError =
				([this](const int& Code, const FString& Error) { UCharismaAPI::Log(-1, Error, ECharismaLogSeverity::Error); });
		});
}

void UPlaythrough::Disconnect()
{
	bIsPlaying = false;
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
	if (!RoomInstance)
	{
		UCharismaAPI::Log(6, "Charisma must be connected to before sending events.", Warning, 5.f);
		return;
	}

	ActionPayload payload;
	payload.conversationUuid = FStringToStdString(ConversationUuid);
	payload.action = FStringToStdString(ActionName);

	if (bUseSpeech)
	{
		payload.speechConfig = GetSpeechConfig();
	}

	RoomInstance->Send("action", payload);
}

void UPlaythrough::Start(const FString& ConversationUuid, const int32 SceneIndex, const int32 StartGraphId,
	const FString& StartGraphReferenceId, const bool UseSpeech)
{
	if (!RoomInstance)
	{
		UCharismaAPI::Log(6, "Charisma must be connected to before sending events.", Warning, 5.f);
		return;
	}

	bIsPlaying = true;
	bUseSpeech = UseSpeech;

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

	if (bUseSpeech)
	{
		payload.speechConfig = GetSpeechConfig();
	}

	RoomInstance->Send("start", payload);
}

void UPlaythrough::Tap(const FString& ConversationUuid) const
{
	if (!RoomInstance)
	{
		UCharismaAPI::Log(6, "Charisma must be connected to before sending events.", Warning, 5.f);
		return;
	}

	TapPayload payload;
	payload.conversationUuid = FStringToStdString(ConversationUuid);

	if (bUseSpeech)
	{
		payload.speechConfig = GetSpeechConfig();
	}

	RoomInstance->Send("tap", payload);
}

void UPlaythrough::Reply(const FString& ConversationUuid, const FString& Message) const
{
	if (!RoomInstance)
	{
		UCharismaAPI::Log(6, "Charisma must be connected to before sending events.", Warning, 5.f);
		return;
	}

	ReplyPayload payload;
	payload.conversationUuid = FStringToStdString(ConversationUuid);
	payload.text = FStringToStdString(Message);

	if (bUseSpeech)
	{
		payload.speechConfig = GetSpeechConfig();
	}

	RoomInstance->Send("reply", payload);
}

void UPlaythrough::Resume(const FString& ConversationUuid) const
{
	if (!RoomInstance)
	{
		UCharismaAPI::Log(6, "Charisma must be connected to before sending events.", Warning, 5.f);
		return;
	}

	ResumePayload payload;
	payload.conversationUuid = FStringToStdString(ConversationUuid);

	if (bUseSpeech)
	{
		payload.speechConfig = GetSpeechConfig();
	}

	RoomInstance->Send("resume", payload);
}

void UPlaythrough::ToggleSpeechOn()
{
	bUseSpeech = true;
}

void UPlaythrough::ToggleSpeechOff()
{
	bUseSpeech = false;
}

SpeechConfig UPlaythrough::GetSpeechConfig() const
{
	SpeechConfig speechConfig;
	speechConfig.encoding = "ogg";
	speechConfig.output = "buffer";
	return speechConfig;
}

void UPlaythrough::SaveEmotionsMemories(const TArray<FCharismaEmotion>& Emotions, const TArray<FCharismaMemory>& Memories)
{
	PlaythroughEmotions = Emotions;
	PlaythroughMemories = Memories;
}

void UPlaythrough::ReconnectionFlow()
{
	if (bCalledByDisconnect)
	{
		return;
	}

	UCharismaAPI::Log(1, "Reconnecting...", Info);

	ClientInstance->JoinOrCreate<void>(RoomInstance->Id, {{"sessionId", FStringToStdString(RoomInstance->SessionId)}},
		[this](TSharedPtr<MatchMakeError> Error, TSharedPtr<Room<void>> Room) {
			if (Error)
			{
				UPlaythrough::ReconnectionFlowCreate();
				return;
			}

			this->RoomInstance = Room;

			UCharismaAPI::Log(1, "Connected.", Info);

			OnConnected.Broadcast(true);
			OnReady.Broadcast();

			this->RoomInstance->OnMessage(
				"status", [](const msgpack::object& message) { UCharismaAPI::Log(-1, TEXT("Ready to begin playing."), Info); });

			this->RoomInstance->OnMessage("message", [this](const msgpack::object& message) {
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

				if (Event.EndStory)
				{
					this->bIsPlaying = false;
				}

				UCharismaAPI::Log(-1, Event.Message.Character.Name + TEXT(": ") + Event.Message.Text, Info);

				UPlaythrough::SaveEmotionsMemories(Event.Emotions, Event.Memories);
				OnMessage.Broadcast(Event);
			});

			this->RoomInstance->OnMessage("start-typing", [this](const msgpack::object& message) { OnTyping.Broadcast(true); });
			this->RoomInstance->OnMessage("stop-typing", [this](const msgpack::object& message) { OnTyping.Broadcast(false); });

			this->RoomInstance->OnMessage("problem", [this](const msgpack::object& message) {
				FCharismaErrorEvent Event = message.as<FCharismaErrorEvent>();

				UCharismaAPI::Log(-1, Event.Error, ECharismaLogSeverity::Error);

				OnError.Broadcast(Event);
			});

			this->RoomInstance->OnLeave = ([this]() {
				OnConnected.Broadcast(false);

				UPlaythrough::ReconnectionFlow();
			});

			this->RoomInstance->OnError =
				([this](const int& Code, const FString& Error) { UCharismaAPI::Log(-1, Error, ECharismaLogSeverity::Error); });
			return;
		});
}

void UPlaythrough::ReconnectionFlowCreate()
{
	ClientInstance->JoinOrCreate<void>("chat",
		{{"token", FStringToStdString(CurToken)}, {"playthroughId", FStringToStdString(CurPlaythroughUuid)}},
		[this](TSharedPtr<MatchMakeError> Error, TSharedPtr<Room<void>> Room) {
			if (Error)
			{
				ReconnectionTryCount++;

				if (ReconnectionTryCount <= 20)
				{
					UPlaythrough::ReconnectionDelay();
				}
				return;
			}

			ReconnectionTryCount = 0;

			this->RoomInstance = Room;

			UCharismaAPI::Log(1, "Connected.", Info);

			OnConnected.Broadcast(true);
			OnReady.Broadcast();

			this->RoomInstance->OnMessage(
				"status", [](const msgpack::object& message) { UCharismaAPI::Log(-1, TEXT("Ready to begin playing."), Info); });

			this->RoomInstance->OnMessage("message", [this](const msgpack::object& message) {
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

				if (Event.EndStory)
				{
					this->bIsPlaying = false;
				}

				UCharismaAPI::Log(-1, Event.Message.Character.Name + TEXT(": ") + Event.Message.Text, Info);

				UPlaythrough::SaveEmotionsMemories(Event.Emotions, Event.Memories);
				OnMessage.Broadcast(Event);
			});

			this->RoomInstance->OnMessage("start-typing", [this](const msgpack::object& message) { OnTyping.Broadcast(true); });
			this->RoomInstance->OnMessage("stop-typing", [this](const msgpack::object& message) { OnTyping.Broadcast(false); });

			this->RoomInstance->OnMessage("problem", [this](const msgpack::object& message) {
				FCharismaErrorEvent Event = message.as<FCharismaErrorEvent>();

				UCharismaAPI::Log(-1, Event.Error, ECharismaLogSeverity::Error);

				OnError.Broadcast(Event);
			});

			this->RoomInstance->OnLeave = ([this]() {
				OnConnected.Broadcast(false);
				UPlaythrough::ReconnectionFlow();
			});

			this->RoomInstance->OnError =
				([this](const int& Code, const FString& Error) { UCharismaAPI::Log(-1, Error, ECharismaLogSeverity::Error); });
			return;
		});
}

void UPlaythrough::ReconnectionDelay()
{
	float DelayTime = 5;
	DelayTime = DelayTime + FMath::RandRange(0, 3);
	FTimerHandle ReconnectionTimer;

	UWorld* World = GEngine->GetWorldFromContextObject(CurWorldContextObject);
	World->GetTimerManager().SetTimer(ReconnectionTimer, this, &UPlaythrough::ReconnectionFlowCreate, DelayTime, false);
}
