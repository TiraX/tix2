/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

class TResSettings
{
public:
	enum
	{
		Astc_Quality_High,
		Astc_Quality_Mid,
		Astc_Quality_Low
	};

	static TResSettings GlobalSettings;
	TResSettings()
		: Iterate(false)
		, Force32BitIndex(false)
		, ForceAlphaChannel(false)
		, IgnoreTexture(false)
		, AstcQuality(Astc_Quality_Low)
		, MeshClusterSize(0)
		, ClusterVerbose(false)
	{}

	TString SrcPath;
	TString SrcName;
	bool Iterate;
	bool Force32BitIndex;
	bool ForceAlphaChannel;
	bool IgnoreTexture;
	TString VTInfoFile;
	int32 AstcQuality;
	int32 MeshClusterSize;
	bool ClusterVerbose;
};