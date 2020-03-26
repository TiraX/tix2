/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once
#include "TSphere.h"

class TResMeshCluster
{
public:
	TResMeshCluster();
	TResMeshCluster(
		const TVector<vector3df>& PosArray, 
		const TVector<vector3df>& NormalArray, 
		const TVector<vector3di>& PrimsArray, 
		const TString& InMeshName, 
		int32 InSection, 
		int32 InMinVertexIndex
	);
	~TResMeshCluster();

	void GenerateCluster(uint32 ClusterTriangles);

private:
	void SortPrimitives();
	void CalcPrimNormals();
	void ScatterToVolume();
	void MakeClusters(uint32 ClusterTriangles);
	void GetNeighbourPrims(const TVector<uint32>& InPrims, TVector<uint32>& OutNeighbourPrims, const TVector<uint32>& InPrimsClusterId);
	void MergeSmallClusters(uint32 ClusterTriangles);
	void CalcMetaInfos();
	TSphere GetBoundingSphere(const TVector<vector3df>& Points);

	// For debug
	bool SaveObjFile(const TString& Filename);
	bool SaveClusterObjFile(const TString& Filename);

private:
	TString MeshName;
	int32 Section;
	int32 MinVertexIndex;
	TVector<vector3df> P;
	TVector<vector3df> N;

	aabbox3df BBox;
	TVector<vector3di> Prims;
	TVector<vector3df> PrimsN;

	// Volume cells
	aabbox3df MeshVolume;
	vector3di MeshVolumeCellCount;
	
	// Each cell contains prims intersects with this cell
	TVector< TVector<uint32> > VolumeCells;
	// Each prim remember the cells it intersected
	TVector< TVector<uint32> > PrimVolumePositions;

	// Final Clusters
	TVector< TVector<uint32> > Clusters;
public:
	TVector< TVector<uint32> > ClusterIndices;
	TVector< aabbox3df > ClusterBBoxes;
	TVector< vector4df > ClusterCones;
};