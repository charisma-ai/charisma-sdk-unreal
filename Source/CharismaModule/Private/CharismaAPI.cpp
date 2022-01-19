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


UPlaythrough* UCharismaAPI::CreatePlaythrough(UObject* Owner, UCharismaAPI*& Charisma)
{
	Charisma = NewObject<UCharismaAPI>(Owner);
	return NewObject<UPlaythrough>(Owner);
}

void UCharismaAPI::CreatePlaythroughToken(const int32 StoryId, const int32 StoryVersion, const FString& ApiKey) const
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
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UCharismaAPI::OnTokenRequestComplete);

	HttpRequest->ProcessRequest();
}

void UCharismaAPI::OnTokenRequestComplete(
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
