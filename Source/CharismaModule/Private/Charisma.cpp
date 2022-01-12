#include "Charisma.h"

#include "Json.h"
#include "JsonUtilities.h"

#include <nlohmann/json.hpp>
#include <sstream>

const FString UCharisma::BaseURL = TEXT("https://play.charisma.ai");
const FString UCharisma::SocketURL = UCharisma::BaseURL.Replace(TEXT("http"), TEXT("ws"));

FString ToQueryString(const TMap<FString, FString>& QueryParams)
{
	if (!QueryParams.Num())
	{
		return TEXT("");
	}

	FString Result;
	for (const TPair<FString, FString>& Pair : QueryParams)
	{
		Result.Append(Pair.Key);
		Result.Append(TEXT(""));
		Result.Append(Pair.Value);
	}

	return Result;
}

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

void UCharisma::CreatePlaythroughToken(const int32 StoryId, const int32 StoryVersion, const FString& ApiKey) const
{
	TSharedPtr<FJsonObject> RequestData = MakeShareable(new FJsonObject);
	RequestData->SetNumberField("storyId", StoryId);

	if (StoryVersion != 0)
	{
		RequestData->SetNumberField("version", StoryVersion);
	}

	FString OutputString;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(RequestData.ToSharedRef(), Writer);

	FHttpModule* HttpModule = &FHttpModule::Get();
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = HttpModule->CreateRequest();

	if (StoryVersion == -1)
	{
		if (!ApiKey.IsEmpty())
		{
			HttpRequest->SetHeader("Authorization", "API-Key " + ApiKey);
		}
		else
		{
			Log(-3, "An API key must be provided to create a playthrough token for the draft story version.", Error);
		}
	}

	HttpRequest->SetVerb("POST");
	HttpRequest->AppendToHeader("Content-Type", "application/json");
	HttpRequest->SetURL(BaseURL + "/play/token");
	HttpRequest->SetContentAsString(OutputString);
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UCharisma::OnTokenRequestComplete);

	HttpRequest->ProcessRequest();
}

void UCharisma::CreateConversation(const FString& Token) const
{
	FHttpModule* HttpModule = &FHttpModule::Get();
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = HttpModule->CreateRequest();

	HttpRequest->SetVerb("POST");
	HttpRequest->SetHeader("Authorization", "Bearer " + Token);
	HttpRequest->SetURL(BaseURL + "/play/conversation");
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UCharisma::OnConversationRequestComplete);

	HttpRequest->ProcessRequest();
}

void UCharisma::SetMemory(const FString& Token, const FString& RecallValue, const FString& SaveValue) const
{
	TSharedPtr<FJsonObject> RequestData = MakeShareable(new FJsonObject);
	RequestData->SetStringField("memoryRecallValue", RecallValue);
	RequestData->SetStringField("saveValue", SaveValue);

	FString OutputString;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(RequestData.ToSharedRef(), Writer);

	FHttpModule* HttpModule = &FHttpModule::Get();
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = HttpModule->CreateRequest();

	HttpRequest->SetVerb("POST");
	HttpRequest->SetHeader("Authorization", "Bearer " + Token);
	HttpRequest->AppendToHeader("Content-Type", "application/json");
	HttpRequest->SetURL(BaseURL + "/play/set-memory");
	HttpRequest->SetContentAsString(OutputString);
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UCharisma::OnSetMemory);

	HttpRequest->ProcessRequest();
}

void UCharisma::RestartFromEventId(const FString& TokenForRestart, const int64 EventId) const
{
	TSharedPtr<FJsonObject> RequestData = MakeShareable(new FJsonObject);
	RequestData->SetStringField("eventId", FString::Printf(TEXT("%lld"), EventId));

	FString OutputString;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(RequestData.ToSharedRef(), Writer);

	FHttpModule* HttpModule = &FHttpModule::Get();
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = HttpModule->CreateRequest();

	HttpRequest->SetVerb("POST");
	HttpRequest->SetHeader("Authorization", "Bearer " + TokenForRestart);
	HttpRequest->AppendToHeader("Content-Type", "application/json");
	HttpRequest->SetURL(BaseURL + "/play/restart-from-event");
	HttpRequest->SetContentAsString(OutputString);
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UCharisma::OnRestartRequestComplete);

	HttpRequest->ProcessRequest();
}

void UCharisma::GetMessageHistory(const FString& Token, const int32 ConversationId, const int64 MinEventId) const
{
	FHttpModule* HttpModule = &FHttpModule::Get();
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = HttpModule->CreateRequest();

	TMap<FString, FString> QueryParams;
	if (ConversationId)
	{
		QueryParams.Add("conversationId", FString::Printf(TEXT("%d"), ConversationId));
	}
	if (MinEventId)
	{
		QueryParams.Add("minEventId", FString::Printf(TEXT("%lld"), MinEventId));
	}

	FString Query = ToQueryString(QueryParams);

	HttpRequest->SetVerb("GET");
	HttpRequest->SetHeader("Authorization", "Bearer " + Token);
	HttpRequest->SetURL(BaseURL + "/play/message-history" + Query);
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UCharisma::OnMessageHistoryComplete);

	HttpRequest->ProcessRequest();
}

void UCharisma::GetPlaythroughInfo(const FString& Token) const
{
	FHttpModule* HttpModule = &FHttpModule::Get();
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = HttpModule->CreateRequest();

	HttpRequest->SetVerb("GET");
	HttpRequest->SetHeader("Authorization", "Bearer " + Token);
	HttpRequest->SetURL(BaseURL + "/play/playthrough-info");
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UCharisma::OnPlaythroughInfoComplete);

	HttpRequest->ProcessRequest();
}

void UCharisma::OnTokenRequestComplete(
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
				const FString Token = ResponseData->GetStringField(TEXT("token"));
				const int32 PlaythroughId = ResponseData->GetIntegerField(TEXT("playthroughId"));

				OnTokenCreated.Broadcast(Token, PlaythroughId);
			}
			else
			{
				Log(-2, "Failed to deserialize response data.", Error, 5.f);
			}
		}
		else
		{
			TArray<FStringFormatArg> Args;
			Args.Add(FStringFormatArg(FString::FromInt(ResponseCode)));
			Args.Add(FStringFormatArg(Content));
			Log(-2, FString::Format(TEXT("{0}, {1}."), Args), Error, 5.f);
		}
	}
}

void UCharisma::OnConversationRequestComplete(
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

void UCharisma::OnSetMemory(FHttpRequestPtr Request, FHttpResponsePtr Response, bool WasSuccessful) const
{
	if (WasSuccessful)
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

void UCharisma::OnRestartRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool WasSuccessful) const
{
	if (WasSuccessful)
	{
		const int32 ResponseCode = Response->GetResponseCode();
		const FString Content = Response->GetContentAsString();

		if (ResponseCode == 200)
		{
			Log(-1, "Restarted from chosen event", Info);
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

void UCharisma::OnMessageHistoryComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool WasSuccessful) const
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
				Log(-1, "Failed to deserialize message history response data.", Error, 5.f);
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

void UCharisma::OnPlaythroughInfoComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool WasSuccessful) const
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
				Log(-1, "Failed to deserialize playthrough info response data.", Error, 5.f);
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

void UCharisma::Connect(const FString& Token, const int32 PlaythroughId)
{
	if (bIsPlaying)
	{
		return;
	}

	ClientInstance = MakeShared<Client>(SocketURL);
	Log(1, "Connecting...", Info);
	ClientInstance->JoinOrCreate<void>("chat", {{"token", FStringToStdString(Token)}, {"playthroughId", PlaythroughId}},
		[this](TSharedPtr<MatchMakeError> Error, TSharedPtr<Room<void>> Room) {
			if (Error)
			{
				return;
			}

			this->RoomInstance = Room;

			Log(1, "Connected.", Info);

			OnConnected.Broadcast(true);
			OnReady.Broadcast();

			this->RoomInstance->OnMessage(
				"status", [](const msgpack::object& message) { Log(-1, TEXT("Ready to begin playing."), Info); });

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

				Log(-1, Event.Message.Character.Name + TEXT(": ") + Event.Message.Text, Info);

				OnMessage.Broadcast(Event);
			});

			this->RoomInstance->OnMessage("start-typing", [this](const msgpack::object& message) { OnTyping.Broadcast(true); });
			this->RoomInstance->OnMessage("stop-typing", [this](const msgpack::object& message) { OnTyping.Broadcast(false); });

			this->RoomInstance->OnMessage("problem", [this](const msgpack::object& message) {
				FCharismaErrorEvent Event = message.as<FCharismaErrorEvent>();

				Log(-1, Event.Error, ECharismaLogSeverity::Error);

				OnError.Broadcast(Event);
			});

			this->RoomInstance->OnLeave = ([this]() {
				Log(-1, "Disconnected.", Info);

				OnConnected.Broadcast(false);
			});

			this->RoomInstance->OnError =
				([this](const int& Code, const FString& Error) { Log(-1, Error, ECharismaLogSeverity::Error); });
		});
}

void UCharisma::Disconnect()
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

void UCharisma::Action(const int32 ConversationId, const FString& ActionName) const
{
	if (!RoomInstance)
	{
		Log(6, "Charisma must be connected to before sending events.", Warning, 5.f);
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

void UCharisma::Start(const int32 ConversationId, const int32 SceneIndex, const int32 StartGraphId,
	const FString& StartGraphReferenceId, const bool UseSpeech)
{
	if (!RoomInstance)
	{
		Log(6, "Charisma must be connected to before sending events.", Warning, 5.f);
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

void UCharisma::Tap(const int32 ConversationId) const
{
	if (!RoomInstance)
	{
		Log(6, "Charisma must be connected to before sending events.", Warning, 5.f);
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

void UCharisma::Reply(const int32 ConversationId, const FString& Message) const
{
	if (!RoomInstance)
	{
		Log(6, "Charisma must be connected to before sending events.", Warning, 5.f);
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

void UCharisma::Resume(const int32 ConversationId) const
{
	if (!RoomInstance)
	{
		Log(6, "Charisma must be connected to before sending events.", Warning, 5.f);
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

void UCharisma::ToggleSpeechOn()
{
	bUseSpeech = true;
}

void UCharisma::ToggleSpeechOff()
{
	bUseSpeech = false;
}

SpeechConfig UCharisma::GetSpeechConfig() const
{
	SpeechConfig speechConfig;
	speechConfig.encoding = "ogg";
	speechConfig.output = "buffer";
	return speechConfig;
}

void UCharisma::Log(const int32 Key, const FString& Message, const ECharismaLogSeverity Severity, const float Duration)
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
		default:
			MessageColor = FColor::White;
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(Key, Duration, MessageColor, FString("Charisma: " + Message));
	}
}
