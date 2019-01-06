/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

#include "rapidjson/document.h"
using namespace rapidjson;

namespace tix
{
	class TJSONNodeIterator
	{
	public:
		TJSONNodeIterator(Value::MemberIterator InIter)
			: Iter(InIter)
		{}

		TJSONNodeIterator& operator ++ ()
		{
			++Iter;
			return *this;
		}

		bool operator != (const TJSONNodeIterator& Other)
		{
			return Iter != Other.Iter;
		}

		const int8* Name()
		{
			return Iter->name.GetString();
		}

	private:
		Value::MemberIterator Iter;
	};

	class TJSONNode
	{
	public:
		TJSONNode()
			: JsonValue(nullptr)
		{}

		TJSONNode(Value* InValue)
			: JsonValue(InValue)
		{}

		TJSONNode operator[] (const int8* Name)
		{
			if (IsNull())
				return TJSONNode();

			Value::MemberIterator Member = JsonValue->FindMember(Name);
			if (Member != JsonValue->MemberEnd())
			{
				TJSONNode Node(&(*JsonValue)[Name]);
				return Node;
			}

			return TJSONNode();
		}

		TJSONNode operator[] (int32 Index)
		{
			if (IsNull())
				return TJSONNode();

			TJSONNode Node(&(*JsonValue)[Index]);
			return Node;
		}

		TJSONNodeIterator MemberBegin()
		{
			TI_ASSERT(!IsNull());
			TJSONNodeIterator It(JsonValue->MemberBegin());
			return It;
		}

		TJSONNodeIterator MemberEnd()
		{
			TI_ASSERT(!IsNull());
			TJSONNodeIterator It(JsonValue->MemberEnd());
			return It;
		}

		bool IsNull() const
		{
			return JsonValue == nullptr;
		}

		bool IsArray()
		{
			if (IsNull())
				return false;

			return JsonValue->IsArray();
		}

		bool IsObject()
		{
			if (IsNull())
				return false;

			return JsonValue->IsObject();
		}

		bool IsString()
		{
			if (IsNull())
				return false;

			return JsonValue->IsString();
		}

		int32 Size() const
		{
			if (IsNull())
				return 0;

			return (int32)JsonValue->Size();
		}

		bool GetBool()
		{
			if (IsNull())
				return false;

			return JsonValue->GetBool();
		}

		const int8* GetString()
		{
			static const int8* EmptyStr = "";
			if (IsNull())
				return EmptyStr;

			return JsonValue->GetString();
		}

		int32 GetInt()
		{
			if (IsNull())
				return 0;

			return JsonValue->GetInt();
		}

		float GetFloat()
		{
			if (IsNull())
				return 0.f;

			return JsonValue->GetFloat();
		}

	protected:
		Value* JsonValue;
	};

	class TJSON : public TJSONNode
	{
	public:
		TJSON()
		{}

		~TJSON()
		{}

		void Parse(const int8* JsonText)
		{
			JsonDoc.Parse(JsonText);
			JsonValue = &JsonDoc;
		}

	protected:
		Document JsonDoc;
	};
}