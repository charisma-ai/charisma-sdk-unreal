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
