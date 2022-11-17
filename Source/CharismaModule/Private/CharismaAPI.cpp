// Copyright 2022 Charisma Entertainment Ltd

#include "CharismaAPI.h"

#include "CharismaLogger.h"
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
			CharismaLogger::Log(-3, "An API key must be provided to create a playthrough token for the draft story version.",
				CharismaLogger::Error);
		}
	}

	HttpRequest->SetVerb("POST");
	HttpRequest->AppendToHeader("Content-Type", "application/json");
	HttpRequest->SetURL(BaseURL + "/play/token");
	HttpRequest->SetContentAsString(OutputString);

	HttpRequest->OnProcessRequestComplete().BindLambda(
		[SuccessCallback, ErrorCallback](const FHttpRequestPtr Request, const FHttpResponsePtr Response, const bool WasSuccessful)
		{
			if (!WasSuccessful)
			{
				ErrorCallback(TEXT("`/play/token`: Error! HTTP request was not successful."));
				return;
			}

			const int32 ResponseCode = Response->GetResponseCode();
			const FString Content = Response->GetContentAsString();

			if (ResponseCode != 200)
			{
				FString ErrorMessage =
					FString::Printf(TEXT("`/play/token`: Error! (Response code: %d): %s"), ResponseCode, *Content);

				CharismaLogger::Log(-2, ErrorMessage, CharismaLogger::Error);
				ErrorCallback(ErrorMessage);

				return;
			}

			TSharedPtr<FJsonObject> ResponseData = MakeShareable(new FJsonObject);

			const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Content);
			if (!FJsonSerializer::Deserialize(Reader, ResponseData))
			{
				FString ErrorMessage =
					FString::Printf(TEXT("`/play/token`: Error! Failed to deserialize response data: %s"), *Content);

				CharismaLogger::Log(-2, ErrorMessage, CharismaLogger::Error);
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
		[SuccessCallback, ErrorCallback](const FHttpRequestPtr Request, const FHttpResponsePtr Response, const bool WasSuccessful)
		{
			if (!WasSuccessful)
			{
				ErrorCallback(TEXT("`/play/conversation`: Error! HTTP request was not successful."));
				return;
			}

			const int32 ResponseCode = Response->GetResponseCode();
			const FString Content = Response->GetContentAsString();

			if (ResponseCode != 200)
			{
				FString ErrorMessage =
					FString::Printf(TEXT("`/play/conversation`: Error! (Response code: %d): %s"), ResponseCode, *Content);

				CharismaLogger::Log(-2, ErrorMessage, CharismaLogger::Error);
				ErrorCallback(ErrorMessage);

				return;
			}

			TSharedPtr<FJsonObject> ResponseData = MakeShareable(new FJsonObject);

			const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Content);
			if (!FJsonSerializer::Deserialize(Reader, ResponseData))
			{
				FString ErrorMessage =
					FString::Printf(TEXT("`/play/conversation`: Error! Failed to deserialize response data: %s"), *Content);

				CharismaLogger::Log(-2, ErrorMessage, CharismaLogger::Error);
				ErrorCallback(ErrorMessage);
				return;
			}

			const FString ConversationUuid = ResponseData->GetStringField(TEXT("conversationUuid"));
			SuccessCallback(ConversationUuid);
		});

	HttpRequest->ProcessRequest();
}

void UCharismaAPI::SetMemory(const FString& Token, const FString& RecallValue, const FString& SaveValue,
	const TFunction<void()>& SuccessCallback, const TFunction<void(const FString Error)>& ErrorCallback)
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
		[SuccessCallback, ErrorCallback](const FHttpRequestPtr Request, const FHttpResponsePtr Response, const bool WasSuccessful)
		{
			if (!WasSuccessful)
			{
				ErrorCallback(TEXT("`/play/set-memory`: Error! HTTP request was not successful."));
				return;
			}

			const int32 ResponseCode = Response->GetResponseCode();
			const FString Content = Response->GetContentAsString();

			if (ResponseCode != 200)
			{
				FString ErrorMessage =
					FString::Printf(TEXT("`/play/set-memory`: Error! (Response code: %d): %s"), ResponseCode, *Content);

				CharismaLogger::Log(-2, ErrorMessage, CharismaLogger::Error);
				ErrorCallback(ErrorMessage);

				return;
			}

			SuccessCallback();
		});

	HttpRequest->ProcessRequest();
}

void UCharismaAPI::RestartFromEventId(const FString& TokenForRestart, const FString& EventId,
	const TFunction<void()>& SuccessCallback,
	const TFunction<void(const FString Error)>& ErrorCallback)
{
	TSharedPtr<FJsonObject> RequestData = MakeShareable(new FJsonObject);
	RequestData->SetStringField("eventId", EventId);

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
		[SuccessCallback, ErrorCallback](const FHttpRequestPtr Request, const FHttpResponsePtr Response, const bool WasSuccessful)
		{
			if (!WasSuccessful)
			{
				ErrorCallback(TEXT("`/play/restart-from-event`: Error! HTTP request was not successful."));
				return;
			}

			const int32 ResponseCode = Response->GetResponseCode();
			const FString Content = Response->GetContentAsString();

			if (ResponseCode != 200)
			{
				FString ErrorMessage =
					FString::Printf(TEXT("`/play/restart-from-event`: Error! (Response code: %d): %s"), ResponseCode, *Content);

				CharismaLogger::Log(-2, ErrorMessage, CharismaLogger::Error);
				ErrorCallback(ErrorMessage);

				return;
			}

			SuccessCallback();
		});

	HttpRequest->ProcessRequest();
}

void UCharismaAPI::GetMessageHistory(const FString& Token, const FString& ConversationUuid, const FString& MinEventId,
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
	if (MinEventId.Len() > 0)
	{
		QueryParams.Add("minEventId", MinEventId);
	}

	FString Query = UCharismaAPI::ToQueryString(QueryParams);

	HttpRequest->SetVerb("GET");
	HttpRequest->SetHeader("Authorization", "Bearer " + Token);
	HttpRequest->SetURL(UCharismaAPI::BaseURL + "/play/message-history" + Query);

	HttpRequest->OnProcessRequestComplete().BindLambda(
		[SuccessCallback, ErrorCallback](const FHttpRequestPtr Request, const FHttpResponsePtr Response, const bool WasSuccessful)
		{
			if (!WasSuccessful)
			{
				ErrorCallback(TEXT("`/play/message-history`: Error! HTTP request was not successful."));
				return;
			}

			const int32 ResponseCode = Response->GetResponseCode();
			const FString Content = Response->GetContentAsString();

			if (ResponseCode != 200)
			{
				FString ErrorMessage =
					FString::Printf(TEXT("`/play/message-history`: Error! (Response code: %d): %s"), ResponseCode, *Content);

				CharismaLogger::Log(-2, ErrorMessage, CharismaLogger::Error);
				ErrorCallback(ErrorMessage);

				return;
			}

			TSharedPtr<FJsonObject> ResponseData = MakeShareable(new FJsonObject);

			FCharismaMessageHistoryResponse MessageHistory;
			if (!FJsonObjectConverter::JsonObjectStringToUStruct(Content, &MessageHistory, 0, 0))
			{
				FString ErrorMessage =
					FString::Printf(TEXT("`/play/message-history`: Error! Failed to deserialize response data: %s"), *Content);

				CharismaLogger::Log(-2, ErrorMessage, CharismaLogger::Error);
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
		[SuccessCallback, ErrorCallback](const FHttpRequestPtr Request, const FHttpResponsePtr Response, const bool WasSuccessful)
		{
			if (!WasSuccessful)
			{
				ErrorCallback(TEXT("`/play/playthrough-info`: Error! HTTP request was not successful."));
				return;
			}

			const int32 ResponseCode = Response->GetResponseCode();
			const FString Content = Response->GetContentAsString();

			if (ResponseCode != 200)
			{
				FString ErrorMessage =
					FString::Printf(TEXT("`/play/playthrough-info`: Error! (Response code: %d): %s"), ResponseCode, *Content);

				CharismaLogger::Log(-2, ErrorMessage, CharismaLogger::Error);
				ErrorCallback(ErrorMessage);

				return;
			}

			TSharedPtr<FJsonObject> ResponseData = MakeShareable(new FJsonObject);

			FCharismaPlaythroughInfoResponse PlaythroughInfo;
			if (!FJsonObjectConverter::JsonObjectStringToUStruct(Content, &PlaythroughInfo, 0, 0))
			{
				FString ErrorMessage =
					FString::Printf(TEXT("`/play/playthrough-info`: Error! Failed to deserialize response data: %s"), *Content);

				CharismaLogger::Log(-2, ErrorMessage, CharismaLogger::Error);
				ErrorCallback(ErrorMessage);
				return;
			}

			SuccessCallback(PlaythroughInfo);
		});

	HttpRequest->ProcessRequest();
}

void UCharismaAPI::ForkPlaythrough(const FString& Token,
	const TFunction<void(const FString NewToken, const FString PlaythroughUuid)>& SuccessCallback,
	const TFunction<void(const FString Error)>& ErrorCallback)
{
	FHttpModule* HttpModule = &FHttpModule::Get();
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = HttpModule->CreateRequest();

	HttpRequest->SetVerb("POST");
	HttpRequest->SetHeader("Authorization", "Bearer " + Token);
	HttpRequest->SetURL(UCharismaAPI::BaseURL + "/play/fork-playthrough");

	HttpRequest->OnProcessRequestComplete().BindLambda(
		[SuccessCallback, ErrorCallback](const FHttpRequestPtr Request, const FHttpResponsePtr Response, const bool WasSuccessful)
		{
			if (!WasSuccessful)
			{
				ErrorCallback(TEXT("`/play/fork-playthrough`: Error! HTTP request was not successful."));
				return;
			}

			const int32 ResponseCode = Response->GetResponseCode();
			const FString Content = Response->GetContentAsString();

			if (ResponseCode != 200)
			{
				const FString ErrorMessage = FString::Printf(
					TEXT("`/play/fork-playthrough`: Error! (Response code: %d): %s"), ResponseCode, *Content);
				CharismaLogger::Log(-2, ErrorMessage, CharismaLogger::Error);
				ErrorCallback(ErrorMessage);

				return;
			}

			TSharedPtr<FJsonObject> ResponseData = MakeShareable(new FJsonObject);

			const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Content);
			if (!FJsonSerializer::Deserialize(Reader, ResponseData))
			{
				FString ErrorMessage =
					FString::Printf(TEXT("`/play/fork-playthrough`: Error! Failed to deserialize response data: %s"), *Content);

				CharismaLogger::Log(-2, ErrorMessage, CharismaLogger::Error);
				ErrorCallback(ErrorMessage);
				return;
			}

			const FString NewToken = ResponseData->GetStringField(TEXT("token"));
			const FString NewPlaythroughUuid = ResponseData->GetStringField(TEXT("playthroughUuid"));

			SuccessCallback(NewToken, NewPlaythroughUuid);
		});

	HttpRequest->ProcessRequest();
}

void UCharismaAPI::ResetPlaythrough(const FString& Token, const FString& EventId, const TFunction<void()>& SuccessCallback,
	const TFunction<void(const FString Error)>& ErrorCallback)
{
	TSharedPtr<FJsonObject> RequestData = MakeShareable(new FJsonObject);
	RequestData->SetStringField("eventId", EventId);

	FString OutputString;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(RequestData.ToSharedRef(), Writer);

	FHttpModule* HttpModule = &FHttpModule::Get();
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = HttpModule->CreateRequest();

	HttpRequest->SetVerb("POST");
	HttpRequest->SetHeader("Authorization", "Bearer " + Token);
	HttpRequest->AppendToHeader("Content-Type", "application/json");
	HttpRequest->SetURL(UCharismaAPI::BaseURL + "/play/reset-playthrough");
	HttpRequest->SetContentAsString(OutputString);

	HttpRequest->OnProcessRequestComplete().BindLambda(
		[SuccessCallback, ErrorCallback](const FHttpRequestPtr Request, const FHttpResponsePtr Response, const bool WasSuccessful)
		{
			if (!WasSuccessful)
			{
				ErrorCallback(TEXT("`/play/reset-playthrough`: Error! HTTP request was not successful."));
				return;
			}

			const int32 ResponseCode = Response->GetResponseCode();
			const FString Content = Response->GetContentAsString();

			if (ResponseCode != 200)
			{
				FString ErrorMessage =
					FString::Printf(TEXT("`/play/reset-playthrough`: Error! (Response code: %d): %s"), ResponseCode, *Content);

				CharismaLogger::Log(-2, ErrorMessage, CharismaLogger::Error);
				ErrorCallback(ErrorMessage);

				return;
			}

			SuccessCallback();
		});

	HttpRequest->ProcessRequest();
}
