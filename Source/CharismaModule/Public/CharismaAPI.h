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

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FTokenDelegate, FString, Token, int32, PlaythroughId);


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CHARISMAMODULE_API UCharismaAPI : public UObject
{
	GENERATED_BODY()

public:

	// Static

	static const FString BaseURL;
	static const FString SocketURL;

	UFUNCTION(BlueprintCallable, Category = CharismaAPI)
	static UPlaythrough* CreatePlaythrough(UObject* Owner, UCharismaAPI*& Charisma);

	static void Log(const int32 Key, const FString& Message, const ECharismaLogSeverity Severity, const float Duration = 5.f);

	// Member

	UFUNCTION(BlueprintCallable, Category = Setup)
	void CreatePlaythroughToken(const int32 StoryId, const int32 StoryVersion, const FString& ApiKey) const;

	UFUNCTION()
	static FString ToQueryString(const TMap<FString, FString>& QueryParams);


	// Properties

	UPROPERTY(BlueprintReadWrite)
	FString PlaythroughToken;

	// Events

	UPROPERTY(BlueprintAssignable, Category = Events)
	FTokenDelegate OnTokenCreated;

private:
	// Member

	void OnTokenRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool WasSuccessful) const;

};
