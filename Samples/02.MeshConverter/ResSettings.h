/*
	TiX Engine v2.0 Copyright (C) 2018~2019
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
		: ForceAlphaChannel(false)
		, IgnoreTexture(false)
		, AstcQuality(Astc_Quality_Low)
	{}

	TString SrcPath;
	TString SrcName;
	bool ForceAlphaChannel;
	bool IgnoreTexture;
	TString VTInfoFile;
	int32 AstcQuality;
};