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

	struct TResMeshDefine
	{
		TResMeshDefine()
			: NumVertices(0)
			, NumTriangles(0)
			, LinkedMaterialInstance("MI_Unknown")
		{
		}
		TResMeshDefine(const TString& InName, int32 InVertices, int32 InTriangles)
			: Name(InName)
			, NumVertices(InVertices)
			, NumTriangles(InTriangles)
			, LinkedMaterialInstance("MI_Unknown")
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
			Name = Other.Name;
			NumVertices = Other.NumVertices;
			NumTriangles = Other.NumTriangles;
			LinkedMaterialInstance = Other.LinkedMaterialInstance;

			for (int32 i = 0; i < ESSI_TOTAL; ++i)
			{
				Segments[i] = Other.Segments[i];
			}
			Faces = Other.Faces;
			return *this;
		}

		TString Name;
		int32 NumVertices;
		int32 NumTriangles;
		TResMeshSegment Segments[ESSI_TOTAL];
		TResMeshFaces Faces;
		TString LinkedMaterialInstance;

		// Data
		TVector<float> Vertices;
		TVector<int32> Indices;

		TVector< TVector<uint32> > Clusters;

		void AddSegment(E_MESH_STREAM_INDEX InStreamType, float* InData, int32 InStrideInByte);
		void SetFaces(int32* Indices, int32 Count);
		void SetMaterial(const TString& MaterialName);
	};

	class TResMeshHelper
	{
	public:
		TResMeshHelper();
		~TResMeshHelper();

		static bool LoadMeshFile(TJSON& Doc, TStream& OutStream, TVector<TString>& OutStrings);
		static bool LoadObjFile(const TString& Filename, TStream& OutStream, TVector<TString>& OutStrings);

		void AllocateMeshes(int32 Size)
		{
			Meshes.resize(Size);
		}
		TResMeshDefine& GetMesh(int32 Index)
		{
			return Meshes[Index];
		}
		void OutputMesh(TStream& OutStream, TVector<TString>& OutStrings);

	private:

	private:
		TVector<TResMeshDefine> Meshes;

		TVector<TCollisionSet::TSphere> ColSpheres;
		TVector<TCollisionSet::TBox> ColBoxes;
		TVector<TCollisionSet::TCapsule> ColCapsules;
		TVector<TCollisionSet::TConvex> ColConvexes;

	};
}