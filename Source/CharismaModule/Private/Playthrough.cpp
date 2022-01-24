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

void UPlaythrough::OnTokenRequestComplete(const FHttpRequestPtr Request, const FHttpResponsePtr Response, const bool WasSuccessful) const
{
	if (WasSuccessful)
	{
		const int32 ResponseCode = Response->GetResponseCode();
		const FString Content = Response->GetContentAsString();

		if (ResponseCode == 200)
		{
			TSharedPtr<FJsonObject> ResponseData = MakeShareable(new FJsonObject);

			const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Content);
			if (FJsonSerializer::Deserialize(Reader, ResponseData))
			{
				const FString Token = ResponseData->GetStringField(TEXT("token"));
				const int32 PlaythroughId = ResponseData->GetIntegerField(TEXT("playthroughId"));
				OnTokenCreated.Broadcast(Token, PlaythroughId, this);
			}
			else
			{
				UCharismaAPI::Log(-2, "Failed to deserialize response data.", Error, 5.f);
			}
		}
		else
		{
			const FString Token = "Null";
			const int32 PlaythroughId = 0;
			UPlaythrough* Playthrough = nullptr;
			OnTokenFailCreation.Broadcast(Token, PlaythroughId, Playthrough);
			TArray<FStringFormatArg> Args;
			Args.Add(FStringFormatArg(FString::FromInt(ResponseCode)));
			Args.Add(FStringFormatArg(Content));
			UCharismaAPI::Log(-2, FString::Format(TEXT("{0}, {1}."), Args), Error, 5.f);
		}
	}
}

void UPlaythrough::OnConversationRequestComplete(
	const FHttpRequestPtr Request, const FHttpResponsePtr Response, const bool WasSuccessful) const
{
	if (WasSuccessful)
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
				UCharismaAPI::Log(-1, "Failed to deserialize response data.", Error, 5.f);
			}
		}
		else
		{
			TArray<FStringFormatArg> Args;
			Args.Add(FStringFormatArg(FString::FromInt(ResponseCode)));
			Args.Add(FStringFormatArg(Content));
			UCharismaAPI::Log(-1, FString::Format(TEXT("{0}, {1}."), Args), Error, 5.f);
		}
	}
}

void UPlaythrough::OnSetMemory(FHttpRequestPtr Request, FHttpResponsePtr Response, bool WasSuccessful) const
{
	if (WasSuccessful)
	{
		const int32 ResponseCode = Response->GetResponseCode();
		const FString Content = Response->GetContentAsString();

		if (ResponseCode == 200)
		{
			UCharismaAPI::Log(0, Response.Get()->GetContentAsString(), Info);
		}
		else
		{
			TArray<FStringFormatArg> Args;
			Args.Add(FStringFormatArg(FString::FromInt(ResponseCode)));
			Args.Add(FStringFormatArg(Content));
			UCharismaAPI::Log(0, FString::Format(TEXT("{0}, {1}."), Args), Error, 5.f);
		}
	}
}

void UPlaythrough::OnRestartRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool WasSuccessful) const
{
	if (WasSuccessful)
	{
		const int32 ResponseCode = Response->GetResponseCode();
		const FString Content = Response->GetContentAsString();

		if (ResponseCode == 200)
		{
			UCharismaAPI::Log(-1, "Restarted from chosen event", Info);
		}
		else
		{
			TArray<FStringFormatArg> Args;
			Args.Add(FStringFormatArg(FString::FromInt(ResponseCode)));
			Args.Add(FStringFormatArg(Content));
			UCharismaAPI::Log(-1, FString::Format(TEXT("{0}, {1}."), Args), Error, 5.f);
		}
	}
}

void UPlaythrough::OnMessageHistoryComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool WasSuccessful) const
{
	if (WasSuccessful)
	{
		const int32 ResponseCode = Response->GetResponseCode();
		const FString Content = Response->GetContentAsString();

		if (ResponseCode == 200)
		{
			FCharismaMessageHistoryResponse MessageHistory;
			if (FJsonObjectConverter::JsonObjectStringToUStruct(Content, &MessageHistory, 0, 0))
			{
				OnMessageHistory.Broadcast(MessageHistory);
			}
			else
			{
				UCharismaAPI::Log(-1, "Failed to deserialize message history response data.", Error, 5.f);
			}
		}
		else
		{
			TArray<FStringFormatArg> Args;
			Args.Add(FStringFormatArg(FString::FromInt(ResponseCode)));
			Args.Add(FStringFormatArg(Content));
			UCharismaAPI::Log(-1, FString::Format(TEXT("{0}, {1}."), Args), Error, 5.f);
		}
	}
}

void UPlaythrough::OnPlaythroughInfoComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool WasSuccessful) const
{
	if (WasSuccessful)
	{
		const int32 ResponseCode = Response->GetResponseCode();
		const FString Content = Response->GetContentAsString();

		if (ResponseCode == 200)
		{
			FCharismaPlaythroughInfoResponse PlaythroughInfo;
			if (FJsonObjectConverter::JsonObjectStringToUStruct(Content, &PlaythroughInfo, 0, 0))
			{
				OnPlaythroughInfo.Broadcast(PlaythroughInfo);
			}
			else
			{
				UCharismaAPI::Log(-1, "Failed to deserialize playthrough info response data.", Error, 5.f);
			}
		}
		else
		{
			TArray<FStringFormatArg> Args;
			Args.Add(FStringFormatArg(FString::FromInt(ResponseCode)));
			Args.Add(FStringFormatArg(Content));
			UCharismaAPI::Log(-1, FString::Format(TEXT("{0}, {1}."), Args), Error, 5.f);
		}
	}
}

void UPlaythrough::Connect(const FString& Token, const int32 PlaythroughId)
{
	if (bIsPlaying)
	{
		return;
	}

	ClientInstance = MakeShared<Client>(UCharismaAPI::SocketURL);
	UCharismaAPI::Log(1, "Connecting...", Info);
	ClientInstance->JoinOrCreate<void>("chat", {{"token", FStringToStdString(Token)}, {"playthroughId", PlaythroughId}},
		[this](TSharedPtr<MatchMakeError> Error, TSharedPtr<Room<void>> Room)
		{
			if (Error)
			{
				return;
			}

			this->RoomInstance = Room;

			UCharismaAPI::Log(1, "Connected.", Info);

			OnConnected.Broadcast(true);
			OnReady.Broadcast();

			this->RoomInstance->OnMessage(
				"status", [](const msgpack::object& message) { UCharismaAPI::Log(-1, TEXT("Ready to begin playing."), Info); });

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

					if (Event.EndStory)
					{
						this->bIsPlaying = false;
					}

					UCharismaAPI::Log(-1, Event.Message.Character.Name + TEXT(": ") + Event.Message.Text, Info);

					OnMessage.Broadcast(Event);
				});

			this->RoomInstance->OnMessage("start-typing", [this](const msgpack::object& message) { OnTyping.Broadcast(true); });
			this->RoomInstance->OnMessage("stop-typing", [this](const msgpack::object& message) { OnTyping.Broadcast(false); });

			this->RoomInstance->OnMessage("problem",
				[this](const msgpack::object& message)
				{
					FCharismaErrorEvent Event = message.as<FCharismaErrorEvent>();

					UCharismaAPI::Log(-1, Event.Error, ECharismaLogSeverity::Error);

					OnError.Broadcast(Event);
				});

			this->RoomInstance->OnLeave = ([this]() {
				UCharismaAPI::Log(-1, "Disconnected.", Info);

				OnConnected.Broadcast(false);
			});

			this->RoomInstance->OnError =
				([this](const int& Code, const FString& Error) { UCharismaAPI::Log(-1, Error, ECharismaLogSeverity::Error); });
		});
}

void UPlaythrough::Disconnect()
{
	bIsPlaying = false;

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

void UPlaythrough::Action(const int32 ConversationId, const FString& ActionName) const
{
	if (!RoomInstance)
	{
		UCharismaAPI::Log(6, "Charisma must be connected to before sending events.", Warning, 5.f);
		return;
	}

	ActionPayload payload;
	payload.conversationId = ConversationId;
	payload.action = FStringToStdString(ActionName);

	if (bUseSpeech)
	{
		payload.speechConfig = GetSpeechConfig();
	}

	RoomInstance->Send("action", payload);
}

void UPlaythrough::Start(const int32 ConversationId, const int32 SceneIndex, const int32 StartGraphId,
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
	payload.conversationId = ConversationId;

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

void UPlaythrough::Tap(const int32 ConversationId) const
{
	if (!RoomInstance)
	{
		UCharismaAPI::Log(6, "Charisma must be connected to before sending events.", Warning, 5.f);
		return;
	}

	TapPayload payload;
	payload.conversationId = ConversationId;

	if (bUseSpeech)
	{
		payload.speechConfig = GetSpeechConfig();
	}

	RoomInstance->Send("tap", payload);
}

void UPlaythrough::Reply(const int32 ConversationId, const FString& Message) const
{
	if (!RoomInstance)
	{
		UCharismaAPI::Log(6, "Charisma must be connected to before sending events.", Warning, 5.f);
		return;
	}

	ReplyPayload payload;
	payload.conversationId = ConversationId;
	payload.text = FStringToStdString(Message);

	if (bUseSpeech)
	{
		payload.speechConfig = GetSpeechConfig();
	}

	RoomInstance->Send("reply", payload);
}

void UPlaythrough::Resume(const int32 ConversationId) const
{
	if (!RoomInstance)
	{
		UCharismaAPI::Log(6, "Charisma must be connected to before sending events.", Warning, 5.f);
		return;
	}

	ResumePayload payload;
	payload.conversationId = ConversationId;

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