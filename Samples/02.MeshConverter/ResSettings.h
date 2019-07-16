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
		: ISPCCompress(true)
		, ForceAlphaChannel(false)
		, IgnoreTexture(false)
	{}

	TString SrcPath;
	TString SrcName;
	bool ISPCCompress;
	bool ForceAlphaChannel;
	bool IgnoreTexture;
	TString VTInfoFile;
};