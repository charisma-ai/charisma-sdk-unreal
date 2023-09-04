// Copyright 2022 Charisma Entertainment Ltd

#pragma once

#include "ColyseusUtils.h"
#include "HAL/Platform.h"

#if PLATFORM_WINDOWS
#include "Windows/WindowsHWrapper.h"
#endif

THIRD_PARTY_INCLUDES_START
#pragma push_macro("check")
#undef check
#include <msgpack.hpp>
#pragma pop_macro("check")
THIRD_PARTY_INCLUDES_END

#include "CharismaEvents.generated.h"

// Custom msgpack adapters

namespace msgpack
{
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
{
	namespace adaptor
	{
	template <typename T>
	struct convert<TOptional<T>>
	{
		msgpack::object const& operator()(msgpack::object const& o, TOptional<T>& v) const
		{
			switch (o.type)
			{
				case msgpack::type::NIL:
					v.Reset();
					break;
				default:
					T t;
					msgpack::adaptor::convert<T>()(o, t);
					v = t;
					break;
			}
			return o;
		}
	};

	template <typename T>
	struct pack<TOptional<T>>
	{
		template <typename Stream>
		msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, const TOptional<T>& v) const
		{
			if (v.IsSet())
				o.pack(*v);
			else
				o.pack_nil();
			return o;
		}
	};

	template <>
	struct convert<FString>
	{
		msgpack::object const& operator()(msgpack::object const& o, FString& v) const
		{
			switch (o.type)
			{
				case msgpack::type::BIN:
					v = FString(o.via.bin.size, o.via.bin.ptr);
					break;
				case msgpack::type::STR:
					v = FString(o.via.str.size, o.via.str.ptr);
					break;
				default:
					throw msgpack::type_error();
					break;
			}
			return o;
		}
	};

	template <>
	struct convert<TArray<uint8>>
	{
		msgpack::object const& operator()(msgpack::object const& o, TArray<uint8>& v) const
		{
			switch (o.type)
			{
				case msgpack::type::BIN:
					v.AddUninitialized(o.via.bin.size);
					if (o.via.bin.size != 0)
					{
						FMemory::Memcpy(v.GetData(), o.via.bin.ptr, o.via.bin.size);
					}
					break;
				case msgpack::type::STR:
					v.AddUninitialized(o.via.str.size);
					if (o.via.str.size != 0)
					{
						FMemory::Memcpy(v.GetData(), o.via.str.ptr, o.via.str.size);
					}
					break;
				default:
					throw msgpack::type_error();
					break;
			}
			return o;
		}
	};

	template <>
	struct pack<TArray<uint8>>
	{
		template <typename Stream>
		msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, const TArray<uint8>& v) const
		{
			int32 size = v.Num();
			o.pack_bin(size);
			if (size != 0)
			{
				o.pack_bin_body(reinterpret_cast<const char*>(v.GetData()), size);
			}

			return o;
		}
	};

	template <typename T>
	struct convert<TArray<T>>
	{
		msgpack::object const& operator()(msgpack::object const& o, TArray<T>& v) const
		{
			if (o.type != msgpack::type::ARRAY)
			{
				throw msgpack::type_error();
			}
			v.Reserve(o.via.array.size);
			if (o.via.array.size > 0)
			{
				msgpack::object* p = o.via.array.ptr;
				msgpack::object* const pend = o.via.array.ptr + o.via.array.size;
				// auto it = v.CreateIterator();
				int32 index = 0;
				do
				{
					T tmp;
					p->convert(tmp);
					v.Emplace(tmp);
					++index;
					++p;
				} while (p < pend);
			}
			return o;
		}
	};

	template <typename KeyType, typename ValueType, typename SetAllocator, typename KeyFuncs>
	struct convert<TMap<KeyType, ValueType, SetAllocator, KeyFuncs>>
	{
		msgpack::object const& operator()(msgpack::object const& o, TMap<KeyType, ValueType, SetAllocator, KeyFuncs>& v) const
		{
			if (o.type != msgpack::type::MAP)
			{
				throw msgpack::type_error();
			}
			msgpack::object_kv* p(o.via.map.ptr);
			msgpack::object_kv* const pend(o.via.map.ptr + o.via.map.size);
			for (; p != pend; ++p)
			{
				KeyType key;
				p->key.convert(key);
				ValueType value;
				p->val.convert(value);
				v.Add(key, value);
			}
			return o;
		}
	};
	}	 // namespace adaptor
}	 // MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
}	 // namespace msgpack

// Events sent from server -> client

USTRUCT(BlueprintType)
struct FCharismaActiveEffect
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Charisma|Event")
	FString Feeling;

	UPROPERTY(BlueprintReadOnly, Category = "Charisma|Event")
	float Intensity;

	UPROPERTY(BlueprintReadOnly, Category = "Charisma|Event")
	int32 ReplyPropertiesId;

	UPROPERTY(BlueprintReadOnly, Category = "Charisma|Event")
	int32 CharacterId;

	UPROPERTY(BlueprintReadOnly, Category = "Charisma|Event")
	FString Type;

	UPROPERTY(BlueprintReadOnly, Category = "Charisma|Event")
	int32 DurationRemaining;

	MSGPACK_DEFINE_MAP(MSGPACK_NVP("feeling", Feeling), MSGPACK_NVP("intensity", Intensity),
		MSGPACK_NVP("replyPropertiesId", ReplyPropertiesId), MSGPACK_NVP("characterId", CharacterId), MSGPACK_NVP("type", Type),
		MSGPACK_NVP("durationRemaining", DurationRemaining));
};

USTRUCT(BlueprintType)
struct FCharismaEmotion
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Charisma|Event")
	int32 Id;

	UPROPERTY(BlueprintReadOnly, Category = "Charisma|Event")
	FString Name;

	UPROPERTY(BlueprintReadOnly, Category = "Charisma|Event")
	float MoodPositivity;

	UPROPERTY(BlueprintReadOnly, Category = "Charisma|Event")
	float MoodEnergy;

	UPROPERTY(BlueprintReadOnly, Category = "Charisma|Event")
	float PlayerRelationship;

	UPROPERTY(BlueprintReadOnly, Category = "Charisma|Event")
	TArray<FCharismaActiveEffect> ActiveEffects;

	MSGPACK_DEFINE_MAP(MSGPACK_NVP("id", Id), MSGPACK_NVP("name", Name), MSGPACK_NVP("moodPositivity", MoodPositivity),
		MSGPACK_NVP("moodEnergy", MoodEnergy), MSGPACK_NVP("playerRelationship", PlayerRelationship),
		MSGPACK_NVP("activeEffects", ActiveEffects));
};

USTRUCT(BlueprintType)
struct FCharismaMemory
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Charisma|Event")
	FString RecallValue;

	UPROPERTY(BlueprintReadOnly, Category = "Charisma|Event")
	FString SaveValue;

	TOptional<FString> SaveValue_Optional;

	MSGPACK_DEFINE_MAP(MSGPACK_NVP("recallValue", RecallValue), MSGPACK_NVP("saveValue", SaveValue_Optional));
};

USTRUCT(BlueprintType)
struct FCharismaSpeech
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Charisma|Event")
	TArray<uint8> Audio;

	UPROPERTY(BlueprintReadOnly, Category = "Charisma|Event")
	float Duration;

	UPROPERTY(BlueprintReadOnly, Category = "Charisma|Event")
	FString Encoding;

	MSGPACK_DEFINE_MAP(MSGPACK_NVP("audio", Audio), MSGPACK_NVP("duration", Duration), MSGPACK_NVP("encoding", Encoding));
};

USTRUCT(BlueprintType)
struct FCharismaCharacter
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Charisma|Event")
	int32 Id;

	UPROPERTY(BlueprintReadOnly, Category = "Charisma|Event")
	FString Name;

	UPROPERTY(BlueprintReadOnly, Category = "Charisma|Event")
	FString Avatar;

	TOptional<FString> Avatar_Optional;

	MSGPACK_DEFINE_MAP(MSGPACK_NVP("id", Id), MSGPACK_NVP("name", Name), MSGPACK_NVP("avatar", Avatar_Optional));
};

USTRUCT(BlueprintType)
struct FCharismaMessage
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Charisma|Event")
	FString Text;

	UPROPERTY(BlueprintReadOnly, Category = "Charisma|Event")
	TMap<FString, FString> Metadata;

	UPROPERTY(BlueprintReadOnly, Category = "Charisma|Event")
	FCharismaCharacter Character;

	TOptional<FCharismaCharacter> Character_Optional;

	UPROPERTY(BlueprintReadOnly, Category = "Charisma|Event")
	FCharismaSpeech Speech;

	TOptional<FCharismaSpeech> Speech_Optional;

	MSGPACK_DEFINE_MAP(MSGPACK_NVP("text", Text), MSGPACK_NVP("metadata", Metadata), MSGPACK_NVP("character", Character_Optional),
		MSGPACK_NVP("speech", Speech_Optional));
};

USTRUCT(BlueprintType)
struct FCharismaMessageEvent
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Charisma|Event")
	FString ConversationUuid;

	UPROPERTY(BlueprintReadOnly, Category = "Charisma|Event")
	FString Type;

	UPROPERTY(BlueprintReadOnly, Category = "Charisma|Event")
	FCharismaMessage Message;

	UPROPERTY(BlueprintReadOnly, Category = "Charisma|Event")
	bool EndStory;

	UPROPERTY(BlueprintReadOnly, Category = "Charisma|Event")
	bool TapToContinue;

	UPROPERTY(BlueprintReadOnly, Category = "Charisma|Event")
	TArray<FCharismaEmotion> Emotions;

	UPROPERTY(BlueprintReadOnly, Category = "Charisma|Event")
	TArray<FCharismaMemory> Memories;

	UPROPERTY(BlueprintReadOnly, Category = "Charisma|Event")
	FString EventId;

	UPROPERTY(BlueprintReadOnly, Category = "Charisma|Event")
	int64 Timestamp;

	MSGPACK_DEFINE_MAP(MSGPACK_NVP("conversationUuid", ConversationUuid), MSGPACK_NVP("type", Type),
		MSGPACK_NVP("message", Message), MSGPACK_NVP("endStory", EndStory), MSGPACK_NVP("tapToContinue", TapToContinue),
		MSGPACK_NVP("emotions", Emotions), MSGPACK_NVP("memories", Memories), MSGPACK_NVP("eventId", EventId),
		MSGPACK_NVP("timestamp", Timestamp));
};

USTRUCT(BlueprintType)
struct FCharismaErrorEvent
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Charisma|Event")
	FString Error;

	MSGPACK_DEFINE_MAP(MSGPACK_NVP("error", Error));
};

USTRUCT(BlueprintType)
struct FCharismaPlaythroughInfoResponse
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Charisma|Event")
	TArray<FCharismaEmotion> Emotions;

	UPROPERTY(BlueprintReadOnly, Category = "Charisma|Event")
	TArray<FCharismaMemory> Memories;
};

USTRUCT(BlueprintType)
struct FCharismaMessageHistoryResponse
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Charisma|Event")
	TArray<FCharismaMessage> Messages;
};

USTRUCT(BlueprintType)
struct FCharismaSpeechRecognitionResultEvent
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Charisma|Event")
	FString Text;

	UPROPERTY(BlueprintReadOnly, Category = "Charisma|Event")
	bool SpeechFinal;

	UPROPERTY(BlueprintReadOnly, Category = "Charisma|Event")
	bool IsFinal;

	MSGPACK_DEFINE_MAP(MSGPACK_NVP("speechFinal", SpeechFinal), MSGPACK_NVP("isFinal", IsFinal), MSGPACK_NVP("text", Text));
};

struct FSpeechRecognitionErrorResult
{
	std::string Message;
	std::string ErrorOccuredWhen;

	MSGPACK_DEFINE_MAP(MSGPACK_NVP("message", Message), MSGPACK_NVP("errorOccuredWhen", ErrorOccuredWhen));
};

// Events sent from client -> server

struct SpeechConfig
{
	std::vector<std::string> encoding;
	std::string output;
	MSGPACK_DEFINE_MAP(encoding, output);
};

struct ActionPayload
{
	std::string conversationUuid;
	std::string action;
	TOptional<SpeechConfig> speechConfig;
	MSGPACK_DEFINE_MAP(conversationUuid, action, speechConfig);
};

struct StartPayload
{
	std::string conversationUuid;
	TOptional<int32> sceneIndex;
	TOptional<int32> startGraphId;
	TOptional<std::string> startGraphReferenceId;
	TOptional<SpeechConfig> speechConfig;
	MSGPACK_DEFINE_MAP(conversationUuid, sceneIndex, startGraphId, startGraphReferenceId, speechConfig);
};

struct TapPayload
{
	std::string conversationUuid;
	TOptional<SpeechConfig> speechConfig;
	MSGPACK_DEFINE_MAP(conversationUuid, speechConfig);
};

struct ReplyPayload
{
	std::string conversationUuid;
	std::string text;
	TOptional<SpeechConfig> speechConfig;
	MSGPACK_DEFINE_MAP(conversationUuid, text, speechConfig);
};

struct ResumePayload
{
	std::string conversationUuid;
	TOptional<SpeechConfig> speechConfig;
	MSGPACK_DEFINE_MAP(conversationUuid, speechConfig);
};

struct SpeechRecognitionStartPayload
{
	std::string service;
	TOptional<int> sampleRate;
	TOptional<std::string> languageCode;
	TOptional<std::string> encoding;

	MSGPACK_DEFINE_MAP(service, sampleRate, languageCode, encoding);
};
