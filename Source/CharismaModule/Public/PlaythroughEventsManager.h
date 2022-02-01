#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Playthrough.h"

#include "PlaythroughEventsManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FPlaythroughEventsManagerDelegate, int32, ConversationId, bool, IsConnected, bool,
	IsTyping, const FCharismaMessageEvent&, MessageEvent);

UCLASS()
class CHARISMAMODULE_API UPlaythroughEventsManager : public UBlueprintAsyncActionBase
{
	GENERATED_UCLASS_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"), Category = "")
	static UPlaythroughEventsManager* AttachPlaythroughHandlers(UObject* WorldContextObject, UPlaythrough* Playthrough);

	virtual void Activate() override;

	UPROPERTY()
	UPlaythrough* CurPlaythrough;
	UPROPERTY()
	int32 CurConversationId;
	UPROPERTY()
	bool CurIsConnected = false;
	UPROPERTY()
	bool CurIsTyping = false;
	UPROPERTY()
	FCharismaMessageEvent CurMessageEvent;

private:
	UPROPERTY(BlueprintAssignable, Category = Events)
	FPlaythroughEventsManagerDelegate OnConnected;
	UPROPERTY(BlueprintAssignable, Category = Events)
	FPlaythroughEventsManagerDelegate OnReady;
	UPROPERTY(BlueprintAssignable, Category = Events)
	FPlaythroughEventsManagerDelegate OnTyping;
	UPROPERTY(BlueprintAssignable, Category = Events)
	FPlaythroughEventsManagerDelegate OnMessage;

	UFUNCTION()
	void OnConnectedFUNC(bool IsConnected);
	UFUNCTION()
	void OnReadyFUNC();
	UFUNCTION()
	void OnTypingFUNC(bool IsTyping);
	UFUNCTION()
	void OnMessageFUNC(const FCharismaMessageEvent& MessageEvent);

	UObject* WorldContextObject;
};
