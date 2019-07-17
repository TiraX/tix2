/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

class TResSettings
{
public:
	static TResSettings GlobalSettings;
	TResSettings()
		: ForceAlphaChannel(false)
		, IgnoreTexture(false)
	{}

	TString SrcPath;
	TString SrcName;
	bool ForceAlphaChannel;
	bool IgnoreTexture;
	TString VTInfoFile;
};