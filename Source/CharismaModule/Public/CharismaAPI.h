// Copyright 2022 Charisma Entertainment Ltd

#pragma once

#include "CharismaEvents.h"
#include "Client.h"
#include "CoreMinimal.h"
#include "Interfaces/IHttpResponse.h"
#include "Room.h"

#include "CharismaAPI.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CHARISMAMODULE_API UCharismaAPI : public UObject
{
	GENERATED_BODY()

public:
	static const FString BaseURL;
	static const FString SocketURL;

	static FString ToQueryString(const TMap<FString, FString>& QueryParams);

	static void CreatePlaythroughToken(const int32 StoryId, const int32 StoryVersion, const FString& ApiKey,
		const TFunction<void(const FString Token, const FString PlaythroughUuid)>& SuccessCallback,
		const TFunction<void(const FString Error)>& ErrorCallback);

	static void CreateConversation(const FString& PlaythroughToken,
		const TFunction<void(const FString ConversationUuid)>& SuccessCallback,
		const TFunction<void(const FString Error)>& ErrorCallback);

	static void SetMemory(const FString& PlaythroughToken, const FString& RecallValue, const FString& SaveValue,
		const TFunction<void()>& SuccessCallback, const TFunction<void(const FString Error)>& ErrorCallback);

	static void RestartFromEventId(const FString& PlaythroughToken, const FString& EventId,
		const TFunction<void()>& SuccessCallback,
		const TFunction<void(const FString Error)>& ErrorCallback);

	static void GetMessageHistory(const FString& PlaythroughToken, const FString& ConversationUuid, const FString& MinEventId,
		const TFunction<void(const FCharismaMessageHistoryResponse MessageHistory)>& SuccessCallback,
		const TFunction<void(const FString Error)>& ErrorCallback);

	static void GetPlaythroughInfo(const FString& PlaythroughToken,
		const TFunction<void(const FCharismaPlaythroughInfoResponse PlaythroughInfo)>& SuccessCallback,
		const TFunction<void(const FString Error)>& ErrorCallback);

	static void ForkPlaythrough(const FString& PlaythroughToken,
		const TFunction<void(const FString NewToken, const FString PlaythroughUuid)>& SuccessCallback,
		const TFunction<void(const FString Error)>& ErrorCallback);

	static void ResetPlaythrough(const FString& PlaythroughToken, const FString& EventId, const TFunction<void()>& SuccessCallback,
		const TFunction<void(const FString Error)>& ErrorCallback);
};
