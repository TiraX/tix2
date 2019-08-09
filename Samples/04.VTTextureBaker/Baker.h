/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class TVTTextureBaker
	{
	public:
		bool bDumpAllPages;
		bool bDumpAllVTs;
		bool bDumpAllVTWithBorder;
		bool bIgnoreBorders;
		bool bDebugBorders;

		// Virtual texture size
		int32 VTSize;
		// Physical page size
		int32 PPSize;
		// Indirection texture size
		//static const int32 ITSize = VTSize / PPSize;

		struct TVTTextureBasicInfo
		{
			TString Name;
			vector2di Size;
			int32 AddressMode;
			int32 Srgb;
			int32 LodBias;
		};

		TVTTextureBaker();
		~TVTTextureBaker();

		void Bake(const TString& InSceneFileName, const TString& InOutputPath);

	private:
		void LoadTextureFiles(const TString& SceneFileName);
		void AddTexturesToVTRegion();
		void SortTextures(TList<int32>& OrderArray);
		void SplitTextures(const TList<int32>& OrderArray);
		void BakeMipmapsMT();
		void CompressTextures();

		void ClearAllTextures();

		void OutputDebugTextures();

	private:
		TRegion VTRegion;

		TVector<TVTTextureBasicInfo> TextureInfos;
		TVector<vector4di> TextureRegionInVT;
		TVector<THMap<int32, TImage*> > MipPages;
		TVector<THMap<int32, TImage*> > MipPagesWithBorder;

		TString SceneFileName;
		TString OutputPath;
	};
}