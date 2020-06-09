/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "ResHelper.h"
#include "ResMeshHelper.h"
#include "TMeshBufferSemantic.h"
#include "ResMeshCluster.h"
#include "ResMultiThreadTask.h"

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
		ChunkHeader.ElementCount = 1;

		TStream HeaderStream, DataStream(1024 * 8);
		// Mesh data
		for (int32 m = 0; m < ChunkHeader.ElementCount; ++m)
		{
			TI_ASSERT(Mesh.Segments[ESSI_POSITION].Data != nullptr);

			int32 IndexType = Mesh.NumVertices > 65535 ? EIT_32BIT : EIT_16BIT;
			if (TResSettings::GlobalSettings.Force32BitIndex)
			{
				IndexType = EIT_32BIT;
			}

			// init header
			THeaderMesh MeshHeader;
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
			MeshHeader.IndexType = IndexType;
			MeshHeader.Sections = (int32)Mesh.Sections.size();
			TI_ASSERT(MeshHeader.Sections > 0);
			MeshHeader.Flag = 0;
			vector3df FirstPosition(Mesh.Segments[ESSI_POSITION].Data[0], Mesh.Segments[ESSI_POSITION].Data[1], Mesh.Segments[ESSI_POSITION].Data[2]);
			MeshHeader.BBox.reset(FirstPosition);

			TVector<THeaderMeshSection> MeshSections;
			MeshSections.resize(MeshHeader.Sections);
			for (int32 s = 0 ; s < MeshHeader.Sections ; ++ s)
			{
				MeshSections[s].StrId_Name = AddStringToList(OutStrings, Mesh.Sections[s].Name);
				MeshSections[s].StrMaterialInstance = AddStringToList(OutStrings, Mesh.Sections[s].LinkedMaterialInstance);
				MeshSections[s].IndexStart = Mesh.Sections[s].IndexStart;
				MeshSections[s].Triangles = Mesh.Sections[s].Triangles;
			}

			if (TResSettings::GlobalSettings.MeshClusterSize > 0)
			{
				// Remember all clusters
				MeshHeader.ClusterSize = TResSettings::GlobalSettings.MeshClusterSize;
				int32 TotalClusters = 0;
				for (int32 s = 0 ; s < MeshHeader.Sections ; ++ s)
				{
					TotalClusters += (int32)Mesh.Sections[s].ClusterIndices.size();
				}
				TI_ASSERT(TotalClusters < 65000);
				MeshHeader.Clusters = (uint16)TotalClusters;
				MeshHeader.PrimitiveCount = MeshHeader.ClusterSize * MeshHeader.Clusters;

				// Correct section index_start and triangles
				int32 IndexOffset = 0;
				for (int32 s = 0; s < MeshHeader.Sections; ++s)
				{
					MeshSections[s].IndexStart = IndexOffset;
					MeshSections[s].Triangles = (int32)(Mesh.Sections[s].ClusterIndices.size() * MeshHeader.ClusterSize);
					for (const auto& CI : Mesh.Sections[s].ClusterIndices)
					{
						TI_ASSERT(CI.size() == MeshHeader.ClusterSize * 3);
					}
					IndexOffset += MeshSections[s].Triangles * 3;
				}
			}

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

			if (TResSettings::GlobalSettings.MeshClusterSize > 0)
			{
				if (MeshHeader.IndexType == EIT_16BIT)
				{
					for (int32 s = 0; s < MeshHeader.Sections; ++s)
					{
						for (auto CI : Mesh.Sections[s].ClusterIndices)
						{
							for (auto I : CI)
							{
								uint16 Index = (uint16)I;
								DataStream.Put(&Index, sizeof(uint16));
							}
						}
					}
				}
				else
				{
					for (int32 s = 0; s < MeshHeader.Sections; ++s)
					{
						for (auto CI : Mesh.Sections[s].ClusterIndices)
						{
							for (auto I : CI)
							{
								uint32 Index = (uint32)I;
								DataStream.Put(&Index, sizeof(uint32));
							}
						}
					}
				}
			}
			else
			{
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
			}
			FillZero4(DataStream);

			// Export cluster meta data
			if (TResSettings::GlobalSettings.MeshClusterSize > 0)
			{
				for (int32 s = 0; s < MeshHeader.Sections; ++s)
				{
					TI_ASSERT(Mesh.Sections[s].ClusterBBoxes.size() == Mesh.Sections[s].ClusterCones.size());

					const uint32 ClusterCount = (uint32)Mesh.Sections[s].ClusterBBoxes.size();
					for (uint32 c = 0; c < ClusterCount; ++c)
					{
						TMeshClusterDef Cluster;
						Cluster.BBox = Mesh.Sections[s].ClusterBBoxes[c];
						Cluster.Cone = Mesh.Sections[s].ClusterCones[c];

						DataStream.Put(&Cluster, sizeof(TMeshClusterDef));
					}
				}
			}

			// Fill header
			HeaderStream.Put(&MeshHeader, sizeof(THeaderMesh));
			FillZero4(HeaderStream);
			HeaderStream.Put(MeshSections.data(), (uint32)(sizeof(THeaderMeshSection) * MeshSections.size()));
			FillZero4(HeaderStream);
		}

		// Collision data
		{
			// Header
			THeaderCollisionSet HeaderCollision;
			HeaderCollision.NumSpheres = (uint32)Mesh.ColSpheres.size();
			HeaderCollision.NumBoxes = (uint32)Mesh.ColBoxes.size();
			HeaderCollision.NumCapsules = (uint32)Mesh.ColCapsules.size();
			HeaderCollision.NumConvexes = (uint32)Mesh.ColConvexes.size();
			HeaderCollision.SpheresSizeInBytes = sizeof(TCollisionSet::TSphere) * (uint32)Mesh.ColSpheres.size();
			HeaderCollision.BoxesSizeInBytes = sizeof(TCollisionSet::TBox) * (uint32)Mesh.ColBoxes.size();
			HeaderCollision.CapsulesSizeInBytes = sizeof(TCollisionSet::TCapsule) * (uint32)Mesh.ColCapsules.size();
			HeaderCollision.ConvexesSizeInBytes = sizeof(vector2du) * (uint32)Mesh.ColConvexes.size();
			HeaderStream.Put(&HeaderCollision, sizeof(THeaderCollisionSet));

			// Data
			DataStream.Put(Mesh.ColSpheres.data(), HeaderCollision.SpheresSizeInBytes);
			DataStream.Put(Mesh.ColBoxes.data(), HeaderCollision.BoxesSizeInBytes);
			DataStream.Put(Mesh.ColCapsules.data(), HeaderCollision.CapsulesSizeInBytes);
			TVector<vector2du> ConvexVertexIndexCount;
			ConvexVertexIndexCount.reserve(Mesh.ColConvexes.size());
			for (const auto& Convex : Mesh.ColConvexes)
			{
				vector2du VertexIndexCount;
				VertexIndexCount.X = (uint32)Convex.VertexData.size();
				VertexIndexCount.Y = (uint32)Convex.IndexData.size();
				ConvexVertexIndexCount.push_back(VertexIndexCount);
			}
			DataStream.Put(ConvexVertexIndexCount.data(), HeaderCollision.ConvexesSizeInBytes);

			// Convex vertex data and index data
			for (const auto& Convex : Mesh.ColConvexes)
			{
				DataStream.Put(Convex.VertexData.data(), sizeof(vector3df) * (uint32)Convex.VertexData.size());
				DataStream.Put(Convex.IndexData.data(), sizeof(uint16) * (uint32)Convex.IndexData.size());
				FillZero4(DataStream);
			}
		}

		ChunkHeader.ChunkSize = HeaderStream.GetLength() + DataStream.GetLength();

		OutStream.Put(&ChunkHeader, sizeof(TResfileChunkHeader));
		FillZero4(OutStream);
		OutStream.Put(HeaderStream.GetBuffer(), HeaderStream.GetLength());
		OutStream.Put(DataStream.GetBuffer(), DataStream.GetLength());
	}

	class TMeshClusterTask : public TResMTTask
	{
	public:
		TMeshClusterTask(TResMeshDefine * InMesh, const TString& InMeshName, int32 InSection, int32 InSectionIndexStart, int32 InTriangleCount)
			: MeshSection(InMesh)
			, MeshName(InMeshName)
			, Section(InSection)
			, SectionIndexStart(InSectionIndexStart)
			, TriangleCount(InTriangleCount)
		{
		}

		TResMeshDefine * MeshSection;
		TString MeshName;
		int32 Section;
		int32 SectionIndexStart;
		int32 TriangleCount;

		virtual void Exec() override
		{
			const float* Positions = MeshSection->Segments[ESSI_POSITION].Data;
			const float* Normals = MeshSection->Segments[ESSI_NORMAL].Data;
			const int32 StrideInFloat = MeshSection->Segments[ESSI_POSITION].StrideInFloat;
		
			// Find mesh vertex index start and end
			int32 MaxVertexIndex = 0;
			int32 MinVertexIndex = 999999999;
			for (int32 i = SectionIndexStart ; i < SectionIndexStart + TriangleCount * 3 ; ++ i)
			{
				int32 VertexIndex = MeshSection->Indices[i];
				if (VertexIndex > MaxVertexIndex)
					MaxVertexIndex = VertexIndex;
				if (VertexIndex < MinVertexIndex)
					MinVertexIndex = VertexIndex;
			}

			// Copy
			TVector<vector3df> PosArray, NormalArray;
			const int32 TotalVertices = MaxVertexIndex - MinVertexIndex + 1;
			PosArray.resize(TotalVertices);
			NormalArray.resize(TotalVertices);
			int32 DataOffset = MinVertexIndex * StrideInFloat;
			for (int32 v = 0 ; v < TotalVertices; ++ v)
			{
				vector3df P(Positions[DataOffset + 0], Positions[DataOffset + 1], Positions[DataOffset + 2]);
				PosArray[v] = P;
				vector3df N(Normals[DataOffset + 0], Normals[DataOffset + 1], Normals[DataOffset + 2]);
				NormalArray[v] = N;
				DataOffset += StrideInFloat;
			}

			TVector<vector3di> PrimArray;
			PrimArray.resize(TriangleCount);
			for (int32 f = 0; f < (int32)TriangleCount * 3; f += 3)
			{
				vector3di F(
					MeshSection->Indices[f + SectionIndexStart + 0] - MinVertexIndex, 
					MeshSection->Indices[f + SectionIndexStart + 1] - MinVertexIndex, 
					MeshSection->Indices[f + SectionIndexStart + 2] - MinVertexIndex
				);
				PrimArray[f / 3] = F;
			}

			//TResMeshCluster(
			//	const TVector<vector3df>& PosArray,
			//	const TVector<vector3df>& NormalArray,
			//	const TVector<vector3di>& PrimsArray,
			//	const TString& InMeshName,
			//	int32 InSection,
			//	int32 InMinVertexIndex
			//);
			TResMeshCluster MC(PosArray, NormalArray, PrimArray, MeshName, Section, MinVertexIndex);
			MC.GenerateCluster(TResSettings::GlobalSettings.MeshClusterSize);

			MeshSection->Sections[Section].ClusterIndices = MC.ClusterIndices;
			MeshSection->Sections[Section].ClusterBBoxes = MC.ClusterBBoxes;
			MeshSection->Sections[Section].ClusterCones = MC.ClusterCones;
		}
	};
	bool TResMeshHelper::LoadMeshFile(TJSON& Doc, TStream& OutStream, TVector<TString>& OutStrings)
	{
		TResMeshHelper ResMesh;

		TString Name = Doc["name"].GetString();
		//int32 Version = Doc["version"].GetInt();
		int32 TotalVertices = Doc["vertex_count_total"].GetInt();
		int32 TotalIndices = Doc["index_count_total"].GetInt();

		//int32 VCount = Doc["vertex_count_total"].GetInt();
		//int32 ICount = Doc["index_count_total"].GetInt();
		//int32 UVCount = Doc["texcoord_count"].GetInt();

		// Load mesh data
		{
			TJSONNode JData = Doc["data"];
			TJSONNode JVsFormat = JData["vs_format"];

			TVector<TString> SFormat;
			ConvertJArrayToArray(JVsFormat, SFormat);

			TJSONNode JVertices = JData["vertices"];
			TJSONNode JIndices = JData["indices"];
			int32 VsFormat = 0;
			int32 ElementsStride = 0;
			for (const auto& S : SFormat)
			{
				E_VERTEX_STREAM_SEGMENT Segment = GetVertexSegment(S);
				VsFormat |= Segment;
				ElementsStride += GetSegmentElements(Segment);
			}
			TI_ASSERT(TotalVertices * ElementsStride == JVertices.Size());

			const int32 BytesStride = ElementsStride * sizeof(float);
			TResMeshDefine& Mesh = ResMesh.GetMesh();
			Mesh.NumVertices = TotalVertices;
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
		}

		// Load mesh sections
		{
			TJSONNode JSections = Doc["sections"];

			ResMesh.Mesh.Sections.clear();
			ResMesh.Mesh.Sections.resize(JSections.Size());

			for (int32 i = 0; i < JSections.Size(); ++i)
			{
				TJSONNode JSection = JSections[i];
				TResMeshSection& Section = ResMesh.Mesh.Sections[i];

				TJSONNode JSectionName = JSection["name"];
				TJSONNode JMaterial = JSection["material"];
				TJSONNode JIndexStart= JSection["index_start"];
				TJSONNode JTriangles = JSection["triangles"];

				Section.Name = JSectionName.GetString();

				if (!JMaterial.IsNull())
				{
					TString MaterialName = JMaterial.GetString();
					Section.LinkedMaterialInstance = MaterialName;
				}

				Section.IndexStart = JIndexStart.GetInt();
				Section.Triangles = JTriangles.GetInt();
			}

			// Generate mesh cluster for this section
			if (TResSettings::GlobalSettings.MeshClusterSize > 0)
			{
				TVector<TMeshClusterTask*> Tasks;
				TResMeshDefine& Mesh = ResMesh.GetMesh();

				for (int32 i = 0; i < JSections.Size(); ++i)
				{
					TJSONNode JSection = JSections[i];
					int32 IndexStart = JSection["index_start"].GetInt();
					int32 Triangles = JSection["triangles"].GetInt();

					TI_ASSERT(TResSettings::GlobalSettings.MeshClusterSize % 64 == 0);

					TMeshClusterTask * Task = ti_new TMeshClusterTask(&Mesh, Name, i, IndexStart, Triangles);
					TResMTTaskExecuter::Get()->AddTask(Task);
					Tasks.push_back(Task);
				}

				TResMTTaskExecuter::Get()->StartTasks();
				TResMTTaskExecuter::Get()->WaitUntilFinished();
			}
		}

		// Load collisions
		{
			TJSONNode Collisions = Doc["collisions"];

			TJSONNode ColSpheres = Collisions["sphere"];
			TJSONNode ColBoxes = Collisions["box"];
			TJSONNode ColCapsules = Collisions["capsule"];
			TJSONNode ColConvex = Collisions["convex"];

			ResMesh.Mesh.ColSpheres.resize(ColSpheres.Size());
			ResMesh.Mesh.ColBoxes.resize(ColBoxes.Size());
			ResMesh.Mesh.ColCapsules.resize(ColCapsules.Size());
			ResMesh.Mesh.ColConvexes.resize(ColConvex.Size());

			// Spheres
			for (int32 i = 0 ; i < ColSpheres.Size(); ++ i)
			{
				TJSONNode JSphere = ColSpheres[i];
				TJSONNode JCenter = JSphere["center"];
				TJSONNode JRadius = JSphere["radius"];

				ResMesh.Mesh.ColSpheres[i].Center = TJSONUtil::JsonArrayToVector3df(JCenter);
				ResMesh.Mesh.ColSpheres[i].Radius = JRadius.GetFloat();
			}

			// Boxes
			for (int32 i = 0 ; i < ColBoxes.Size(); ++ i)
			{
				TJSONNode JBox = ColBoxes[i];
				TJSONNode JCenter = JBox["center"];
				TJSONNode JRotation = JBox["quat"];
				TJSONNode JX = JBox["x"];
				TJSONNode JY = JBox["y"];
				TJSONNode JZ = JBox["z"];

				ResMesh.Mesh.ColBoxes[i].Center = TJSONUtil::JsonArrayToVector3df(JCenter);
				ResMesh.Mesh.ColBoxes[i].Rotation = TJSONUtil::JsonArrayToQuaternion(JRotation);
				ResMesh.Mesh.ColBoxes[i].Edge.X = JX.GetFloat();
				ResMesh.Mesh.ColBoxes[i].Edge.Y = JY.GetFloat();
				ResMesh.Mesh.ColBoxes[i].Edge.Z = JZ.GetFloat();
			}

			// Capsules
			for (int32 i = 0 ; i < ColCapsules.Size() ; ++ i)
			{
				TJSONNode JCapsule = ColCapsules[i];
				TJSONNode JCenter = JCapsule["center"];
				TJSONNode JRotation = JCapsule["quat"];
				TJSONNode JRadius = JCapsule["radius"];
				TJSONNode JLength = JCapsule["length"];

				ResMesh.Mesh.ColCapsules[i].Center = TJSONUtil::JsonArrayToVector3df(JCenter);
				ResMesh.Mesh.ColCapsules[i].Rotation = TJSONUtil::JsonArrayToQuaternion(JRotation);
				ResMesh.Mesh.ColCapsules[i].Radius = JRadius.GetFloat();
				ResMesh.Mesh.ColCapsules[i].Length = JLength.GetFloat();
			}

			// Convex
			for (int32 i = 0; i < ColConvex.Size(); ++i)
			{
				TJSONNode JConvex = ColConvex[i];
				TJSONNode JBBox = JConvex["bbox"];
				TJSONNode JCookedVB = JConvex["cooked_mesh_vertex_data"];
				TJSONNode JCookedIB = JConvex["cooked_mesh_index_data"];

				TJSONNode JTrans = JConvex["translation"];
				TJSONNode JRot = JConvex["rotation"];
				TJSONNode JScale = JConvex["scale"];

				vector3df Translation = TJSONUtil::JsonArrayToVector3df(JTrans);
				quaternion Rotation = TJSONUtil::JsonArrayToQuaternion(JRot);
				vector3df Scale = TJSONUtil::JsonArrayToVector3df(JScale);

				matrix4 Mat;
				Rotation.getMatrix(Mat);
				Mat.postScale(Scale);
				Mat.setTranslation(Translation);

				TI_ASSERT(JCookedVB.Size() % 3 == 0 && JCookedIB.Size() % 3 == 0);
				TVector<vector3df> VertexData;
				VertexData.resize(JCookedVB.Size() / 3);
				for (int32 e = 0 ; e < JCookedVB.Size(); e += 3)
				{
					float X = JCookedVB[e + 0].GetFloat();
					float Y = JCookedVB[e + 1].GetFloat();
					float Z = JCookedVB[e + 2].GetFloat();
					vector3df Position = vector3df(X, Y, Z);
					Mat.transformVect(Position);
					VertexData[e/3] = Position;
				}
				TI_ASSERT(JCookedIB.Size() < 65535);
				TVector<uint16> IndexData;
				IndexData.resize(JCookedIB.Size());
				for (int32 e = 0; e < JCookedIB.Size(); ++e)
				{
					uint16 Index = JCookedIB[e].GetInt();
					TI_ASSERT(Index < (uint32)VertexData.size());
					IndexData[e] = Index;
				}

				ResMesh.Mesh.ColConvexes[i].VertexData = VertexData;
				ResMesh.Mesh.ColConvexes[i].IndexData = IndexData;
			}
		}

		ResMesh.OutputMesh(OutStream, OutStrings);
		return true;
	}
}
