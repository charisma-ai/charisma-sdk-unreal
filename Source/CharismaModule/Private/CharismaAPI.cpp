#include "CharismaAPI.h"
#include "Playthrough.h"

#include "Json.h"
#include "JsonUtilities.h"

#include <nlohmann/json.hpp>
#include <sstream>

const FString UCharismaAPI::BaseURL = TEXT("https://play.charisma.ai");
const FString UCharismaAPI::SocketURL = UCharismaAPI::BaseURL.Replace(TEXT("http"), TEXT("ws"));

FString UCharismaAPI::ToQueryString(const TMap<FString, FString>& QueryParams)
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

void UCharismaAPI::CreateCharismaPlaythroughObject(UPlaythrough*& Playthrough)
{
	Playthrough = NewObject<UPlaythrough>();
}

 void UCharismaAPI::InitPlaythoughObject(const int32 StoryId, const int32 StoryVersion, const FString& ApiKey, UPlaythrough*& Playthrough)
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
	HttpRequest->OnProcessRequestComplete().BindUObject(Playthrough, &UPlaythrough::OnTokenRequestComplete);

	HttpRequest->ProcessRequest();
}

void UCharismaAPI::Log(const int32 Key, const FString& Message, const ECharismaLogSeverity Severity, const float Duration)
{
	FColor MessageColor;
	switch (Severity)
	{
		case Info:
			MessageColor = FColor::Green;
			UE_LOG(LogTemp, Log, TEXT("Charisma: %s"), *Message);
			break;
		case Warning:
			MessageColor = FColor::Orange;
			UE_LOG(LogTemp, Warning, TEXT("Charisma: %s"), *Message);
			break;
		case Error:
			MessageColor = FColor::Red;
			UE_LOG(LogTemp, Error, TEXT("Charisma: %s"), *Message);
			break;
		default:
			MessageColor = FColor::White;
			UE_LOG(LogTemp, Log, TEXT("Charisma: %s"), *Message);
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(Key, Duration, MessageColor, FString("Charisma: " + Message));
	}
}

void UCharismaAPI::CreateConversationAPI(UPlaythrough* Playthrough, const FString& Token)
{
	FHttpModule* HttpModule = &FHttpModule::Get();
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = HttpModule->CreateRequest();

	HttpRequest->SetVerb("POST");
	HttpRequest->SetHeader("Authorization", "Bearer " + Token);
	HttpRequest->SetURL(UCharismaAPI::BaseURL + "/play/conversation");
	HttpRequest->OnProcessRequestComplete().BindUObject(Playthrough, &UPlaythrough::OnConversationRequestComplete);

	HttpRequest->ProcessRequest();
}

void UCharismaAPI::SetMemoryAPI(UPlaythrough* Playthrough, const FString& Token, const FString& RecallValue, const FString& SaveValue)
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
	HttpRequest->SetURL(UCharismaAPI::BaseURL + "/play/set-memory");
	HttpRequest->SetContentAsString(OutputString);
	HttpRequest->OnProcessRequestComplete().BindUObject(Playthrough, &UPlaythrough::OnSetMemory);

	HttpRequest->ProcessRequest();
}

 void UCharismaAPI::RestartFromEventIdAPI(UPlaythrough* Playthrough, const FString& TokenForRestart, const int64 EventId)
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
	 HttpRequest->SetURL(UCharismaAPI::BaseURL + "/play/restart-from-event");
	 HttpRequest->SetContentAsString(OutputString);
	 HttpRequest->OnProcessRequestComplete().BindUObject(Playthrough, &UPlaythrough::OnRestartRequestComplete);

	 HttpRequest->ProcessRequest();
 }

void UCharismaAPI::GetMessageHistoryAPI(UPlaythrough* Playthrough, const FString& Token, const int32 ConversationId, const int64 MinEventId)
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

	FString Query = UCharismaAPI::ToQueryString(QueryParams);

	HttpRequest->SetVerb("GET");
	HttpRequest->SetHeader("Authorization", "Bearer " + Token);
	HttpRequest->SetURL(UCharismaAPI::BaseURL + "/play/message-history" + Query);
	HttpRequest->OnProcessRequestComplete().BindUObject(Playthrough, &UPlaythrough::OnMessageHistoryComplete);

	HttpRequest->ProcessRequest();
}

void UCharismaAPI::GetPlaythroughInfoAPI(UPlaythrough* Playthrough, const FString& Token)
{
	FHttpModule* HttpModule = &FHttpModule::Get();
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = HttpModule->CreateRequest();

	HttpRequest->SetVerb("GET");
	HttpRequest->SetHeader("Authorization", "Bearer " + Token);
	HttpRequest->SetURL(UCharismaAPI::BaseURL + "/play/playthrough-info");
	HttpRequest->OnProcessRequestComplete().BindUObject(Playthrough, &UPlaythrough::OnPlaythroughInfoComplete);

	HttpRequest->ProcessRequest();
}