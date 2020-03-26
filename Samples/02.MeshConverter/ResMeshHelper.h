/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	struct TResMeshSegment
	{
		float* Data;
		int32 StrideInFloat;
		TResMeshSegment()
			: Data(nullptr)
			, StrideInFloat(0)
		{}
	};

	struct TResMeshFaces
	{
		int32* Data;
		int32 Count;
		TResMeshFaces()
			: Data(nullptr)
			, Count(0)
		{}
	};

	struct TResMeshSection
	{
		TResMeshSection()
			: LinkedMaterialInstance("MI_Unknown")
			, IndexStart(0)
			, Triangles(0)
		{
		}

		void SetMaterial(const TString& MaterialName)
		{
			LinkedMaterialInstance = MaterialName;
		}

		TResMeshSection& operator = (const TResMeshSection& Other)
		{
			Name = Other.Name;
			LinkedMaterialInstance = Other.LinkedMaterialInstance;
			return *this;
		}

		TString Name;
		TString LinkedMaterialInstance;
		int32 IndexStart;
		int32 Triangles;

		TVector< TVector<uint32> > ClusterIndices;
		TVector< aabbox3df > ClusterBBoxes;
		TVector< vector4df > ClusterCones;
	};

	struct TResMeshDefine
	{
		TResMeshDefine()
			: NumVertices(0)
			, NumTriangles(0)
		{
		}
		TResMeshDefine(const TString& InName, int32 InVertices, int32 InTriangles)
			: NumVertices(InVertices)
			, NumTriangles(InTriangles)
		{
		}
		~TResMeshDefine()
		{
		}

		TResMeshDefine(const TResMeshDefine& Other)
		{
			*this = Other;
		}
		TResMeshDefine& operator = (const TResMeshDefine& Other)
		{
			NumVertices = Other.NumVertices;
			NumTriangles = Other.NumTriangles;
		
			for (int32 i = 0; i < ESSI_TOTAL; ++i)
			{
				Segments[i] = Other.Segments[i];
			}
			Faces = Other.Faces;
			return *this;
		}

		int32 NumVertices;
		int32 NumTriangles;
		TResMeshSegment Segments[ESSI_TOTAL];

		TResMeshFaces Faces;

		// Data
		TVector<float> Vertices;
		TVector<int32> Indices;

		void AddSegment(E_MESH_STREAM_INDEX InStreamType, float* InData, int32 InStrideInByte);
		void SetFaces(int32* Indices, int32 Count);

		TVector<TResMeshSection> Sections;

		TVector<TCollisionSet::TSphere> ColSpheres;
		TVector<TCollisionSet::TBox> ColBoxes;
		TVector<TCollisionSet::TCapsule> ColCapsules;
		TVector<TCollisionSet::TConvex> ColConvexes;
	};

	class TResMeshHelper
	{
	public:
		TResMeshHelper();
		~TResMeshHelper();

		static bool LoadMeshFile(TJSON& Doc, TStream& OutStream, TVector<TString>& OutStrings);

		TResMeshDefine& GetMesh()
		{
			return Mesh;
		}
		void OutputMesh(TStream& OutStream, TVector<TString>& OutStrings);

	private:

	private:
		TResMeshDefine Mesh;

	};
}