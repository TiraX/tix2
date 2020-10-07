/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class TI_API TShape
	{
	public:
		static void CreateICOSphere(
			uint32 Frequency,
			const vector3df& Center, 
			float Radius, 
			TVector<vector3df>& OutPositions, 
			TVector<uint32>& OutIndices);

		static void CreateBox(
			const vector3df& Center, 
			const vector3df& Edges, 
			const quaternion& Rotation, 
			TVector<vector3df>& OutPositions, 
			TVector<uint32>& OutIndices);

		static void CreateCapsule(
			uint32 Latitude,
			uint32 Longitude,
			const vector3df& Center, 
			float Radius, 
			float Length, 
			const quaternion& Rotation, 
			TVector<vector3df>& OutPositions, 
			TVector<uint32>& OutIndices);

		static void RecalcNormal(
			const TVector<vector3df>& Positions,
			const TVector<uint32>& Indices,
			TVector<vector3df>& OutNormals);
	};
}
