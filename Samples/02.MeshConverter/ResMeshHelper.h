/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	struct TResMeshSegment
	{
		float* Data;
		int32 StrideInFloat;
	};

	struct TResMeshFaces
	{
		int32* Data;
		int32 Count;
	};

	struct TResMeshDefine
	{
		TResMeshDefine(const TString& InName, int32 InNumVertices, int32 InNumTriangles)
			: Name(InName)
			, NumVertices(InNumVertices)
			, NumTriangles(InNumTriangles)
		{
			memset(Segments, 0, sizeof(TResMeshSegment*) * ESSI_TOTAL);

			Faces.Data = nullptr;
			Faces.Count = 0;
		}
		~TResMeshDefine()
		{
			for (int32 i = 0; i < ESSI_TOTAL; ++i)
			{
				SAFE_DELETE(Segments[i]);
			}
		}

		TString Name;
		int32 NumVertices;
		int32 NumTriangles;
		TResMeshSegment* Segments[ESSI_TOTAL];
		TResMeshFaces Faces;

		void AddSegment(E_MESH_STREAM_INDEX InStreamType, float* InData, int32 InStrideInByte);
		void SetFaces(int32* Indices, int32 Count);
	};

	class TResMeshHelper
	{
	public:
		TResMeshHelper();
		~TResMeshHelper();

		static bool LoadMeshFile(rapidjson::Document& Doc, TStream& OutStream, TVector<TString>& OutStrings);
		static bool LoadObjFile(const TString& Filename, TStream& OutStream, TVector<TString>& OutStrings);

		TResMeshDefine& AddMesh(const TString& Name, int32 NumVertices, int32 NumTriangles);
		void OutputMesh(TStream& OutStream, TVector<TString>& OutStrings);

	private:

	private:
		TVector<TResMeshDefine> Meshes;
	};
}