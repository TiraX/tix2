/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	struct TResInstance
	{
		vector3df Position;
		quaternion Rotation;
		vector3df Scale;
	};
	class TResInstancesHelper
	{
	public:
		TResInstancesHelper();
		~TResInstancesHelper();

		static bool LoadInstances(TJSON& Doc, TStream& OutStream, TVector<TString>& OutStrings);
		void OutputInstances(TStream& OutStream, TVector<TString>& OutStrings);

		TString LinkedMesh;
		uint32 InsFormat;
		TVector<TResInstance> Instances;
	};
}