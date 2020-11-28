/*
	TiX Engine v2.0 Copyright (C) 2018~2021
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

	class TJSONUtil
	{
	public:
		static vector2di JsonArrayToVector2di(TJSONNode ArrayNode)
		{
			TI_ASSERT(ArrayNode.IsArray() && ArrayNode.Size() == 2);
			vector2di Result;
			Result.X = ArrayNode[0].GetInt();
			Result.Y = ArrayNode[1].GetInt();

			return Result;
		}
		static vector3df JsonArrayToVector3df(TJSONNode ArrayNode)
		{
			TI_ASSERT(ArrayNode.IsArray() && ArrayNode.Size() == 3);
			vector3df Result;
			Result.X = ArrayNode[0].GetFloat();
			Result.Y = ArrayNode[1].GetFloat();
			Result.Z = ArrayNode[2].GetFloat();

			return Result;
		}
		static quaternion JsonArrayToQuaternion(TJSONNode ArrayNode)
		{
			TI_ASSERT(ArrayNode.IsArray() && ArrayNode.Size() == 4);
			quaternion Result;
			Result.X = ArrayNode[0].GetFloat();
			Result.Y = ArrayNode[1].GetFloat();
			Result.Z = ArrayNode[2].GetFloat();
			Result.W = ArrayNode[3].GetFloat();

			return Result;
		}
		static SColorf JsonArrayToSColorf(TJSONNode ArrayNode)
		{
			TI_ASSERT(ArrayNode.IsArray() && ArrayNode.Size() == 4);
			SColorf Result;
			Result.R = ArrayNode[0].GetFloat();
			Result.G = ArrayNode[1].GetFloat();
			Result.B = ArrayNode[2].GetFloat();
			Result.A = ArrayNode[3].GetFloat();

			return Result;
		}
		static aabbox3df JsonArrayToAABBox(TJSONNode ArrayNode)
		{
			TI_ASSERT(ArrayNode.IsArray() && ArrayNode.Size() == 6);
			aabbox3df Result;
			Result.MinEdge.X = ArrayNode[0].GetFloat();
			Result.MinEdge.Y = ArrayNode[1].GetFloat();
			Result.MinEdge.Z = ArrayNode[2].GetFloat();
			Result.MaxEdge.X = ArrayNode[3].GetFloat();
			Result.MaxEdge.Y = ArrayNode[4].GetFloat();
			Result.MaxEdge.Z = ArrayNode[5].GetFloat();

			return Result;
		}
		static void JsonArrayToFloatArray(TJSONNode ArrayNode, float* OutArray, int32 Count)
		{
			TI_ASSERT(ArrayNode.IsArray() && ArrayNode.Size() == Count);
			for (int32 i = 0; i < Count; i++)
			{
				OutArray[i] = ArrayNode[i].GetFloat();
			}
		}

	};
}