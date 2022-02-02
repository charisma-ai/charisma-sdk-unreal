#include "APIBlueprints/GetMessageHistory.h"

UGetMessageHistory::UGetMessageHistory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer), WorldContextObject(nullptr)
{
}

UGetMessageHistory* UGetMessageHistory::GetMessageHistory(
	UObject* WorldContextObject, const FString Token, const FString ConversationUuid, const int64 MinEventId)
{
	UGetMessageHistory* BlueprintNode = NewObject<UGetMessageHistory>();
	BlueprintNode->WorldContextObject = WorldContextObject;
	BlueprintNode->Token = Token;
	BlueprintNode->ConversationUuid = ConversationUuid;
	BlueprintNode->MinEventId = MinEventId;
	return BlueprintNode;
}

void UGetMessageHistory::Activate()
{
	UCharismaAPI::GetMessageHistory(
		Token, ConversationUuid, MinEventId,
		[&](const FCharismaMessageHistoryResponse MessageHistory) { Success.Broadcast(MessageHistory, TEXT("")); },
		[&](const FString Error) { Failure.Broadcast(FCharismaMessageHistoryResponse(), Error); });
}
