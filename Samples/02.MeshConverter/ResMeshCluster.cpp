/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "ResMeshCluster.h"
#include "TSmallestEncloseSphere.h"

static float VolumeCellSize = 1.f;
static const bool EnableVerbose = false;

TResMeshCluster::TResMeshCluster()
	: Section(0)
{
}

TResMeshCluster::TResMeshCluster(const TVector<vector3df>& PosArray, const TVector<vector3di>& PrimsArray, const TString& InMeshName, int32 InSection)
	: MeshName(InMeshName)
	, Section(InSection)
{
	P = PosArray;
	Prims = PrimsArray;

	// Calc bbox
	TI_ASSERT(P.size() > 0);
	BBox.reset(P[0]);
	for (uint32 i = 1 ; i < (uint32)P.size() ; ++ i)
	{
		BBox.addInternalPoint(P[i]);
	}
}

TResMeshCluster::~TResMeshCluster()
{
}

TSphere TResMeshCluster::GetBoundingSphere(const TVector<vector3df>& Points)
{
	TVector<vector3df64> Points64;
	Points64.resize(Points.size());
	for (uint32 p = 0; p < (uint32)Points.size(); ++p)
	{
		Points64[p].X = Points[p].X;
		Points64[p].Y = Points[p].Y;
		Points64[p].Z = Points[p].Z;
	}
	TSmallestEncloseSphere<double> SmallSphere(Points64);
	TSphere Sphere;
	Sphere.Radius = (float)SmallSphere.GetRadius();
	Sphere.Center.X = (float)SmallSphere.GetCenter().X;
	Sphere.Center.Y = (float)SmallSphere.GetCenter().Y;
	Sphere.Center.Z = (float)SmallSphere.GetCenter().Z;
	return Sphere;
}

static int32 _debug_index = 0;
void TResMeshCluster::GenerateCluster(uint32 ClusterTriangles)
{
	static const bool bExportDebugObjFile = true;
	if (bExportDebugObjFile)
	{
		char debug_name[64];
		sprintf(debug_name, "_origin_%d.obj", _debug_index);
		SaveObjFile(debug_name);
	}

	SortPrimitives();
	CalcPrimNormals();
	ScatterToVolume();
	MakeClusters(ClusterTriangles);
	MergeSmallClusters(ClusterTriangles);
	CalcMetaInfos();

	if (bExportDebugObjFile)
	{
		char debug_name[64];
		sprintf(debug_name, "_result_%d.obj", _debug_index);
		SaveClusterObjFile(debug_name);
		++_debug_index;
	}
}

void TResMeshCluster::SortPrimitives()
{

}

void TResMeshCluster::CalcPrimNormals()
{
	PrimsN.reserve(Prims.size());
	for (const auto& Prim : Prims)
	{
		const vector3df& P0 = P[Prim.X];
		const vector3df& P1 = P[Prim.Y];
		const vector3df& P2 = P[Prim.Z];

		vector3df P10 = P1 - P0;
		vector3df P20 = P2 - P0;
		vector3df N = P10.crossProduct(P20);
		N.normalize();
		PrimsN.push_back(N);
	}
}

inline vector3df vec_floor(const vector3df& vec)
{
	vector3df r;
	r.X = floor(vec.X);
	r.Y = floor(vec.Y);
	r.Z = floor(vec.Z);
	return r;
}
inline vector3df vec_ceil(const vector3df& vec)
{
	vector3df r;
	r.X = ceil(vec.X);
	r.Y = ceil(vec.Y);
	r.Z = ceil(vec.Z);
	return r;
}
inline aabbox3df GetBoundingVolume(const aabbox3df& BBox)
{
	aabbox3df VolumeBox;
	VolumeBox.MinEdge = vec_floor(BBox.MinEdge / VolumeCellSize) * VolumeCellSize;
	VolumeBox.MaxEdge = vec_ceil(BBox.MaxEdge / VolumeCellSize) * VolumeCellSize;
	return VolumeBox;
}
inline vector3di GetVolumeCellCount(const aabbox3df& VolumeBox)
{
	vector3df VolumeSize = VolumeBox.getExtent();
	vector3di VolumeCellCount;
	for (int32 i = 0; i < 3; ++i)
	{
		VolumeCellCount[i] = ti_round(VolumeSize[i] / VolumeCellSize);
	}
	return VolumeCellCount;
}
inline uint32 GetCellIndex(const vector3di& CellPosition, const vector3di& VolumeCellCount)
{
	const uint32 PageSize = VolumeCellCount.X * VolumeCellCount.Y;
	return CellPosition.Z * PageSize + CellPosition.Y * VolumeCellCount.X + CellPosition.X;
}
inline vector3di GetCellPosition(uint32 CellIndex, const vector3di& VolumeCellCount)
{
	const uint32 PageSize = VolumeCellCount.X * VolumeCellCount.Y;

	vector3di Result;
	Result.Z = CellIndex / PageSize;
	Result.Y = (CellIndex % PageSize) / VolumeCellCount.X;
	Result.X = (CellIndex % PageSize) % VolumeCellCount.X;

	return Result;
}
void TResMeshCluster::ScatterToVolume()
{
	const uint32 PointCount = (uint32)P.size();
	const uint32 PrimCount = (uint32)Prims.size();

	// Determine VolumeCellSize
	MeshVolume = GetBoundingVolume(BBox);
	MeshVolumeCellCount = GetVolumeCellCount(MeshVolume);
	static const int32 TargetCellCount = 6;
	while (MeshVolumeCellCount.X * MeshVolumeCellCount.Y * MeshVolumeCellCount.Z > TargetCellCount * TargetCellCount * TargetCellCount)
	{
		VolumeCellSize += 1.f;
		MeshVolume = GetBoundingVolume(BBox);
		MeshVolumeCellCount = GetVolumeCellCount(MeshVolume);
	}
	VolumeCells.resize(MeshVolumeCellCount.X * MeshVolumeCellCount.Y * MeshVolumeCellCount.Z);
	PrimVolumePositions.resize(PrimCount);

	if (TResSettings::GlobalSettings.ClusterVerbose)
	{
		_LOG(Log, "  [%s(%d)] Prims [%d]. Volumes [%d, %d, %d] with size : %f.\n", MeshName.c_str(), Section, PrimCount, MeshVolumeCellCount.X, MeshVolumeCellCount.Y, MeshVolumeCellCount.Z, VolumeCellSize);
	}

	// Scatter every triangle to volume cell
	for (uint32 PrimIndex = 0; PrimIndex < PrimCount; ++PrimIndex)
	{
		const vector3di& Prim = Prims[PrimIndex];

		triangle3df Triangle(P[Prim.X], P[Prim.Y], P[Prim.Z]);

		aabbox3df Box = Triangle.getBoundingBox();
		aabbox3df VolumeBox = GetBoundingVolume(Box);

		vector3di VolumeCellCount = GetVolumeCellCount(VolumeBox);
		vector3di VolumeCellStart = GetVolumeCellCount(aabbox3df(MeshVolume.MinEdge, VolumeBox.MinEdge));

		for (int32 z = 0; z < VolumeCellCount.Z; ++z)
		{
			for (int32 y = 0; y < VolumeCellCount.Y; ++y)
			{
				for (int32 x = 0; x < VolumeCellCount.X; ++x)
				{
					aabbox3df Cell;
					Cell.MinEdge = VolumeBox.MinEdge + vector3df(VolumeCellSize * x, VolumeCellSize * y, VolumeCellSize * z);
					Cell.MaxEdge = Cell.MinEdge + vector3df(VolumeCellSize, VolumeCellSize, VolumeCellSize);

					if (Triangle.isIntersectWithBox(Cell))
					{
						// Mark prim in this cell
						vector3di PrimVolumePosition = vector3di(VolumeCellStart.X + x, VolumeCellStart.Y + y, VolumeCellStart.Z + z);
						uint32 CellIndex = GetCellIndex(PrimVolumePosition, MeshVolumeCellCount);
						VolumeCells[CellIndex].push_back(PrimIndex);

						// Mark cell position for this prim
						PrimVolumePositions[PrimIndex].push_back(CellIndex);
					}
				}
			}
		}
	}
}

inline bool IsNormalValid(const vector3df& InN, const vector3df& ClusterN)
{
	// angle between InN and ClusterN should NOT bigger than 60 degree
	static const float V = cos(DEG_TO_RAD(60));
	return InN.dotProduct(ClusterN) > V;
}

void TResMeshCluster::MakeClusters(uint32 ClusterSize)
{
	// Go through each triangles
	const uint32 PointCount = (uint32)P.size();
	const uint32 PrimCount = (uint32)Prims.size();

	TVector<uint32> PrimsClusterId;
	PrimsClusterId.resize(PrimCount);
	memset(PrimsClusterId.data(), 0, PrimCount * sizeof(uint32));

	Clusters.empty();
	Clusters.reserve(PrimCount / ClusterSize + 2);

	uint32 ClusterId = 0;
	Clusters.push_back(TVector<uint32>());	// Cluster 0 always an empty cluster
	for (uint32 PrimIndex = 0; PrimIndex < PrimCount; ++PrimIndex)
	{
		if (PrimsClusterId[PrimIndex] != 0)
		{
			// Already in cluster, next
			continue;
		}

		++ClusterId;

		TVector<uint32> Cluster;
		Cluster.reserve(ClusterSize);

		// Mark this prim in ClusterId
		PrimsClusterId[PrimIndex] = ClusterId;
		Cluster.push_back(PrimIndex);

		// Calculate bounding sphere
		TVector<vector3df> ClusterPoints;
		TMap<uint32, uint32> PointsInCluster;
		THMap<vector3df, uint32> UniquePosMap;
		ClusterPoints.reserve(ClusterSize * 3);
		{
			const vector3di& Prim = Prims[PrimIndex];
			ClusterPoints.push_back(P[Prim.X]);
			ClusterPoints.push_back(P[Prim.Y]);
			ClusterPoints.push_back(P[Prim.Z]);
			PointsInCluster[Prim.X] = 1;
			PointsInCluster[Prim.Y] = 1;
			PointsInCluster[Prim.Z] = 1;
			UniquePosMap[P[Prim.X]] = 1;
			UniquePosMap[P[Prim.Y]] = 1;
			UniquePosMap[P[Prim.Z]] = 1;
		}

		// Cluster average normal
		TVector<vector3df> ClusterPrimNormals;
		ClusterPrimNormals.reserve(ClusterSize);
		vector3df ClusterN = PrimsN[PrimIndex];
		ClusterPrimNormals.push_back(ClusterN);

		// Init Bounding sphere
		TSphere BSphere = GetBoundingSphere(ClusterPoints);

		// Start search from this prim
		for (uint32 i = 1 ; i < ClusterSize; ++ i)
		{
			// Get neighbor triangles
			TVector<uint32> NeighbourPrims;
			GetNeighbourPrims(Cluster, NeighbourPrims, PrimsClusterId);
			if (EnableVerbose)
			{
				_LOG(Log, " %d analysis %d neighbours with points %d.\n", i, NeighbourPrims.size(), ClusterPoints.size());
			}
			
			// Find the nearest prim,
			// 1, if any prim in BSphere, select it directly
			int32 PrimFound = -1;
			for (const auto& NeighbourPrimIndex : NeighbourPrims)
			{
				const vector3di& NeighbourPrim = Prims[NeighbourPrimIndex];

				if (!IsNormalValid(PrimsN[NeighbourPrimIndex], ClusterN))
				{
					continue;
				}

				bool InsideBSphere = true;
				for (uint32 ii = 0; ii < 3; ++ii)
				{
					if (!BSphere.IsPointInsideSphere(P[NeighbourPrim[ii]]))
					{
						InsideBSphere = false;
						break;
					}
				}
				if (InsideBSphere)
				{
					PrimFound = NeighbourPrimIndex;
					break;
				}
			}

			// 2, No prims in BSphere, find the new Smallest BSphere
			if (PrimFound < 0)
			{
				float SmallestBSphereRadius = FLT_MAX;
				for (const auto& NeighbourPrimIndex : NeighbourPrims)
				{
					const vector3di& NeighbourPrim = Prims[NeighbourPrimIndex];

					if (!IsNormalValid(PrimsN[NeighbourPrimIndex], ClusterN))
					{
						continue;
					}

					uint32 PointsAdded = 0;
					for (uint32 ii = 0; ii < 3; ++ii)
					{
						uint32 PIndex = NeighbourPrim[ii];
						if (PointsInCluster.find(PIndex) == PointsInCluster.end())
						{
							ClusterPoints.push_back(P[PIndex]);
							++PointsAdded;
						}
					}
					TSphere NewBSphere = GetBoundingSphere(ClusterPoints);
					for (uint32 ii = 0 ; ii < PointsAdded ; ++ii)
					{
						ClusterPoints.pop_back();
					}
					if (NewBSphere.Radius < SmallestBSphereRadius)
					{
						PrimFound = NeighbourPrimIndex;
						SmallestBSphereRadius = NewBSphere.Radius;
						BSphere = NewBSphere;
					}
				}
			}

			if (PrimFound < 0)
			{
				//TI_ASSERT(NeighbourPrims.size() == 0);
				break;
			}

			// Add PrimFound to Cluster
			PrimsClusterId[PrimFound] = ClusterId;
			Cluster.push_back(PrimFound);
			{
				const vector3di& Prim = Prims[PrimFound];
				for (int32 ii = 0 ; ii < 3 ; ++ ii)
				{
					if (PointsInCluster.find(Prim[ii]) == PointsInCluster.end())
					{
						if (UniquePosMap.find(P[Prim[ii]]) == UniquePosMap.end())
						{
							ClusterPoints.push_back(P[Prim[ii]]);
							UniquePosMap[P[Prim[ii]]] = 1;
						}
						PointsInCluster[Prim[ii]] = 1;
					}
				}
			}
			// Update Cluster N
			{
				ClusterPrimNormals.push_back(PrimsN[PrimFound]);
				TSphere NBSphere = GetBoundingSphere(ClusterPrimNormals);
				ClusterN = NBSphere.Center;
				ClusterN.normalize();
			}
			// Update Bounding Sphere
			BSphere = GetBoundingSphere(ClusterPoints);
		}

		TI_ASSERT(Clusters.size() == ClusterId);
		Clusters.push_back(Cluster);

		if (TResSettings::GlobalSettings.ClusterVerbose)
		{
			_LOG(Log, "  [%s(%d)] Cluster %d generated with %d prims.\n", MeshName.c_str(), Section, ClusterId, Cluster.size());
		}
	}
}

inline void GetNeighbourCells(uint32 CellIndex, const vector3di& MeshVolumeCellCount, TVector<uint32>& OutNeighbourCells)
{
	OutNeighbourCells.reserve(9);
	vector3di CellPosition = GetCellPosition(CellIndex, MeshVolumeCellCount);

	for (int32 z = CellPosition.Z - 1 ; z <= CellPosition.Z + 1 ; ++ z)
	{
		if (z >= 0 && z < MeshVolumeCellCount.Z)
		{
			for (int32 y = CellPosition.Y - 1 ; y <= CellPosition.Y + 1 ; ++ y)
			{
				if (y >= 0 && y < MeshVolumeCellCount.Y)
				{
					for (int32 x = CellPosition.X - 1 ; x <= CellPosition.X + 1 ; ++ x)
					{
						if (x >= 0 && x < MeshVolumeCellCount.X)
						{
							uint32 CIndex = GetCellIndex(vector3di(x, y, z), MeshVolumeCellCount);
							OutNeighbourCells.push_back(CIndex);
						}
					}
				}
			}
		}
	}
}

void TResMeshCluster::GetNeighbourPrims(const TVector<uint32>& InPrims, TVector<uint32>& OutNeighbourPrims, const TVector<uint32>& InPrimsClusterId)
{
	const uint32 MIN_PRIMS_FOUND = 12;
	TMap<uint32, uint32> CellSearched;
	TMap<uint32, uint32> PrimsAdded;
	for (uint32 PrimIndex : InPrims)
	{
		const TVector<uint32>& CellPositions = PrimVolumePositions[PrimIndex];

		for (uint32 CellIndex : CellPositions)
		{
			if (CellSearched.find(CellIndex) == CellSearched.end())
			{
				// Mark cell as searched
				CellSearched[CellIndex] = 1;

				// Get triangles NOT in cluster in this cell
				const TVector<uint32>& Primitives = VolumeCells[CellIndex];
				for (uint32 CellPrimIndex : Primitives)
				{
					if (InPrimsClusterId[CellPrimIndex] == 0 && PrimsAdded.find(CellPrimIndex) == PrimsAdded.end())
					{
						OutNeighbourPrims.push_back(CellPrimIndex);
						PrimsAdded[CellPrimIndex] = 1;
					}
				}
			}
		}
	}
	if (PrimsAdded.size() > MIN_PRIMS_FOUND)
	{
		return;
	}

	for (uint32 PrimIndex : InPrims)
	{
		// Search neighbour cells
		const uint32 MAX_ITERATION = 5;
		uint32 Iteration = 0;
		TMap<uint32, uint32> LastSearchedCells;
		while (Iteration < MAX_ITERATION)
		{
			TMap<uint32, uint32> CellSearchedCopy = CellSearched;
			// Remove last searched cells
			for (const auto LastCell : LastSearchedCells)
			{
				CellSearchedCopy.erase(LastCell.first);
			}

			// iteration on cell
			for (const auto& CellItem : CellSearchedCopy)
			{
				uint32 CellIndex = CellItem.first;
				TVector<uint32> NeighbourCells;
				GetNeighbourCells(CellIndex, MeshVolumeCellCount, NeighbourCells);

				for (uint32 NeighbourCellIndex : NeighbourCells)
				{
					if (CellSearched.find(NeighbourCellIndex) == CellSearched.end())
					{
						// Mark cell as searched
						CellSearched[NeighbourCellIndex] = 1;

						// Get triangles NOT in cluster in this cell
						const TVector<uint32>& Primitives = VolumeCells[NeighbourCellIndex];
						for (uint32 CellPrimIndex : Primitives)
						{
							if (InPrimsClusterId[CellPrimIndex] == 0 && PrimsAdded.find(CellPrimIndex) == PrimsAdded.end())
							{
								OutNeighbourPrims.push_back(CellPrimIndex);
								PrimsAdded[CellPrimIndex] = 1;
							}
						}
					}
				}
			}
			if (PrimsAdded.size() > MIN_PRIMS_FOUND)
			{
				break;
			}

			LastSearchedCells = CellSearchedCopy;
			++Iteration;
		}
	}
}

void TResMeshCluster::MergeSmallClusters(uint32 ClusterTriangles)
{
	TVector<TVector<uint32>> FullClusters;
	TVector<TVector<uint32>> SmallClusters;
	for (TVector<TVector<uint32>>::iterator it = Clusters.begin() ; it != Clusters.end() ; ++ it)
	{
		if (it->size() < ClusterTriangles)
		{
			// Remove from Clusters, add to SmallClusters;
			SmallClusters.push_back(*it);
		}
		else
		{
			FullClusters.push_back(*it);
		}
	}
	TVector<TVector<uint32>> MergedClusters;
	TVector<uint32> Merged;
	Merged.reserve(ClusterTriangles);
	for (const auto& SC : SmallClusters)
	{
		const uint32 Tri = (uint32)SC.size();
		for (uint32 t = 0 ;t < Tri ; ++ t)
		{
			Merged.push_back(SC[t]);
			if (Merged.size() == ClusterTriangles)
			{
				MergedClusters.push_back(Merged);
				Merged.clear();
			}
		}
	}
	if (Merged.size() > 0)
	{
		uint32 LastPrim = Merged[Merged.size() - 1];

		TI_TODO("Fill with zero sized triangle (means index[0] == index[1] == index[2];");
		for (uint32 m = (uint32)Merged.size(); m < ClusterTriangles; ++m)
		{
			Merged.push_back(LastPrim);
		}
		MergedClusters.push_back(Merged);
	}
	for (const auto& MC : MergedClusters)
	{
		FullClusters.push_back(MC);
	}
	Clusters = FullClusters;


	// Copy indices
	ClusterIndices.clear();
	ClusterIndices.reserve(Clusters.size());
	for (const auto& C : Clusters)
	{
		TVector<uint32> I;
		I.reserve(C.size() * 3);
		for (uint32 PrimIndex : C)
		{
			const vector3di& Prim = Prims[PrimIndex];
			I.push_back(Prim.X);
			I.push_back(Prim.Y);
			I.push_back(Prim.Z);
		}
		ClusterIndices.push_back(I);
	}
}

void TResMeshCluster::CalcMetaInfos()
{
	// Calc cluster bbox and cone info
	ClusterBBoxes.reserve(Clusters.size());
	ClusterCones.reserve(Clusters.size());

	for (const auto& Cluster : Clusters)
	{
		const vector3df& P0 = P[Prims[Cluster[0]].X];
		aabbox3df CBBox(P0);

		for (auto CPrim : Cluster)
		{
			const vector3df& CP0 = P[Prims[CPrim].X];
			const vector3df& CP1 = P[Prims[CPrim].Y];
			const vector3df& CP2 = P[Prims[CPrim].Z];

			CBBox.addInternalPoint(CP0);
			CBBox.addInternalPoint(CP1);
			CBBox.addInternalPoint(CP2);
		}
		ClusterBBoxes.push_back(CBBox);
	}

	for (const auto& Cluster : Clusters)
	{
		TVector<vector3df> ClusterNormals;
		ClusterNormals.reserve(Clusters.size());
		for (auto CPrim : Cluster)
		{
			const vector3df& PrimN = PrimsN[CPrim];
			ClusterNormals.push_back(PrimN);
		}
		TSphere NBSphere = GetBoundingSphere(ClusterNormals);
		vector3df CNormal = NBSphere.Center;
		CNormal.normalize();
		float SinA = NBSphere.Radius;
		vector4df Cone;
		Cone.X = CNormal.X;
		Cone.Y = CNormal.Y;
		Cone.Z = CNormal.Z;
		Cone.W = -SinA;
		ClusterCones.push_back(Cone);
	}
}

bool TResMeshCluster::SaveObjFile(const TString& Filename)
{
	TStringStream SS;

	for (const auto& P : P)
	{
		SS << "v " << P.X << " " << P.Y << " " << P.Z << "\n";
	}
	SS << "\n";
	SS << "vt 0.0 0.0 0.0\n\n";
	SS << "vn 0 0 1\n\n";

	SS << "g origin01\n";
	SS << "s origins01\n\n";

	const uint32 NPrims = (uint32)Prims.size();
	for (uint32 p = 0; p < NPrims; ++p)
	{
		const vector3di& Prim = Prims[p];
		vector3di T = Prim + vector3di(1, 1, 1);

		SS << "f ";
		SS << T.X << "/1/1 ";
		SS << T.Y << "/1/1 ";
		SS << T.Z << "/1/1\n";
	}
	SS << "\n";

	// Write to file
	TFile objfile;
	if (objfile.Open(Filename, EFA_CREATEWRITE))
	{
		objfile.Write(SS.str().c_str(), (int32)SS.str().size());
		objfile.Close();
	}

	return true;
}

bool TResMeshCluster::SaveClusterObjFile(const TString& Filename)
{
	TStringStream SS;

	for (const auto& P : P)
	{
		SS << "v " << P.X << " " << P.Y << " " << P.Z << "\n";
	}
	SS << "\n";
	SS << "vt 0.0 0.0 0.0\n\n";
	SS << "vn 0 0 1\n\n";

	const uint32 NClusters = (uint32)Clusters.size();
	for (uint32 c = 0; c < NClusters; ++c)
	{
		if (Clusters[c].size() == 0)
		{
			continue;
		}
		char Name[128];
		sprintf(Name, "Cluster%03d", c);
		SS << "g " << Name << "\n";
		SS << "s " << c << "\n";

		for (uint32 PIndex : Clusters[c])
		{
			const vector3di& Prim = Prims[PIndex];
			vector3di T = Prim + vector3di(1, 1, 1);

			SS << "f ";
			SS << T.X << "/1/1 ";
			SS << T.Y << "/1/1 ";
			SS << T.Z << "/1/1\n";
		}
		SS << "\n";
	}

	// Write to file
	TFile objfile;
	if (objfile.Open(Filename, EFA_CREATEWRITE))
	{
		objfile.Write(SS.str().c_str(), (int32)SS.str().size());
		objfile.Close();
	}

	return true;
}