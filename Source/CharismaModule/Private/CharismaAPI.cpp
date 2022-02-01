#include "CharismaAPI.h"

#include "Json.h"
#include "JsonUtilities.h"
#include "Playthrough.h"

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

	FString Result = TEXT("?");
	for (const TPair<FString, FString>& Pair : QueryParams)
	{
		Result.Append(Pair.Key);
		Result.Append(TEXT("="));
		Result.Append(Pair.Value);
		Result.Append(TEXT("&"));
	}

	return Result;
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

void UCharismaAPI::CreatePlaythroughToken(const int32 StoryId, const int32 StoryVersion, const FString& ApiKey,
	const TFunction<void(const FString Token, const FString PlaythroughUuid)>& SuccessCallback,
	const TFunction<void(const FString Error)>& ErrorCallback)
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

	HttpRequest->OnProcessRequestComplete().BindLambda([SuccessCallback, ErrorCallback](const FHttpRequestPtr Request,
														   const FHttpResponsePtr Response, const bool WasSuccessful) {
		if (!WasSuccessful)
		{
			ErrorCallback(TEXT("`/play/token`: Error! HTTP request was not successful."));
			return;
		}

		const int32 ResponseCode = Response->GetResponseCode();
		const FString Content = Response->GetContentAsString();

		if (ResponseCode != 200)
		{
			TArray<FStringFormatArg> Args;
			Args.Add(FStringFormatArg(FString::FromInt(ResponseCode)));
			Args.Add(FStringFormatArg(Content));
			FString ErrorMessage = FString::Format(TEXT("`/play/token`: Error! (Response code: {0}): {1}"), Args);

			UCharismaAPI::Log(-2, ErrorMessage, Error, 5.f);
			ErrorCallback(ErrorMessage);

			return;
		}

		TSharedPtr<FJsonObject> ResponseData = MakeShareable(new FJsonObject);

		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Content);
		if (!FJsonSerializer::Deserialize(Reader, ResponseData))
		{
			TArray<FStringFormatArg> Args;
			Args.Add(FStringFormatArg(Content));
			FString ErrorMessage = FString::Format(TEXT("`/play/token`: Error! Failed to deserialize response data: {0}"), Args);

			UCharismaAPI::Log(-2, ErrorMessage, Error, 5.f);
			ErrorCallback(ErrorMessage);
			return;
		}

		const FString Token = ResponseData->GetStringField(TEXT("token"));
		const FString PlaythroughUuid = ResponseData->GetStringField(TEXT("playthroughUuid"));
		SuccessCallback(Token, PlaythroughUuid);
	});

	HttpRequest->ProcessRequest();
}

void UCharismaAPI::CreateConversation(const FString& Token, const TFunction<void(const FString ConversationUuid)>& SuccessCallback,
	const TFunction<void(const FString Error)>& ErrorCallback)
{
	FHttpModule* HttpModule = &FHttpModule::Get();
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = HttpModule->CreateRequest();

	HttpRequest->SetVerb("POST");
	HttpRequest->SetHeader("Authorization", "Bearer " + Token);
	HttpRequest->SetURL(UCharismaAPI::BaseURL + "/play/conversation");

	HttpRequest->OnProcessRequestComplete().BindLambda(
		[SuccessCallback, ErrorCallback](const FHttpRequestPtr Request, const FHttpResponsePtr Response, const bool WasSuccessful) {
			if (!WasSuccessful)
			{
				ErrorCallback(TEXT("`/play/conversation`: Error! HTTP request was not successful."));
				return;
			}

			const int32 ResponseCode = Response->GetResponseCode();
			const FString Content = Response->GetContentAsString();

			if (ResponseCode != 200)
			{
				TArray<FStringFormatArg> Args;
				Args.Add(FStringFormatArg(FString::FromInt(ResponseCode)));
				Args.Add(FStringFormatArg(Content));
				FString ErrorMessage = FString::Format(TEXT("`/play/conversation`: Error! (Response code: {0}): {1}"), Args);

				UCharismaAPI::Log(-2, ErrorMessage, Error, 5.f);
				ErrorCallback(ErrorMessage);

				return;
			}

			TSharedPtr<FJsonObject> ResponseData = MakeShareable(new FJsonObject);

			const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Content);
			if (!FJsonSerializer::Deserialize(Reader, ResponseData))
			{
				TArray<FStringFormatArg> Args;
				Args.Add(FStringFormatArg(Content));
				FString ErrorMessage =
					FString::Format(TEXT("`/play/conversation`: Error! Failed to deserialize response data: {0}"), Args);

				UCharismaAPI::Log(-2, ErrorMessage, Error, 5.f);
				ErrorCallback(ErrorMessage);
				return;
			}

			const FString ConversationUuid = ResponseData->GetStringField(TEXT("conversationUuid"));
			SuccessCallback(ConversationUuid);
		});

	HttpRequest->ProcessRequest();
}

void UCharismaAPI::SetMemory(const FString& Token, const FString& RecallValue, const FString& SaveValue,
	const TFunction<void()>& SuccessCallback,
	const TFunction<void(const FString Error)>& ErrorCallback)
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

	HttpRequest->OnProcessRequestComplete().BindLambda(
		[SuccessCallback, ErrorCallback](const FHttpRequestPtr Request, const FHttpResponsePtr Response, const bool WasSuccessful) {
			if (!WasSuccessful)
			{
				ErrorCallback(TEXT("`/play/set-memory`: Error! HTTP request was not successful."));
				return;
			}

			const int32 ResponseCode = Response->GetResponseCode();
			const FString Content = Response->GetContentAsString();

			if (ResponseCode != 200)
			{
				TArray<FStringFormatArg> Args;
				Args.Add(FStringFormatArg(FString::FromInt(ResponseCode)));
				Args.Add(FStringFormatArg(Content));
				FString ErrorMessage = FString::Format(TEXT("`/play/set-memory`: Error! (Response code: {0}): {1}"), Args);

				UCharismaAPI::Log(-2, ErrorMessage, Error, 5.f);
				ErrorCallback(ErrorMessage);

				return;
			}
		});

	HttpRequest->ProcessRequest();
}

void UCharismaAPI::RestartFromEventId(const FString& TokenForRestart, const int64 EventId,
	const TFunction<void()>& SuccessCallback,
	const TFunction<void(const FString Error)>& ErrorCallback)
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

	HttpRequest->OnProcessRequestComplete().BindLambda(
		[SuccessCallback, ErrorCallback](const FHttpRequestPtr Request, const FHttpResponsePtr Response, const bool WasSuccessful) {
			if (!WasSuccessful)
			{
				ErrorCallback(TEXT("`/play/restart-from-event`: Error! HTTP request was not successful."));
				return;
			}

			const int32 ResponseCode = Response->GetResponseCode();
			const FString Content = Response->GetContentAsString();

			if (ResponseCode != 200)
			{
				TArray<FStringFormatArg> Args;
				Args.Add(FStringFormatArg(FString::FromInt(ResponseCode)));
				Args.Add(FStringFormatArg(Content));
				FString ErrorMessage = FString::Format(TEXT("`/play/restart-from-event`: Error! (Response code: {0}): {1}"), Args);

				UCharismaAPI::Log(-2, ErrorMessage, Error, 5.f);
				ErrorCallback(ErrorMessage);

				return;
			}

			SuccessCallback();
		});

	HttpRequest->ProcessRequest();
}

void UCharismaAPI::GetMessageHistory(const FString& Token, const FString& ConversationUuid, const int64 MinEventId,
	const TFunction<void(const FCharismaMessageHistoryResponse MessageHistory)>& SuccessCallback,
	const TFunction<void(const FString Error)>& ErrorCallback)
{
	FHttpModule* HttpModule = &FHttpModule::Get();
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = HttpModule->CreateRequest();

	TMap<FString, FString> QueryParams;
	if (ConversationUuid.Len() > 0)
	{
		QueryParams.Add("conversationUuid", ConversationUuid);
	}
	if (MinEventId)
	{
		QueryParams.Add("minEventId", FString::Printf(TEXT("%lld"), MinEventId));
	}

	FString Query = UCharismaAPI::ToQueryString(QueryParams);

	HttpRequest->SetVerb("GET");
	HttpRequest->SetHeader("Authorization", "Bearer " + Token);
	HttpRequest->SetURL(UCharismaAPI::BaseURL + "/play/message-history" + Query);

	HttpRequest->OnProcessRequestComplete().BindLambda(
		[SuccessCallback, ErrorCallback](const FHttpRequestPtr Request, const FHttpResponsePtr Response, const bool WasSuccessful) {
			if (!WasSuccessful)
			{
				ErrorCallback(TEXT("`/play/message-history`: Error! HTTP request was not successful."));
				return;
			}

			const int32 ResponseCode = Response->GetResponseCode();
			const FString Content = Response->GetContentAsString();

			if (ResponseCode != 200)
			{
				TArray<FStringFormatArg> Args;
				Args.Add(FStringFormatArg(FString::FromInt(ResponseCode)));
				Args.Add(FStringFormatArg(Content));
				FString ErrorMessage = FString::Format(TEXT("`/play/message-history`: Error! (Response code: {0}): {1}"), Args);

				UCharismaAPI::Log(-2, ErrorMessage, Error, 5.f);
				ErrorCallback(ErrorMessage);

				return;
			}

			TSharedPtr<FJsonObject> ResponseData = MakeShareable(new FJsonObject);

			FCharismaMessageHistoryResponse MessageHistory;
			if (!FJsonObjectConverter::JsonObjectStringToUStruct(Content, &MessageHistory, 0, 0))
			{
				TArray<FStringFormatArg> Args;
				Args.Add(FStringFormatArg(Content));
				FString ErrorMessage =
					FString::Format(TEXT("`/play/message-history`: Error! Failed to deserialize response data: {0}"), Args);

				UCharismaAPI::Log(-2, ErrorMessage, Error, 5.f);
				ErrorCallback(ErrorMessage);
				return;
			}

			SuccessCallback(MessageHistory);
		});

	HttpRequest->ProcessRequest();
}

void UCharismaAPI::GetPlaythroughInfo(const FString& Token,
	const TFunction<void(const FCharismaPlaythroughInfoResponse PlaythroughInfo)>& SuccessCallback,
	const TFunction<void(const FString Error)>& ErrorCallback)
{
	FHttpModule* HttpModule = &FHttpModule::Get();
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = HttpModule->CreateRequest();

	HttpRequest->SetVerb("GET");
	HttpRequest->SetHeader("Authorization", "Bearer " + Token);
	HttpRequest->SetURL(UCharismaAPI::BaseURL + "/play/playthrough-info");

	HttpRequest->OnProcessRequestComplete().BindLambda(
		[SuccessCallback, ErrorCallback](const FHttpRequestPtr Request, const FHttpResponsePtr Response, const bool WasSuccessful) {
			if (!WasSuccessful)
			{
				ErrorCallback(TEXT("`/play/playthrough-info`: Error! HTTP request was not successful."));
				return;
			}

			const int32 ResponseCode = Response->GetResponseCode();
			const FString Content = Response->GetContentAsString();

			if (ResponseCode != 200)
			{
				TArray<FStringFormatArg> Args;
				Args.Add(FStringFormatArg(FString::FromInt(ResponseCode)));
				Args.Add(FStringFormatArg(Content));
				FString ErrorMessage = FString::Format(TEXT("`/play/playthrough-info`: Error! (Response code: {0}): {1}"), Args);

				UCharismaAPI::Log(-2, ErrorMessage, Error, 5.f);
				ErrorCallback(ErrorMessage);

				return;
			}

			TSharedPtr<FJsonObject> ResponseData = MakeShareable(new FJsonObject);

			FCharismaPlaythroughInfoResponse PlaythroughInfo;
			if (!FJsonObjectConverter::JsonObjectStringToUStruct(Content, &PlaythroughInfo, 0, 0))
			{
				TArray<FStringFormatArg> Args;
				Args.Add(FStringFormatArg(Content));
				FString ErrorMessage =
					FString::Format(TEXT("`/play/playthrough-info`: Error! Failed to deserialize response data: {0}"), Args);

				UCharismaAPI::Log(-2, ErrorMessage, Error, 5.f);
				ErrorCallback(ErrorMessage);
				return;
			}

			SuccessCallback(PlaythroughInfo);
		});

	HttpRequest->ProcessRequest();
}
