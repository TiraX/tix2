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
		// Virtual texture size
		//static const int32 VTSize = 64 * 1024;
		// Physical page size
		static const int32 PPSize = 256;
		// Indirection texture size
		//static const int32 ITSize = VTSize / PPSize;

		static const int32 PhysicAtlasSize = 20;
		// Physical page count
		static const int32 PPCount = PhysicAtlasSize * PhysicAtlasSize;

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

		TString SceneFileName;
		TString OutputPath;
	};
}