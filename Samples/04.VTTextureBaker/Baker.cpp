/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "Baker.h"
#include "ResMultiThreadTask.h"

namespace tix
{
	static const bool bAddBorder = true;
	static const int32 BorderWidth = 1;
	TVTTextureBaker::TVTTextureBaker()
	{
		TResMTTaskExecuter::Create();
	}

	TVTTextureBaker::~TVTTextureBaker()
	{
		TResMTTaskExecuter::Destroy();
		ClearAllTextures();
	}

	inline int32 PickSize(double MinLength)
	{
		int32 Result = 1024;
		while (Result < (int32)MinLength)
		{
			Result *= 2;
		}
		return Result;
	}

	void TVTTextureBaker::LoadTextureFiles(const TString& SceneFileName)
	{
		printf("Loading Textures...\n");
		TFile f;
		if (!f.Open(SceneFileName, EFA_READ))
		{
			printf("Error : Failed to open file : %s\n", SceneFileName.c_str());
			return;
		}

		int8* content = ti_new int8[f.GetSize() + 1];
		f.Read(content, f.GetSize(), f.GetSize());
		content[f.GetSize()] = 0;
		f.Close();

		TJSON JsonDoc;
		JsonDoc.Parse(content);
		ti_delete content;

		TJSONNode JAssetList = JsonDoc["dependency"];
		TJSONNode JAssetTextures = JAssetList["textures"];

		// Load texture names
		TVector<TString> TextureNames;
		TextureNames.reserve(JAssetTextures.Size());
		for (int32 i = 0; i < JAssetTextures.Size(); ++i)
		{
			TJSONNode JTexture = JAssetTextures[i];
			TextureNames.push_back(JTexture.GetString());
		}

		// Load texture basic infos
		TextureInfos.reserve(TextureNames.size());
		for (const auto& T : TextureNames)
		{
			size_t Pos = T.find(".tasset");
			TString TJSName = T.substr(0, Pos) + ".tjs";

			TFile TexTjsFile;
			if (!TexTjsFile.Open(TJSName, EFA_READ))
			{
				printf("Error: Failed to open %s\n", TJSName.c_str());
				continue;
			}

			int8* TexJson = ti_new int8[TexTjsFile.GetSize() + 1];
			TexTjsFile.Read(TexJson, TexTjsFile.GetSize(), TexTjsFile.GetSize());
			TexJson[TexTjsFile.GetSize()] = 0;
			TexTjsFile.Close();

			TJSON TexJsonDoc;
			TexJsonDoc.Parse(TexJson);
			ti_delete TexJson;

			TVTTextureBasicInfo Info;
			Info.AddressMode = GetAddressMode(TexJsonDoc["address_mode"].GetString());
			Info.Srgb = TexJsonDoc["srgb"].GetInt();
			Info.LodBias = TexJsonDoc["lod_bias"].GetInt();

			Pos = T.rfind('/');
			Info.Name = T.substr(0, Pos + 1) + TexJsonDoc["source"].GetString();

			Info.Size = TImage::LoadImageTGADimension(Info.Name);
			TextureInfos.push_back(Info);
			printf(" - Loaded %s...\n", Info.Name.c_str());
		}

		AddTexturesToVTRegion();

		TList<int32> TextureOrders;
		SortTextures(TextureOrders);
		SplitTextures(TextureOrders);
		BakeMipmapsMT();

		OutputAllTextures();
	}

	void TVTTextureBaker::AddTexturesToVTRegion()
	{
		printf("Add textures to virtual texture...\n");
		// Calc total area of all textures to estimate the area of virtual texture
		int32 Area = 0;
		for (const auto& Info : TextureInfos)
		{
			Area += Info.Size.X * Info.Size.Y;
		}
		double MinLength = sqrt(Area);
		int32 VTSize = PickSize(MinLength);
		int32 ITSize = VTSize / PPSize;

		// Init regions
		VTRegion.Reset(VTSize, PPSize);

		TextureRegionInVT.reserve(TextureInfos.size());
		for (const auto& Info : TextureInfos)
		{
			uint32 RegionIndex;
			int32 SizeX = Info.Size.X >> Info.LodBias;
			int32 SizeY = Info.Size.Y >> Info.LodBias;
			TRegion::TRegionDesc* Region = VTRegion.FindAvailbleRegion(SizeX, SizeY, &RegionIndex);
			if (Region == nullptr)
			{
				printf("Error: out of regions. \n");
				return;
			}

			int32 x = RegionIndex % ITSize;
			int32 y = RegionIndex / ITSize;
			TextureRegionInVT.push_back(vector4di(x, y, Region->XCount, Region->YCount));
			//InPrimitive->SetUVTransform(x * UVInv, y * UVInv, Region->XCount * UVInv, Region->YCount * UVInv);
		}
	}

	struct TSortByPositionInVT
	{
		TSortByPositionInVT(const TVector<vector4di>& InTextureRegions)
			: TextureRegions(InTextureRegions)
		{}

		const TVector<vector4di>& TextureRegions;

		bool operator()(int32 A, int32 B)
		{
			const vector4di& PA = TextureRegions[A];
			const vector4di& PB = TextureRegions[B];
			if (PA.Y != PB.Y)
			{
				return PA.Y < PB.Y;
			}
			else if (PA.X != PB.X)
			{
				return PA.X < PB.X;
			}
			return false;
		}
	};

	void TVTTextureBaker::SortTextures(TList<int32>& OrderArray)
	{
		OrderArray.clear();
		for (size_t i = 0 ; i < TextureInfos.size() ; ++ i)
		{
			OrderArray.push_back((int32)i);
		}

		OrderArray.sort(TSortByPositionInVT(TextureRegionInVT));
	}

	inline int32 CalcMips(int32 RegionSize, int32 MinSize)
	{
		int32 Mips = 1;
		while (RegionSize > MinSize)
		{
			++Mips;
			RegionSize /= 2;
		}
		return Mips;
	}

	static TImage* ProcessBorders(TImage * TgaImage, const recti& SrcRegion, const TVTTextureBaker::TVTTextureBasicInfo& Info, int32 PPSize)
	{
		const int32 PPSizeWithBorder = PPSize + BorderWidth * 2;
		int32 SizeX = Info.Size.X >> Info.LodBias;
		int32 SizeY = Info.Size.Y >> Info.LodBias;

		TImage * PageImageWithBorder = ti_new TImage(TgaImage->GetFormat(), PPSizeWithBorder, PPSizeWithBorder);

		// Copy original image to center
		TgaImage->CopyRegionTo(PageImageWithBorder, recti(1, 1, PPSizeWithBorder - 1, PPSizeWithBorder - 1), 0, SrcRegion, 0);

		if (Info.AddressMode != ETC_REPEAT)
		{
			TI_ASSERT(0);
			printf("Error : unsupported address mode for VT. %s\n", Info.Name.c_str());
		}

		// Left border
		if (SrcRegion.Left == 0)
		{
			recti SrcBorderRegion = SrcRegion;
			SrcBorderRegion.Left = SizeX - 1;
			SrcBorderRegion.Right = SizeX;
			TgaImage->CopyRegionTo(PageImageWithBorder, recti(0, 1, 1, PPSizeWithBorder - 1), 0, SrcBorderRegion, 0);
		}
		else
		{
			recti SrcBorderRegion = SrcRegion;
			SrcBorderRegion.Left = SrcRegion.Left - 1;
			SrcBorderRegion.Right = SrcRegion.Left;
			TgaImage->CopyRegionTo(PageImageWithBorder, recti(0, 1, 1, PPSizeWithBorder - 1), 0, SrcBorderRegion, 0);
		}

		// Right border
		if (SrcRegion.Right == SizeX)
		{
			recti SrcBorderRegion = SrcRegion;
			SrcBorderRegion.Left = 0;
			SrcBorderRegion.Right = 1;
			TgaImage->CopyRegionTo(PageImageWithBorder, recti(PPSizeWithBorder - 1, 1, PPSizeWithBorder, PPSizeWithBorder - 1), 0, SrcBorderRegion, 0);
		}
		else
		{
			recti SrcBorderRegion = SrcRegion;
			SrcBorderRegion.Left = SrcRegion.Right;
			SrcBorderRegion.Right = SrcRegion.Right + 1;
			TgaImage->CopyRegionTo(PageImageWithBorder, recti(PPSizeWithBorder - 1, 1, PPSizeWithBorder, PPSizeWithBorder - 1), 0, SrcBorderRegion, 0);
		}

		// Top border
		if (SrcRegion.Upper == 0)
		{
			recti SrcBorderRegion = SrcRegion;
			SrcBorderRegion.Upper = SizeY - 1;
			SrcBorderRegion.Lower = SizeY;
			TgaImage->CopyRegionTo(PageImageWithBorder, recti(1, 0, PPSizeWithBorder - 1, 1), 0, SrcBorderRegion, 0);
		}
		else
		{
			recti SrcBorderRegion = SrcRegion;
			SrcBorderRegion.Upper = SrcRegion.Upper - 1;
			SrcBorderRegion.Lower = SrcRegion.Upper;
			TgaImage->CopyRegionTo(PageImageWithBorder, recti(1, 0, PPSizeWithBorder - 1, 1), 0, SrcBorderRegion, 0);
		}

		// Bottom border
		if (SrcRegion.Lower == SizeY)
		{
			recti SrcBorderRegion = SrcRegion;
			SrcBorderRegion.Upper = 0;
			SrcBorderRegion.Lower = 1;
			TgaImage->CopyRegionTo(PageImageWithBorder, recti(1, PPSizeWithBorder - 1, PPSizeWithBorder - 1, PPSizeWithBorder), 0, SrcBorderRegion, 0);
		}
		else
		{
			recti SrcBorderRegion = SrcRegion;
			SrcBorderRegion.Upper = SrcRegion.Lower;
			SrcBorderRegion.Lower = SrcRegion.Lower + 1;
			TgaImage->CopyRegionTo(PageImageWithBorder, recti(1, PPSizeWithBorder - 1, PPSizeWithBorder - 1, PPSizeWithBorder), 0, SrcBorderRegion, 0);
		}

		// corners
		int32 SrcX, SrcY;
		SColor SrcC;
		{
			// top left
			SrcX = SrcRegion.Left - 1;
			SrcY = SrcRegion.Upper - 1;
			if (SrcRegion.Left == 0)
				SrcX = SizeX - 1;
			if (SrcRegion.Upper == 0)
				SrcY = SizeY - 1;
			SrcC = TgaImage->GetPixel(SrcX, SrcY);
			PageImageWithBorder->SetPixel(0, 0, SrcC);
		}
		{
			// top right
			SrcX = SrcRegion.Right;
			SrcY = SrcRegion.Upper - 1;
			if (SrcRegion.Right == SizeX)
				SrcX = 0;
			if (SrcRegion.Upper == 0)
				SrcY = SizeY - 1;
			SrcC = TgaImage->GetPixel(SrcX, SrcY);
			PageImageWithBorder->SetPixel(PPSizeWithBorder - 1, 0, SrcC);
		}
		{
			// bottom left
			SrcX = SrcRegion.Left - 1;
			SrcY = SrcRegion.Lower;
			if (SrcRegion.Left == 0)
				SrcX = SizeX - 1;
			if (SrcRegion.Lower == SizeY)
				SrcY = 0;
			SrcC = TgaImage->GetPixel(SrcX, SrcY);
			PageImageWithBorder->SetPixel(0, PPSizeWithBorder - 1, SrcC);
		}
		{
			// bottom right
			SrcX = SrcRegion.Right;
			SrcY = SrcRegion.Lower;
			if (SrcRegion.Right == SizeX)
				SrcX = 0;
			if (SrcRegion.Lower == SizeY)
				SrcY = 0;
			SrcC = TgaImage->GetPixel(SrcX, SrcY);
			PageImageWithBorder->SetPixel(PPSizeWithBorder - 1, PPSizeWithBorder - 1, SrcC);
		}

		return PageImageWithBorder;
	}

	void TVTTextureBaker::SplitTextures(const TList<int32>& OrderArray)
	{
		printf("Split textures...\n");
		const int32 TotalMips = CalcMips(VTRegion.GetRegionSize(), VTRegion.GetCellSize());
		const int32 RegionSize = VTRegion.GetRegionSize() / VTRegion.GetCellSize();

		MipPages.clear();
		MipPages.resize(TotalMips);
		
		int32 Size = RegionSize;
		for (int32 M = 0 ; M < (int32)MipPages.size(); ++ M)
		{
			for (int32 S = 0; S < Size * Size; ++ S)
			{
				MipPages[M][S] = nullptr;
			}
			Size /= 2;
		}

		THMap<int32, TImage*>& SplitedTextures = MipPages[0];

		for (const auto& Index : OrderArray)
		{
			const TVTTextureBasicInfo& Info = TextureInfos[Index];
			const vector4di& Region = TextureRegionInVT[Index];

			int32 SizeX = Info.Size.X >> Info.LodBias;
			int32 SizeY = Info.Size.Y >> Info.LodBias;

			TI_ASSERT(Region.Z == SizeX / PPSize && Region.W == SizeY / PPSize);
			
			TFile TgaFile;
			if (!TgaFile.Open(Info.Name, EFA_READ))
			{
				printf("Error: failed to open tga file %s\n", Info.Name.c_str());
				continue;
			}
			TImage * TgaImage = TImage::LoadImageTGA(TgaFile);
			if (Info.LodBias > 0)
			{
				TgaImage->GenerateMipmaps(Info.LodBias + 1);
				
				TImage * TargetMipImage = ti_new TImage(TgaImage->GetFormat(), SizeX, SizeY);
				TgaImage->CopyRegionTo(TargetMipImage, recti(0, 0, SizeX, SizeY), 0, recti(0, 0, SizeX, SizeY), 1);
				
				ti_delete TgaImage;
				TgaImage = TargetMipImage;
			}

			int32 XCount = Region.Z;
			int32 YCount = Region.W;

			for (int32 y = 0 ; y < YCount ; ++ y)
			{
				for (int32 x = 0 ; x < XCount ; ++ x)
				{
					recti SrcRegion(x * PPSize, y * PPSize, (x + 1) * PPSize, (y + 1) * PPSize);

					TImage * PageImage = ti_new TImage(TgaImage->GetFormat(), PPSize, PPSize);
					if (bAddBorder)
					{
						TImage * PageImageWithBorder = ProcessBorders(TgaImage, SrcRegion, Info, PPSize);
						if (Info.Srgb)
						{
							PageImageWithBorder->ConvertToLinearSpace();
						}
						PageImageWithBorder->CopyRegionTo(PageImage, recti(0, 0, PPSize, PPSize), 0, recti(0, 0, PPSize + BorderWidth * 2, PPSize + BorderWidth * 2), 0);
						if (Info.Srgb)
						{
							PageImage->ConvertToSrgbSpace();
						}

						// debug
						//if (Region.X + x == 0 && (Region.Y + y == 2 || Region.Y + y == 6))
						//{
						//	if (Info.Srgb)
						//	{
						//		PageImageWithBorder->ConvertToSrgbSpace();
						//	}
						//	char name0[128], name1[128];
						//	sprintf_s(name0, 128, "%d_%d_bord.tga", Region.X + x, Region.Y + y);
						//	sprintf_s(name1, 128, "%d_%d_page.tga", Region.X + x, Region.Y + y);
						//	PageImageWithBorder->SaveToTga(name0);
						//	PageImage->SaveToTga(name1);
						//}
						ti_delete PageImageWithBorder;
					}
					else
					{
						TgaImage->CopyRegionTo(PageImage, recti(0, 0, PPSize, PPSize), 0, SrcRegion, 0);
					}

					vector2di Pos;
					Pos.X = Region.X + x;
					Pos.Y = Region.Y + y;

					int32 PageIndex = Pos.Y * RegionSize + Pos.X;
					TI_ASSERT(SplitedTextures[PageIndex] == nullptr);
					SplitedTextures[PageIndex] = PageImage;
				}
			}
			ti_delete TgaImage;
		}
	}

	static TImage* DownSamplePages(THMap<int32, TImage*>& ParentPages, int32 x, int32 y, int32 MipSize, int32 PPSize)
	{
		E_PIXEL_FORMAT Format = EPF_UNKNOWN;

		TImage* i00 = ParentPages[y * MipSize + x];
		TImage* i10 = ParentPages[y * MipSize + x + 1];
		TImage* i01 = ParentPages[(y + 1) * MipSize + x];
		TImage* i11 = ParentPages[(y + 1) * MipSize + x + 1];

		if (i00 != nullptr)
		{
			i00->GenerateMipmaps(2);
			Format = i00->GetFormat();
		}
		if (i10 != nullptr)
		{
			i10->GenerateMipmaps(2);
			Format = i10->GetFormat();
		}
		if (i01 != nullptr)
		{
			i01->GenerateMipmaps(2);
			Format = i01->GetFormat();
		}
		if (i11 != nullptr)
		{
			i11->GenerateMipmaps(2);
			Format = i11->GetFormat();
		}

		if (Format == EPF_UNKNOWN)
		{
			return nullptr;
		}

		recti SrcRect(0, 0, PPSize / 2, PPSize / 2);

		TImage* PageImage = ti_new TImage(Format, PPSize, PPSize);
		if (i00 != nullptr)
		{
			i00->CopyRegionTo(PageImage, recti(0, 0, PPSize / 2, PPSize / 2), 0, SrcRect, 1);
		}
		if (i10 != nullptr)
		{
			i10->CopyRegionTo(PageImage, recti(PPSize / 2, 0, PPSize, PPSize / 2), 0, SrcRect, 1);
		}
		if (i01 != nullptr)
		{
			i01->CopyRegionTo(PageImage, recti(0, PPSize / 2, PPSize / 2, PPSize), 0, SrcRect, 1);
		}
		if (i11 != nullptr)
		{
			i11->CopyRegionTo(PageImage, recti(PPSize / 2, PPSize / 2, PPSize, PPSize), 0, SrcRect, 1);
		}

		return PageImage;
	}

	void TVTTextureBaker::BakeMipmaps()
	{
		printf("Bake mips...\n");
		const int32 RegionSize = VTRegion.GetRegionSize() / VTRegion.GetCellSize();

		int32 MipSize = RegionSize;
		for (int32 Mip = 1 ; Mip < (int32)MipPages.size() ; ++ Mip)
		{
			THMap<int32, TImage*>& ParentPages = MipPages[Mip - 1];
			THMap<int32, TImage*>& CurrentPages = MipPages[Mip];

			int32 CurrMipSize = MipSize / 2;
			for (int32 y = 0; y < MipSize; y += 2)
			{
				for (int32 x = 0; x < MipSize; x += 2)
				{
					TImage* PageImage = DownSamplePages(ParentPages, x, y, MipSize, PPSize);

					if (PageImage == nullptr)
					{
						continue;
					}

					int32 PageIndex = (y / 2 * CurrMipSize) + x / 2;
					CurrentPages[PageIndex] = PageImage;
				}
			}

			MipSize /= 2;
		}
	}

	class TBakePageMipmapTask : public TResMTTask
	{
	public:
		TBakePageMipmapTask(THMap<int32, TImage*>* InParentPages, int32 InX, int32 InY, int32 InMipSize, int32 InPPSize)
			: ParentPages(InParentPages)
			, X(InX)
			, Y(InY)
			, MipSize(InMipSize)
			, PPSize(InPPSize)
			, ResultImage(nullptr)
		{}

		THMap<int32, TImage*>* ParentPages;
		int32 X, Y;
		int32 MipSize;
		int32 PPSize;

		TImage* ResultImage;

		virtual void Exec() override
		{
			ResultImage = DownSamplePages(*ParentPages, X, Y, MipSize, PPSize);
		}
	};

	void TVTTextureBaker::BakeMipmapsMT()
	{
		printf("Bake mips multi threads...\n");
		const int32 RegionSize = VTRegion.GetRegionSize() / VTRegion.GetCellSize();
		const int32 MaxThreads = TResMTTaskExecuter::Get()->GetMaxThreadCount();

		int32 MipSize = RegionSize;
		for (int32 Mip = 1; Mip < (int32)MipPages.size(); ++Mip)
		{
			THMap<int32, TImage*>& ParentPages = MipPages[Mip - 1];
			THMap<int32, TImage*>& CurrentPages = MipPages[Mip];

			TVector<TBakePageMipmapTask*> Tasks;
			Tasks.reserve(MipPages[Mip].size() / MaxThreads);

			int32 CurrMipSize = MipSize / 2;
			// Send tasks
			for (int32 y = 0; y < MipSize; y += 2)
			{
				for (int32 x = 0; x < MipSize; x += 2)
				{
					TBakePageMipmapTask * Task = ti_new TBakePageMipmapTask(&ParentPages, x, y, MipSize, PPSize);
					TResMTTaskExecuter::Get()->AddTask(Task);
					Tasks.push_back(Task);

				}
			}
			// Execute in parallel
			TResMTTaskExecuter::Get()->StartTasks();
			TResMTTaskExecuter::Get()->WaitUntilFinished();
			for (auto Task : Tasks)
			{
				int32 x = Task->X;
				int32 y = Task->Y;
				int32 PageIndex = (y / 2 * CurrMipSize) + x / 2;
				CurrentPages[PageIndex] = Task->ResultImage;

				ti_delete Task;
			}
			Tasks.clear();

			MipSize /= 2;
		}
	}

	void TVTTextureBaker::ClearAllTextures()
	{
		for (int32 M = 0; M < (int32)MipPages.size(); ++M)
		{
			THMap<int32, TImage*>& Pages = MipPages[M];

			for (auto& Page : Pages)
			{
				if (Page.second != nullptr)
				{
					ti_delete Page.second;
				}
			}
		}
		MipPages.clear();
	}

	void TVTTextureBaker::OutputAllTextures()
	{
		bool DebugOutputAllPages = !true;
		bool DebugOutputVTMips = !true;

		if (DebugOutputAllPages)
		{
			printf("Dump all pages...\n");
			int32 RegionSize = VTRegion.GetRegionSize() / VTRegion.GetCellSize();

			for (int32 M = 0; M < (int32)MipPages.size(); ++M)
			{
				THMap<int32, TImage*>& Pages = MipPages[M];

				for (auto& Page : Pages)
				{
					if (Page.second != nullptr)
					{
						int32 PageX = Page.first % RegionSize;
						int32 PageY = Page.first / RegionSize;

						char name[256];
						sprintf_s(name, 256, "page_%02d_%03d_%03d.tga", M, PageX, PageY);
						Page.second->SaveToTga(name, 0);
					}
				}

				RegionSize /= 2;
			}
		}

		if (DebugOutputVTMips)
		{
			printf("Dump virtual textures...\n");
			int32 RegionSize = VTRegion.GetRegionSize() / VTRegion.GetCellSize();

			for (int32 M = 0; M < (int32)MipPages.size(); ++M)
			{
				if (false && M < 2)
				{
					RegionSize /= 2;
					continue;
				}
				TImage * VTMip = ti_new TImage(EPF_RGBA8, RegionSize * PPSize, RegionSize * PPSize);

				THMap<int32, TImage*>& Pages = MipPages[M];

				for (auto& Page : Pages)
				{
					if (Page.second != nullptr)
					{
						int32 PageX = Page.first % RegionSize;
						int32 PageY = Page.first / RegionSize;

						Page.second->CopyRegionTo(VTMip, recti(PageX * PPSize, PageY * PPSize, PageX * PPSize + PPSize, PageY * PPSize + PPSize), 0, recti(0, 0, PPSize, PPSize), 0);
					}
				}

				char name[128];
				sprintf_s(name, 128, "vt_%d.tga", M);
				VTMip->SaveToTga(name);

				ti_delete VTMip;

				RegionSize /= 2;
			}
		}
	}
}
