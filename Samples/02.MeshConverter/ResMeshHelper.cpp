/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "ResHelper.h"
#include "ResMeshHelper.h"

void ConvertJArrayToArray(const Value& JArray, TVector<float>& OutArray)
{
	TI_ASSERT(JArray.IsArray());
	SizeType JArraySize = JArray.Size();
	OutArray.reserve(JArraySize);
	for (SizeType i = 0; i < JArraySize; ++i)
	{
		OutArray.push_back(JArray[i].GetFloat());
	}
}
void ConvertJArrayToArray(const Value& JArray, TVector<int32>& OutArray)
{
	TI_ASSERT(JArray.IsArray());
	SizeType JArraySize = JArray.Size();
	OutArray.reserve(JArraySize);
	for (SizeType i = 0; i < JArraySize; ++i)
	{
		OutArray.push_back(JArray[i].GetInt());
	}
}
void ConvertJArrayToArray(const Value& JArray, TVector<TString>& OutArray)
{
	TI_ASSERT(JArray.IsArray());
	SizeType JArraySize = JArray.Size();
	OutArray.reserve(JArraySize);
	for (SizeType i = 0; i < JArraySize; ++i)
	{
		OutArray.push_back(JArray[i].GetString());
	}
}

inline int32 GetSegmentElements(E_VERTEX_STREAM_SEGMENT Segment)
{
	if (Segment == EVSSEG_POSITION ||
		Segment == EVSSEG_NORMAL ||
		Segment == EVSSEG_TANGENT)
		return 3;
	if (Segment == EVSSEG_TEXCOORD0 ||
		Segment == EVSSEG_TEXCOORD1)
		return 2;
	if (Segment == EVSSEG_COLOR ||
		Segment == EVSSEG_BLENDINDEX ||
		Segment == EVSSEG_BLENDWEIGHT)
		return 4;
	TI_ASSERT(0);
	return 0;
}

namespace tix
{
	void TResMeshDefine::AddSegment(E_MESH_STREAM_INDEX InStreamType, float* InData, int32 InStrideInByte)
	{
		TI_ASSERT(InStrideInByte % sizeof(float) == 0);
		Segments[InStreamType] = ti_new TResMeshSegment;
		Segments[InStreamType]->Data = InData;
		Segments[InStreamType]->StrideInFloat = InStrideInByte / sizeof(float);
	}

	void TResMeshDefine::SetFaces(int32* Indices, int32 Count)
	{
		Faces.Data = Indices;
		Faces.Count = Count;
	}

	/////////////////////////////////////////////////////////////////

	TResMeshHelper::TResMeshHelper()
	{
	}

	TResMeshHelper::~TResMeshHelper()
	{
	}

	TResMeshDefine& TResMeshHelper::AddMesh(const TString& Name, int32 NumVertices, int32 NumTriangles)
	{
		Meshes.push_back(TResMeshDefine(Name, NumVertices, NumTriangles));
		return Meshes.back();
	}

	void TResMeshHelper::OutputMesh(TStream& OutStream, TVector<TString>& OutStrings)
	{
		TResfileChunkHeader ChunkHeader;
		ChunkHeader.ID = TIRES_ID_CHUNK_MESH;
		ChunkHeader.Version = TIRES_VERSION_CHUNK_MESH;
		ChunkHeader.ElementCount = (int32)Meshes.size();

		TStream HeaderStream, DataStream(1024 * 8);
		for (int32 m = 0; m < ChunkHeader.ElementCount; ++m)
		{
			const TResMeshDefine& Mesh = Meshes[m];
			TI_ASSERT(Mesh.Segments[ESSI_POSITION] != nullptr);

			// init header
			THeaderMesh MeshHeader;
			MeshHeader.StrId_Name = AddStringToList(OutStrings, Mesh.Name);
			MeshHeader.VertexFormat = 0;
			for (int32 s = 0; s < ESSI_TOTAL; ++s)
			{
				if (Mesh.Segments[s] != nullptr)
				{
					MeshHeader.VertexFormat |= (1 << s);
				}
			}
			MeshHeader.VertexCount = Mesh.NumVertices;
			MeshHeader.PrimitiveCount = Mesh.NumTriangles;
			MeshHeader.IndexType = Mesh.NumVertices > 65535 ? EIT_32BIT : EIT_16BIT;
			MeshHeader.Flag = 0;
			vector3df FirstPosition(Mesh.Segments[ESSI_POSITION]->Data[0], Mesh.Segments[ESSI_POSITION]->Data[1], Mesh.Segments[ESSI_POSITION]->Data[2]);
			MeshHeader.BBox.reset(FirstPosition);

			// fill data
			// - vertices
			float* DataPos = Mesh.Segments[ESSI_POSITION] ? Mesh.Segments[ESSI_POSITION]->Data : nullptr;
			float* DataNormal = Mesh.Segments[ESSI_NORMAL] ? Mesh.Segments[ESSI_NORMAL]->Data : nullptr;
			float* DataColor = Mesh.Segments[ESSI_COLOR] ? Mesh.Segments[ESSI_COLOR]->Data : nullptr;
			float* DataUv0 = Mesh.Segments[ESSI_TEXCOORD0] ? Mesh.Segments[ESSI_TEXCOORD0]->Data : nullptr;
			float* DataUv1 = Mesh.Segments[ESSI_TEXCOORD1] ? Mesh.Segments[ESSI_TEXCOORD1]->Data : nullptr;
			float* DataTangent = Mesh.Segments[ESSI_TANGENT] ? Mesh.Segments[ESSI_TANGENT]->Data : nullptr;
			float* DataBI = Mesh.Segments[ESSI_BLENDINDEX] ? Mesh.Segments[ESSI_BLENDINDEX]->Data : nullptr;
			float* DataBW = Mesh.Segments[ESSI_BLENDWEIGHT] ? Mesh.Segments[ESSI_BLENDWEIGHT]->Data : nullptr;

			TI_ASSERT(DataPos != nullptr);
			const int32 VertexStart = DataStream.GetLength();
			for (int32 v = 0; v < Mesh.NumVertices; ++v)
			{
				if (DataPos != nullptr)
				{
					vector3df pos(DataPos[0], DataPos[1], DataPos[2]);
					MeshHeader.BBox.addInternalPoint(pos);

					TI_ASSERT(sizeof(vector3df) == TMeshBuffer::SemanticSize[ESSI_POSITION]);
					DataStream.Put(DataPos, sizeof(vector3df));
					DataPos += Mesh.Segments[ESSI_POSITION]->StrideInFloat;
				}
				if (DataNormal != nullptr)
				{
					uint8 NData[4];
					NData[0] = FloatToUNorm(DataNormal[0]);
					NData[1] = FloatToUNorm(DataNormal[1]);
					NData[2] = FloatToUNorm(DataNormal[2]);
					NData[3] = 255;

					TI_ASSERT(sizeof(NData) == TMeshBuffer::SemanticSize[ESSI_NORMAL]);
					DataStream.Put(NData, sizeof(NData));
					DataNormal += Mesh.Segments[ESSI_NORMAL]->StrideInFloat;
				}
				if (DataColor != nullptr)
				{
					uint8 CData[4];
					CData[0] = FloatToUNorm(DataColor[0]);
					CData[1] = FloatToUNorm(DataColor[1]);
					CData[2] = FloatToUNorm(DataColor[2]);
					CData[3] = FloatToUNorm(DataColor[3]);

					TI_ASSERT(sizeof(CData) == TMeshBuffer::SemanticSize[ESSI_COLOR]);
					DataStream.Put(CData, sizeof(CData));
					DataColor += Mesh.Segments[ESSI_COLOR]->StrideInFloat;
				}
				if (DataUv0 != nullptr)
				{
					TI_TODO("use full precision uv coord temp, change to half in futher.");
					TI_ASSERT(sizeof(float) * 2 == TMeshBuffer::SemanticSize[ESSI_TEXCOORD0]);
					DataStream.Put(DataUv0, sizeof(float) * 2);
					DataUv0 += Mesh.Segments[ESSI_TEXCOORD0]->StrideInFloat;
				}
				if (DataUv1 != nullptr)
				{
					TI_TODO("use full precision uv coord temp, change to half in futher.");
					TI_ASSERT(sizeof(float) * 2 == TMeshBuffer::SemanticSize[ESSI_TEXCOORD1]);
					DataStream.Put(DataUv1, sizeof(float) * 2);
					DataUv1 += Mesh.Segments[ESSI_TEXCOORD1]->StrideInFloat;
				}
				if (DataTangent != nullptr)
				{
					uint8 TData[4];
					TData[0] = FloatToUNorm(DataTangent[0]);
					TData[1] = FloatToUNorm(DataTangent[1]);
					TData[2] = FloatToUNorm(DataTangent[2]);
					TData[3] = 255;

					TI_ASSERT(sizeof(TData) == TMeshBuffer::SemanticSize[ESSI_TANGENT]);
					DataStream.Put(TData, sizeof(TData));
					DataTangent += Mesh.Segments[ESSI_TANGENT]->StrideInFloat;
				}
				if (DataBI != nullptr)
				{
					TI_TODO("use float precision blend index temp, change to uint8 in futher.");
					TI_ASSERT(sizeof(float) * 4 == TMeshBuffer::SemanticSize[ESSI_BLENDINDEX]);
					DataStream.Put(DataBI, sizeof(float) * 4);
					DataBI += Mesh.Segments[ESSI_BLENDINDEX]->StrideInFloat;
				}
				if (DataBW != nullptr)
				{
					TI_ASSERT(sizeof(float) * 4 == TMeshBuffer::SemanticSize[ESSI_BLENDWEIGHT]);
					DataStream.Put(DataBW, sizeof(float) * 4);
					DataBW += Mesh.Segments[ESSI_BLENDWEIGHT]->StrideInFloat;
				}
			}
			const int32 VertexEnd = DataStream.GetLength();

			// 8 bytes align
			TI_ASSERT((VertexEnd - VertexStart) % 4 == 0);
			FillZero4(DataStream);

			// - Indices
			const int32 IndexStart = DataStream.GetLength();
			TI_ASSERT(Mesh.Faces.Count == MeshHeader.PrimitiveCount * 3);
			if (MeshHeader.IndexType == EIT_16BIT)
			{
				for (int32 i = 0; i < Mesh.Faces.Count; ++i)
				{
					uint16 Index = (uint16)Mesh.Faces.Data[i];
					DataStream.Put(&Index, sizeof(uint16));
				}
			}
			else
			{
				for (int32 i = 0; i < Mesh.Faces.Count; ++i)
				{
					uint32 Index = (uint32)Mesh.Faces.Data[i];
					DataStream.Put(&Index, sizeof(uint32));
				}
			}
			const int32 IndexEnd = DataStream.GetLength();
			FillZero4(DataStream);

			// Fill header
			const int32 HeaderStart = HeaderStream.GetLength();
			HeaderStream.Put(&MeshHeader, sizeof(THeaderMesh));
			const int32 HeaderEnd = HeaderStream.GetLength();
			FillZero4(HeaderStream);
		}
		ChunkHeader.ChunkSize = HeaderStream.GetLength() + DataStream.GetLength();

		OutStream.Put(&ChunkHeader, sizeof(TResfileChunkHeader));
		FillZero4(OutStream);
		OutStream.Put(HeaderStream.GetBuffer(), HeaderStream.GetLength());
		OutStream.Put(DataStream.GetBuffer(), DataStream.GetLength());
	}

	bool TResMeshHelper::LoadMeshFile(rapidjson::Document& Doc, TStream& OutStream, TVector<TString>& OutStrings)
	{
		TResMeshHelper ResMesh;

		TString Name = Doc["name"].GetString();
		int32 Version = Doc["version"].GetInt();

		int32 VCount = Doc["vertex_count_total"].GetInt();
		int32 ICount = Doc["index_count_total"].GetInt();
		int32 UVCount = Doc["texcoord_count"].GetInt();

		Value& Sections = Doc["sections"];

		TVector<float> Vertices;
		TVector<int32> Indices;
		TVector<TString> SFormat;

		// Only support 1 section for now.
		TI_ASSERT(Sections.Size() == 1);
		for (SizeType i = 0; i < Sections.Size(); ++i)
		{
			Value& Section = Sections[i];
			int32 VertexCount = Section["vertex_count"].GetInt();
			Value& JVertices = Section["vertices"];
			Value& JIndices = Section["indices"];
			Value& JVsFormat = Section["vs_format"];

			Vertices.clear();
			Indices.clear();
			SFormat.clear();

			ConvertJArrayToArray(JVertices, Vertices);
			ConvertJArrayToArray(JIndices, Indices);
			ConvertJArrayToArray(JVsFormat, SFormat);

			int32 VsFormat = 0;
			int32 ElementsStride = 0;
			for (const auto& S : SFormat)
			{
				E_VERTEX_STREAM_SEGMENT Segment = GetVertexSegment(S);
				VsFormat |= Segment;
				ElementsStride += GetSegmentElements(Segment);
			}
			TI_ASSERT(VertexCount * ElementsStride == Vertices.size());

			const int32 BytesStride = ElementsStride * sizeof(float);
			TResMeshDefine& Mesh = ResMesh.AddMesh("DefaultMesh", (int32)VertexCount, (int32)Indices.size() / 3);
			int32 ElementOffset = 0;
			{
				TI_ASSERT((VsFormat & EVSSEG_POSITION) != 0);
				Mesh.AddSegment(ESSI_POSITION, (float*)&Vertices[ElementOffset], BytesStride);
				ElementOffset += 3;
			}
			if ((VsFormat & EVSSEG_NORMAL) != 0)
			{
				Mesh.AddSegment(ESSI_NORMAL, (float*)&Vertices[ElementOffset], BytesStride);
				ElementOffset += 3;
			}
			if ((VsFormat & EVSSEG_COLOR) != 0)
			{
				Mesh.AddSegment(ESSI_COLOR, (float*)&Vertices[ElementOffset], BytesStride);
				ElementOffset += 4;
			}
			if ((VsFormat & EVSSEG_TEXCOORD0) != 0)
			{
				Mesh.AddSegment(ESSI_TEXCOORD0, (float*)&Vertices[ElementOffset], BytesStride);
				ElementOffset += 2;
			}
			if ((VsFormat & EVSSEG_TEXCOORD1) != 0)
			{
				Mesh.AddSegment(ESSI_TEXCOORD1, (float*)&Vertices[ElementOffset], BytesStride);
				ElementOffset += 2;
			}
			if ((VsFormat & EVSSEG_TANGENT) != 0)
			{
				Mesh.AddSegment(ESSI_TANGENT, (float*)&Vertices[ElementOffset], BytesStride);
				ElementOffset += 3;
			}
			if ((VsFormat & EVSSEG_BLENDINDEX) != 0)
			{
				Mesh.AddSegment(ESSI_BLENDINDEX, (float*)&Vertices[ElementOffset], BytesStride);
				ElementOffset += 4;
			}
			if ((VsFormat & EVSSEG_BLENDWEIGHT) != 0)
			{
				Mesh.AddSegment(ESSI_BLENDWEIGHT, (float*)&Vertices[ElementOffset], BytesStride);
				ElementOffset += 4;
			}
			Mesh.SetFaces(&Indices[0], (int32)Indices.size());
		}

		ResMesh.OutputMesh(OutStream, OutStrings);
		return true;
	}
}
