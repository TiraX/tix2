/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"

#include "ResHelper.h"
#include "ResMeshHelper.h"
#include <fstream>

namespace tix
{
	bool TResMeshHelper::LoadObjFile(const TString& Filename, TStream& OutStream, TVector<TString>& OutStrings)
	{
		ifstream objfile;
		objfile.open(Filename, ios::in);
		const TString PosStart = "v ";
		const TString NormalStart = "vn";
		const TString UVStart = "vt";
		const TString FaceStart = "f ";
		const TString GroupStart = "g ";

		TVector<vector3df> PosArray, NormalArray, UVArray;
		TVector<vector3di> FaceArray;
		TVector< TVector<vector3di> > Groups;
		TVector< TString > GroupNames;

		// load obj content
		if (objfile.is_open())
		{
			TString line;
			while (getline(objfile, line))
			{
				TString lineStart = line.substr(0, 2);
				if (lineStart == PosStart)
				{
					TVector<float> Position = ReadFloatArray(line.c_str() + 2);
					TI_ASSERT(Position.size() == 3);
					PosArray.push_back(vector3df(Position[0], Position[1], Position[2]));
				}
				else if (lineStart == NormalStart)
				{
					TVector<float> Normal = ReadFloatArray(line.c_str() + 2);
					TI_ASSERT(Normal.size() == 3);
					vector3df N(Normal[0], Normal[1], Normal[2]);
					N.normalize();
					NormalArray.push_back(N);
				}
				else if (lineStart == UVStart)
				{
					TVector<float> UV0 = ReadFloatArray(line.c_str() + 2);
					TI_ASSERT(UV0.size() == 3);
					UVArray.push_back(vector3df(UV0[0], UV0[1], UV0[2]));
				}
				else if (lineStart == FaceStart)
				{
					TVector<TString> TriangleStr = ReadStringArray(line.c_str() + 2);
					TI_ASSERT(TriangleStr.size() == 3);
					// turn indices revert, it seems different back face order with dx12
					for (int f = 2; f >= 0; --f)
					{
						TVector<int> face = ReadIntArray(TriangleStr[f].c_str(), '/');
						TI_ASSERT(face.size() == 3);
						FaceArray.push_back(vector3di(face[0], face[1], face[2]));
					}
				}
				else if (lineStart == GroupStart)
				{
					TString Name = line.c_str() + 2;
					Name = trim(Name);
					GroupNames.push_back(Name);

					if (FaceArray.size() > 0)
					{
						Groups.push_back(FaceArray);
						FaceArray.clear();
					}
				}
			}

			// Put the last group 
			if (FaceArray.size() > 0)
			{
				Groups.push_back(FaceArray);
				FaceArray.clear();
			}

			TI_ASSERT(GroupNames.size() <= Groups.size());
			for (int32 n = (int32)GroupNames.size(); n < (int32)Groups.size(); ++n)
			{
				GroupNames.push_back(TString("Unknown-name"));
			}

			// re-organize vertices
			TVector<vector3df> PosArrayNew, NormalArrayNew, UVArrayNew;
			TMap<vector3di, int32> IndexMap;
			TVector<int32> Indices;
			TResMeshHelper ResMesh;
			ResMesh.AllocateMeshes((int32)Groups.size());
			for (int32 g = 0 ; g < (int32)Groups.size() ; ++ g)
			{
				const TVector<vector3di>& group = Groups[g];
				for (const auto& face : group)
				{
					if (IndexMap.find(face) == IndexMap.end())
					{
						// Add a new vertex
						PosArrayNew.push_back(PosArray[face.X - 1]);
						NormalArrayNew.push_back(NormalArray[face.Z - 1]);
						UVArrayNew.push_back(UVArray[face.Y - 1]);

						// Add a index
						int32 Index = (int32)IndexMap.size();
						IndexMap[face] = Index;
						Indices.push_back(Index);
					}
					else
					{
						// Use old index
						Indices.push_back(IndexMap[face]);
					}
				}
				TI_ASSERT(PosArrayNew.size() == IndexMap.size());

				// assign segment
				TResMeshDefine& Mesh = ResMesh.GetMesh(g);
				Mesh.Name = GroupNames[g];
				Mesh.NumVertices = (int32)PosArrayNew.size();
				Mesh.NumTriangles = (int32)Indices.size() / 3;
				Mesh.AddSegment(ESSI_POSITION, (float*)&PosArrayNew[0], sizeof(vector3df));
				Mesh.AddSegment(ESSI_NORMAL, (float*)&NormalArrayNew[0], sizeof(vector3df));
				Mesh.AddSegment(ESSI_TEXCOORD0, (float*)&UVArrayNew[0], sizeof(vector3df));
				Mesh.SetFaces(&Indices[0], (int32)Indices.size());
			}
			ResMesh.OutputMesh(OutStream, OutStrings);

			return true;
		}
		return false;
	}
}
