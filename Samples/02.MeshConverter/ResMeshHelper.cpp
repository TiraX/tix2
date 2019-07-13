/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "ResHelper.h"
#include "ResMeshHelper.h"
#include "TMeshBufferSemantic.h"

void ConvertJArrayToArray(TJSONNode& JArray, TVector<float>& OutArray)
{
	TI_ASSERT(JArray.IsArray());
	int32 JArraySize = JArray.Size();
	OutArray.reserve(JArraySize);
	for (int32 i = 0; i < JArraySize; ++i)
	{
		OutArray.push_back(JArray[i].GetFloat());
	}
}
void ConvertJArrayToArray(TJSONNode& JArray, TVector<int32>& OutArray)
{
	TI_ASSERT(JArray.IsArray());
	int32 JArraySize = JArray.Size();
	OutArray.reserve(JArraySize);
	for (int32 i = 0; i < JArraySize; ++i)
	{
		OutArray.push_back(JArray[i].GetInt());
	}
}
void ConvertJArrayToArray(TJSONNode& JArray, TVector<TString>& OutArray)
{
	TI_ASSERT(JArray.IsArray());
	int32 JArraySize = JArray.Size();
	OutArray.reserve(JArraySize);
	for (int32 i = 0; i < JArraySize; ++i)
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
		Segments[InStreamType].Data = InData;
		Segments[InStreamType].StrideInFloat = InStrideInByte / sizeof(float);
	}

	void TResMeshDefine::SetFaces(int32* Indices, int32 Count)
	{
		Faces.Data = Indices;
		Faces.Count = Count;
	}

	void TResMeshDefine::SetMaterial(const TString& MaterialName)
	{
		LinkedMaterialInstance = MaterialName;
	}

	/////////////////////////////////////////////////////////////////

	TResMeshHelper::TResMeshHelper()
	{
	}

	TResMeshHelper::~TResMeshHelper()
	{
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
			TI_ASSERT(Mesh.Segments[ESSI_POSITION].Data != nullptr);

			// init header
			THeaderMesh MeshHeader;
			MeshHeader.StrId_Name = AddStringToList(OutStrings, Mesh.Name);
			MeshHeader.VertexFormat = 0;
			for (int32 s = 0; s < ESSI_TOTAL; ++s)
			{
				if (Mesh.Segments[s].Data != nullptr)
				{
					MeshHeader.VertexFormat |= (1 << s);
				}
			}
			MeshHeader.VertexCount = Mesh.NumVertices;
			MeshHeader.PrimitiveCount = Mesh.NumTriangles;
			MeshHeader.IndexType = Mesh.NumVertices > 65535 ? EIT_32BIT : EIT_16BIT;
			MeshHeader.Flag = 0;
			MeshHeader.StrMaterialInstance = AddStringToList(OutStrings, Mesh.LinkedMaterialInstance);
			vector3df FirstPosition(Mesh.Segments[ESSI_POSITION].Data[0], Mesh.Segments[ESSI_POSITION].Data[1], Mesh.Segments[ESSI_POSITION].Data[2]);
			MeshHeader.BBox.reset(FirstPosition);

			// fill data
			// - vertices
			float* DataPos = Mesh.Segments[ESSI_POSITION].Data != nullptr ? Mesh.Segments[ESSI_POSITION].Data : nullptr;
			float* DataNormal = Mesh.Segments[ESSI_NORMAL].Data != nullptr ? Mesh.Segments[ESSI_NORMAL].Data : nullptr;
			float* DataColor = Mesh.Segments[ESSI_COLOR].Data != nullptr ? Mesh.Segments[ESSI_COLOR].Data : nullptr;
			float* DataUv0 = Mesh.Segments[ESSI_TEXCOORD0].Data != nullptr ? Mesh.Segments[ESSI_TEXCOORD0].Data : nullptr;
			float* DataUv1 = Mesh.Segments[ESSI_TEXCOORD1].Data != nullptr ? Mesh.Segments[ESSI_TEXCOORD1].Data : nullptr;
			float* DataTangent = Mesh.Segments[ESSI_TANGENT].Data != nullptr ? Mesh.Segments[ESSI_TANGENT].Data : nullptr;
			float* DataBI = Mesh.Segments[ESSI_BLENDINDEX].Data != nullptr ? Mesh.Segments[ESSI_BLENDINDEX].Data : nullptr;
			float* DataBW = Mesh.Segments[ESSI_BLENDWEIGHT].Data != nullptr ? Mesh.Segments[ESSI_BLENDWEIGHT].Data : nullptr;

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
					DataPos += Mesh.Segments[ESSI_POSITION].StrideInFloat;
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
					DataNormal += Mesh.Segments[ESSI_NORMAL].StrideInFloat;
				}
				if (DataColor != nullptr)
				{
					uint8 CData[4];
					CData[0] = FloatToColor(DataColor[0]);
					CData[1] = FloatToColor(DataColor[1]);
					CData[2] = FloatToColor(DataColor[2]);
					CData[3] = FloatToColor(DataColor[3]);

					TI_ASSERT(sizeof(CData) == TMeshBuffer::SemanticSize[ESSI_COLOR]);
					DataStream.Put(CData, sizeof(CData));
					DataColor += Mesh.Segments[ESSI_COLOR].StrideInFloat;
				}
				if (DataUv0 != nullptr)
				{
					TI_ASSERT(sizeof(float16) * 2 == TMeshBuffer::SemanticSize[ESSI_TEXCOORD0]);
					float16 UvHalf[2];
					UvHalf[0] = DataUv0[0];
					UvHalf[1] = DataUv0[1];
					DataStream.Put(UvHalf, sizeof(float16) * 2);
					DataUv0 += Mesh.Segments[ESSI_TEXCOORD0].StrideInFloat;
				}
				if (DataUv1 != nullptr)
				{
					TI_ASSERT(sizeof(float16) * 2 == TMeshBuffer::SemanticSize[ESSI_TEXCOORD1]);
					float16 UvHalf[2];
					UvHalf[0] = DataUv1[0];
					UvHalf[1] = DataUv1[1];
					DataStream.Put(UvHalf, sizeof(float16) * 2);
					DataUv1 += Mesh.Segments[ESSI_TEXCOORD1].StrideInFloat;
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
					DataTangent += Mesh.Segments[ESSI_TANGENT].StrideInFloat;
				}
				if (DataBI != nullptr)
				{
					TI_ASSERT(sizeof(uint8) * 4 == TMeshBuffer::SemanticSize[ESSI_BLENDINDEX]);
					uint8 BIData[4];
					BIData[0] = (uint8)DataBI[0];
					BIData[1] = (uint8)DataBI[1];
					BIData[2] = (uint8)DataBI[2];
					BIData[3] = (uint8)DataBI[3];
					DataStream.Put(BIData, sizeof(uint8) * 4);
					DataBI += Mesh.Segments[ESSI_BLENDINDEX].StrideInFloat;
				}
				if (DataBW != nullptr)
				{
					uint8 BWData[4];
					BWData[0] = FloatToColor(DataBW[0]);
					BWData[1] = FloatToColor(DataBW[1]);
					BWData[2] = FloatToColor(DataBW[2]);
					BWData[3] = FloatToColor(DataBW[3]);

					TI_ASSERT((int32)sizeof(BWData) == TMeshBuffer::SemanticSize[ESSI_BLENDWEIGHT]);
					DataStream.Put(BWData, sizeof(BWData));
					DataBW += Mesh.Segments[ESSI_BLENDWEIGHT].StrideInFloat;
				}
			}
			const int32 VertexEnd = DataStream.GetLength();

			// 8 bytes align
			TI_ASSERT((VertexEnd - VertexStart) % 4 == 0);
            if ((VertexEnd - VertexStart) % 4 != 0)
            {
				_LOG(Error, "Not aligned vertices.\n");
            }
			FillZero4(DataStream);

			// - Indices
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
			FillZero4(DataStream);

			// Fill header
			HeaderStream.Put(&MeshHeader, sizeof(THeaderMesh));
			FillZero4(HeaderStream);
		}
		ChunkHeader.ChunkSize = HeaderStream.GetLength() + DataStream.GetLength();

		OutStream.Put(&ChunkHeader, sizeof(TResfileChunkHeader));
		FillZero4(OutStream);
		OutStream.Put(HeaderStream.GetBuffer(), HeaderStream.GetLength());
		OutStream.Put(DataStream.GetBuffer(), DataStream.GetLength());
	}

	bool TResMeshHelper::LoadMeshFile(TJSON& Doc, TStream& OutStream, TVector<TString>& OutStrings)
	{
		TResMeshHelper ResMesh;

		TString Name = Doc["name"].GetString();
		//int32 Version = Doc["version"].GetInt();

		//int32 VCount = Doc["vertex_count_total"].GetInt();
		//int32 ICount = Doc["index_count_total"].GetInt();
		//int32 UVCount = Doc["texcoord_count"].GetInt();

		TJSONNode Sections = Doc["sections"];

		TVector<TString> SFormat;

		ResMesh.AllocateMeshes(Sections.Size());
		for (int32 i = 0; i < Sections.Size(); ++i)
		{
			TJSONNode JSection = Sections[i];
			TJSONNode JSectionName = JSection["name"];
			int32 VertexCount = JSection["vertex_count"].GetInt();
			TJSONNode JVertices = JSection["vertices"];
			TJSONNode JIndices = JSection["indices"];
			TJSONNode JVsFormat = JSection["vs_format"];

			SFormat.clear();

			ConvertJArrayToArray(JVsFormat, SFormat);

			TString SectionName = JSectionName.GetString();

			int32 VsFormat = 0;
			int32 ElementsStride = 0;
			for (const auto& S : SFormat)
			{
				E_VERTEX_STREAM_SEGMENT Segment = GetVertexSegment(S);
				VsFormat |= Segment;
				ElementsStride += GetSegmentElements(Segment);
			}
			TI_ASSERT(VertexCount * ElementsStride == JVertices.Size());

			const int32 BytesStride = ElementsStride * sizeof(float);
			TResMeshDefine& Mesh = ResMesh.GetMesh(i);
			Mesh.Name = SectionName;
			Mesh.NumVertices = VertexCount;
			Mesh.NumTriangles = (int32)JIndices.Size() / 3;
			ConvertJArrayToArray(JVertices, Mesh.Vertices);
			ConvertJArrayToArray(JIndices, Mesh.Indices);

			int32 ElementOffset = 0;
			{
				TI_ASSERT((VsFormat & EVSSEG_POSITION) != 0);
				Mesh.AddSegment(ESSI_POSITION, (float*)&Mesh.Vertices[ElementOffset], BytesStride);
				ElementOffset += 3;
			}
			if ((VsFormat & EVSSEG_NORMAL) != 0)
			{
				Mesh.AddSegment(ESSI_NORMAL, (float*)&Mesh.Vertices[ElementOffset], BytesStride);
				ElementOffset += 3;
			}
			if ((VsFormat & EVSSEG_COLOR) != 0)
			{
				Mesh.AddSegment(ESSI_COLOR, (float*)&Mesh.Vertices[ElementOffset], BytesStride);
				ElementOffset += 4;
			}
			if ((VsFormat & EVSSEG_TEXCOORD0) != 0)
			{
				Mesh.AddSegment(ESSI_TEXCOORD0, (float*)&Mesh.Vertices[ElementOffset], BytesStride);
				ElementOffset += 2;
			}
			if ((VsFormat & EVSSEG_TEXCOORD1) != 0)
			{
				Mesh.AddSegment(ESSI_TEXCOORD1, (float*)&Mesh.Vertices[ElementOffset], BytesStride);
				ElementOffset += 2;
			}
			if ((VsFormat & EVSSEG_TANGENT) != 0)
			{
				Mesh.AddSegment(ESSI_TANGENT, (float*)&Mesh.Vertices[ElementOffset], BytesStride);
				ElementOffset += 3;
			}
			if ((VsFormat & EVSSEG_BLENDINDEX) != 0)
			{
				Mesh.AddSegment(ESSI_BLENDINDEX, (float*)&Mesh.Vertices[ElementOffset], BytesStride);
				ElementOffset += 4;
			}
			if ((VsFormat & EVSSEG_BLENDWEIGHT) != 0)
			{
				Mesh.AddSegment(ESSI_BLENDWEIGHT, (float*)&Mesh.Vertices[ElementOffset], BytesStride);
				ElementOffset += 4;
			}
			Mesh.SetFaces(&Mesh.Indices[0], (int32)Mesh.Indices.size());

			TJSONNode JMaterial = JSection["material"];
			if (!JMaterial.IsNull())
			{
				TString MaterialName = JMaterial.GetString();
				Mesh.SetMaterial(MaterialName);
			}
		}

		ResMesh.OutputMesh(OutStream, OutStrings);
		return true;
	}
}
