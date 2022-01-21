#pragma once

#include "CharismaEvents.h"
#include "Client.h"
#include "CoreMinimal.h"
#include "Interfaces/IHttpResponse.h"
#include "Room.h"

#include "CharismaAPI.generated.h"

enum ECharismaLogSeverity
{
	Info,
	Warning,
	Error
};

USTRUCT(BlueprintType)
struct FCharismaMessageHistoryResponse
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadWrite)
	TArray<FCharismaMessage> Messages;
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CHARISMAMODULE_API UCharismaAPI : public UObject
{
	GENERATED_BODY()

public:

	static const FString BaseURL;
	static const FString SocketURL;

	UFUNCTION(BlueprintCallable, Category = CharismaAPI)
	static void InitPlaythoughObject(const int32 StoryId, const int32 StoryVersion, const FString& ApiKey, UPlaythrough*& Playthrough);

	static void Log(const int32 Key, const FString& Message, const ECharismaLogSeverity Severity, const float Duration = 5.f);

	UFUNCTION()
	static void CreateCharismaPlaythroughObject(UPlaythrough*& Playthrough);

	UFUNCTION()
	static FString ToQueryString(const TMap<FString, FString>& QueryParams);
};
