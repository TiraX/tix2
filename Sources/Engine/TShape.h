/*
	TiX Engine v2.0 Copyright (C) 2018~2019
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
			TVector<uint16>& OutIndices);

		static void CreateBox(
			const vector3df& Center, 
			const vector3df& Edges, 
			const quaternion& Rotation, 
			TVector<vector3df>& OutPositions, 
			TVector<uint16>& OutIndices);

		static void CreateCapsule(
			uint32 Latitude,
			uint32 Longitude,
			const vector3df& Center, 
			float Radius, 
			float Length, 
			const quaternion& Rotation, 
			TVector<vector3df>& OutPositions, 
			TVector<uint16>& OutIndices);
	};
}
