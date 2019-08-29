/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "Baker.h"
#include "ResMultiThreadTask.h"
#include "ispc_texcomp.h"
#include "PlatformUtils.h"

namespace tix
{
	static TImage* EmptyPage = nullptr;
	static bool bAddBorder = true;
	static bool bDebugBorder = false;
	static const int32 BorderWidth = 1;
	TVTTextureBaker::TVTTextureBaker()
		: bDumpAllPages(false)
		, bDumpAllVTs(false)
		, VTSize(16 * 1024)
		, PPSize(256)
	{
		TResMTTaskExecuter::Create();
	}

	TVTTextureBaker::~TVTTextureBaker()
	{
		if (EmptyPage != nullptr)
		{
			ti_delete EmptyPage;
		}
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

	void TVTTextureBaker::Bake(const TString& InSceneFileName, const TString& InOutputPath)
	{
		TIMER_RECORDER("Bake total time");

		bAddBorder = !bIgnoreBorders;
		bDebugBorder = bDebugBorders;

		size_t Pos = InSceneFileName.rfind('.');
		SceneFileName = InSceneFileName.substr(0, Pos);
		OutputPath = InOutputPath;

		LoadTextureFiles(InSceneFileName);

		AddTexturesToVTRegion();

		TList<int32> TextureOrders;
		SortTextures(TextureOrders);
		SplitTextures(TextureOrders);
		BakeMipmapsMT();

		CompressTextures();

		OutputDebugTextures();
	}

	void TVTTextureBaker::LoadTextureFiles(const TString& SceneFileName)
	{
		TIMER_RECORDER("Load texture files");
		TFile f;
		if (!f.Open(SceneFileName, EFA_READ))
		{
			_LOG(Error, "Failed to open file : %s\n", SceneFileName.c_str());
			return;
		}

		int8* content = ti_new int8[f.GetSize() + 1];
		f.Read(content, f.GetSize(), f.GetSize());
		content[f.GetSize()] = 0;
		f.Close();

		TJSON JsonDoc;
		JsonDoc.Parse(content);
		ti_delete[] content;

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
				_LOG(Error, "Failed to open %s\n", TJSName.c_str());
				continue;
			}

			int8* TexJson = ti_new int8[TexTjsFile.GetSize() + 1];
			TexTjsFile.Read(TexJson, TexTjsFile.GetSize(), TexTjsFile.GetSize());
			TexJson[TexTjsFile.GetSize()] = 0;
			TexTjsFile.Close();

			TJSON TexJsonDoc;
			TexJsonDoc.Parse(TexJson);
			ti_delete[] TexJson;

			TVTTextureBasicInfo Info;
			Info.AddressMode = GetAddressMode(TexJsonDoc["address_mode"].GetString());
			Info.Srgb = TexJsonDoc["srgb"].GetInt();
			Info.LodBias = TexJsonDoc["lod_bias"].GetInt();

			Pos = T.rfind('/');
			Info.Name = T.substr(0, Pos + 1) + TexJsonDoc["source"].GetString();

			Info.Size = TImage::LoadImageTGADimension(Info.Name);
			TextureInfos.push_back(Info);
		}
	}

	void TVTTextureBaker::AddTexturesToVTRegion()
	{
		TIMER_RECORDER("Add Textures to VT region");
		// Calc total area of all textures to estimate the area of virtual texture
		int32 Area = 0;
		for (const auto& Info : TextureInfos)
		{
			Area += Info.Size.X * Info.Size.Y;
		}
		double MinLength = sqrt(Area);
		int32 SuggestVTSize = PickSize(MinLength);
		_LOG(Log, "Suitable VT Size : %d; Actual VT Size %d\n", SuggestVTSize, VTSize);
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
				_LOG(Error, "out of regions. \n");
				return;
			}

			int32 x = RegionIndex % ITSize;
			int32 y = RegionIndex / ITSize;
			TextureRegionInVT.push_back(vector4di(x, y, Region->XCount, Region->YCount));
			//InPrimitive->SetUVTransform(x * UVInv, y * UVInv, Region->XCount * UVInv, Region->YCount * UVInv);
		}

		// Export region info to json
		stringstream ss_json;
		ss_json << "{\n";
		ss_json << "\t\"scene_name\": \"" << SceneFileName << "\", \n";
		ss_json << "\t\"type\": \"scene_vt_regions\", \n";
		ss_json << "\t\"vt_size\": " << VTSize << ", \n";
		ss_json << "\t\"page_size\": " << PPSize << ", \n";
		ss_json << "\t\"regions\": [\n";
		const int32 TotalTex = (int32)TextureInfos.size();
		for (int32 i = 0 ; i < TotalTex ; ++ i)
		{
			const TVTTextureBasicInfo& Info = TextureInfos[i];
			const vector4di& Region = TextureRegionInVT[i];

			TString TexName = Info.Name;
			size_t TgaPos = Info.Name.find(".tga");
			if (TgaPos != TString::npos)
			{
				TexName = TexName.substr(0, TgaPos);
				TexName += ".tasset";
			}

			ss_json << "\t\t{\n";
			ss_json << "\t\t\t\"name\" : \"" << TexName << "\",\n";
			ss_json << "\t\t\t\"region\" : [" << Region.X << ", " << Region.Y << ", " << Region.Z << ", " << Region.W << "]\n";
			ss_json << "\t\t}";
			if (i != TotalTex - 1)
				ss_json << ",\n";
			else
				ss_json << "\n";
		}
		ss_json << "\t]\n";
		ss_json << "}";

		// Save json file
		{
			TString JsonFileName = SceneFileName + "_vt.tjs";
			TString JsonContent = ss_json.str();
			TFile FileJson;
			if (FileJson.Open(JsonFileName, EFA_CREATEWRITE))
			{
				FileJson.Write(JsonContent.c_str(), (int32)JsonContent.size());
			}
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
		TIMER_RECORDER("Sort textures");
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

	// Expand image 1 pixel with neighbor color
//    static TImage* ProcessBorders(TImage * TgaImage, const recti& SrcRegion, int32 PPSize)
//    {
//        const int32 PPSizeWithBorder = PPSize + BorderWidth * 2;
//        int32 SizeX = TgaImage->GetWidth();
//        int32 SizeY = TgaImage->GetHeight();
//
//        TImage * PageImageWithBorder = ti_new TImage(TgaImage->GetFormat(), PPSizeWithBorder, PPSizeWithBorder);
//
//        // Copy original image to center
//        TgaImage->CopyRegionTo(PageImageWithBorder, recti(1, 1, PPSizeWithBorder - 1, PPSizeWithBorder - 1), 0, SrcRegion, 0);
//
//        if (bDebugBorder)
//        {
//            SColor DebugColor(255, 0, 255, 0);
//            for (int32 i = 0 ; i < PPSizeWithBorder ; ++ i)
//            {
//                PageImageWithBorder->SetPixel(i, 0, DebugColor);
//                PageImageWithBorder->SetPixel(i, PPSizeWithBorder - 1, DebugColor);
//                PageImageWithBorder->SetPixel(0, i, DebugColor);
//                PageImageWithBorder->SetPixel(PPSizeWithBorder - 1, i, DebugColor);
//            }
//            return PageImageWithBorder;
//        }
//
//        // Left border
//        if (SrcRegion.Left == 0)
//        {
//            recti SrcBorderRegion = SrcRegion;
//            SrcBorderRegion.Left = SizeX - 1;
//            SrcBorderRegion.Right = SizeX;
//            TgaImage->CopyRegionTo(PageImageWithBorder, recti(0, 1, 1, PPSizeWithBorder - 1), 0, SrcBorderRegion, 0);
//        }
//        else
//        {
//            recti SrcBorderRegion = SrcRegion;
//            SrcBorderRegion.Left = SrcRegion.Left - 1;
//            SrcBorderRegion.Right = SrcRegion.Left;
//            TgaImage->CopyRegionTo(PageImageWithBorder, recti(0, 1, 1, PPSizeWithBorder - 1), 0, SrcBorderRegion, 0);
//        }
//
//        // Right border
//        if (SrcRegion.Right == SizeX)
//        {
//            recti SrcBorderRegion = SrcRegion;
//            SrcBorderRegion.Left = 0;
//            SrcBorderRegion.Right = 1;
//            TgaImage->CopyRegionTo(PageImageWithBorder, recti(PPSizeWithBorder - 1, 1, PPSizeWithBorder, PPSizeWithBorder - 1), 0, SrcBorderRegion, 0);
//        }
//        else
//        {
//            recti SrcBorderRegion = SrcRegion;
//            SrcBorderRegion.Left = SrcRegion.Right;
//            SrcBorderRegion.Right = SrcRegion.Right + 1;
//            TgaImage->CopyRegionTo(PageImageWithBorder, recti(PPSizeWithBorder - 1, 1, PPSizeWithBorder, PPSizeWithBorder - 1), 0, SrcBorderRegion, 0);
//        }
//
//        // Top border
//        if (SrcRegion.Upper == 0)
//        {
//            recti SrcBorderRegion = SrcRegion;
//            SrcBorderRegion.Upper = SizeY - 1;
//            SrcBorderRegion.Lower = SizeY;
//            TgaImage->CopyRegionTo(PageImageWithBorder, recti(1, 0, PPSizeWithBorder - 1, 1), 0, SrcBorderRegion, 0);
//        }
//        else
//        {
//            recti SrcBorderRegion = SrcRegion;
//            SrcBorderRegion.Upper = SrcRegion.Upper - 1;
//            SrcBorderRegion.Lower = SrcRegion.Upper;
//            TgaImage->CopyRegionTo(PageImageWithBorder, recti(1, 0, PPSizeWithBorder - 1, 1), 0, SrcBorderRegion, 0);
//        }
//
//        // Bottom border
//        if (SrcRegion.Lower == SizeY)
//        {
//            recti SrcBorderRegion = SrcRegion;
//            SrcBorderRegion.Upper = 0;
//            SrcBorderRegion.Lower = 1;
//            TgaImage->CopyRegionTo(PageImageWithBorder, recti(1, PPSizeWithBorder - 1, PPSizeWithBorder - 1, PPSizeWithBorder), 0, SrcBorderRegion, 0);
//        }
//        else
//        {
//            recti SrcBorderRegion = SrcRegion;
//            SrcBorderRegion.Upper = SrcRegion.Lower;
//            SrcBorderRegion.Lower = SrcRegion.Lower + 1;
//            TgaImage->CopyRegionTo(PageImageWithBorder, recti(1, PPSizeWithBorder - 1, PPSizeWithBorder - 1, PPSizeWithBorder), 0, SrcBorderRegion, 0);
//        }
//
//        // corners
//        int32 SrcX, SrcY;
//        SColor SrcC;
//        {
//            // top left
//            SrcX = SrcRegion.Left - 1;
//            SrcY = SrcRegion.Upper - 1;
//            if (SrcRegion.Left == 0)
//                SrcX = SizeX - 1;
//            if (SrcRegion.Upper == 0)
//                SrcY = SizeY - 1;
//            SrcC = TgaImage->GetPixel(SrcX, SrcY);
//            PageImageWithBorder->SetPixel(0, 0, SrcC);
//        }
//        {
//            // top right
//            SrcX = SrcRegion.Right;
//            SrcY = SrcRegion.Upper - 1;
//            if (SrcRegion.Right == SizeX)
//                SrcX = 0;
//            if (SrcRegion.Upper == 0)
//                SrcY = SizeY - 1;
//            SrcC = TgaImage->GetPixel(SrcX, SrcY);
//            PageImageWithBorder->SetPixel(PPSizeWithBorder - 1, 0, SrcC);
//        }
//        {
//            // bottom left
//            SrcX = SrcRegion.Left - 1;
//            SrcY = SrcRegion.Lower;
//            if (SrcRegion.Left == 0)
//                SrcX = SizeX - 1;
//            if (SrcRegion.Lower == SizeY)
//                SrcY = 0;
//            SrcC = TgaImage->GetPixel(SrcX, SrcY);
//            PageImageWithBorder->SetPixel(0, PPSizeWithBorder - 1, SrcC);
//        }
//        {
//            // bottom right
//            SrcX = SrcRegion.Right;
//            SrcY = SrcRegion.Lower;
//            if (SrcRegion.Right == SizeX)
//                SrcX = 0;
//            if (SrcRegion.Lower == SizeY)
//                SrcY = 0;
//            SrcC = TgaImage->GetPixel(SrcX, SrcY);
//            PageImageWithBorder->SetPixel(PPSizeWithBorder - 1, PPSizeWithBorder - 1, SrcC);
//        }
//
//        return PageImageWithBorder;
//    }

	// Expand image 1 pixel with duplicated border color
	static TImage* ProcessBorders1(TImage * TgaImage, const recti& SrcRegion, int32 PPSize)
	{
		const int32 PPSizeWithBorder = PPSize + BorderWidth * 2;

		TImage * PageImageWithBorder = ti_new TImage(TgaImage->GetFormat(), PPSizeWithBorder, PPSizeWithBorder);

		// Copy original image to center
		TgaImage->CopyRegionTo(PageImageWithBorder, recti(1, 1, PPSizeWithBorder - 1, PPSizeWithBorder - 1), 0, SrcRegion, 0);

		if (bDebugBorder)
		{
			SColor DebugColor(255, 0, 255, 0);
			for (int32 i = 0; i < PPSizeWithBorder; ++i)
			{
				PageImageWithBorder->SetPixel(i, 0, DebugColor);
				PageImageWithBorder->SetPixel(i, PPSizeWithBorder - 1, DebugColor);
				PageImageWithBorder->SetPixel(0, i, DebugColor);
				PageImageWithBorder->SetPixel(PPSizeWithBorder - 1, i, DebugColor);
			}
			return PageImageWithBorder;
		}

		// top
		TgaImage->CopyRegionTo(PageImageWithBorder, recti(1, 0, PPSizeWithBorder - 1, 1), 0, recti(SrcRegion.Left, SrcRegion.Upper, SrcRegion.Right, SrcRegion.Upper + 1), 0);
		// bottom
		TgaImage->CopyRegionTo(PageImageWithBorder, recti(1, PPSizeWithBorder - 1, PPSizeWithBorder - 1, PPSizeWithBorder), 0, recti(SrcRegion.Left, SrcRegion.Lower - 1, SrcRegion.Right, SrcRegion.Lower), 0);
		// left
		TgaImage->CopyRegionTo(PageImageWithBorder, recti(0, 1, 1, PPSizeWithBorder - 1), 0, recti(SrcRegion.Left, SrcRegion.Upper, SrcRegion.Left + 1, SrcRegion.Lower), 0);
		// right
		TgaImage->CopyRegionTo(PageImageWithBorder, recti(PPSizeWithBorder - 1, 1, PPSizeWithBorder, PPSizeWithBorder - 1), 0, recti(SrcRegion.Right - 1, SrcRegion.Upper, SrcRegion.Right, SrcRegion.Lower), 0);


		// 4 corners
		PageImageWithBorder->SetPixel(0, 0, TgaImage->GetPixel(0, 0));
		PageImageWithBorder->SetPixel(PPSizeWithBorder - 1, 0, TgaImage->GetPixel(PPSize - 1, 0));
		PageImageWithBorder->SetPixel(0, PPSizeWithBorder - 1, TgaImage->GetPixel(0, PPSize - 1));
		PageImageWithBorder->SetPixel(PPSizeWithBorder - 1, PPSizeWithBorder - 1, TgaImage->GetPixel(PPSize - 1, PPSize - 1));

		return PageImageWithBorder;
	}

	class TBakeSplitTextureTask : public TResMTTask
	{
	public:
		TBakeSplitTextureTask(
			const TVTTextureBaker::TVTTextureBasicInfo& InInfo, 
			const vector4di& InRegion, 
			TImage * InTgaImage, 
			const recti& InSrcRegion,
			int32 InPPSize, 
			int32 InX, int32 InY)
			: Info(InInfo)
			, Region(InRegion)
			, TgaImage(InTgaImage)
			, SrcRegion(InSrcRegion)
			, PPSize(InPPSize)
			, X(InX)
			, Y(InY)
			, PageImage(nullptr)
			, PageImageWithBorder(nullptr)
		{}

		const TVTTextureBaker::TVTTextureBasicInfo& Info;
		const vector4di& Region;
		TImage * TgaImage;
		recti SrcRegion;
		int32 PPSize;
		int32 X, Y;

		TImage* PageImage;
		TImage* PageImageWithBorder;

		virtual void Exec() override
		{
			// Keep origin image block
			PageImage = ti_new TImage(TgaImage->GetFormat(), PPSize, PPSize);
			TgaImage->CopyRegionTo(PageImage, recti(0, 0, PPSize, PPSize), 0, SrcRegion, 0);

			// Process with border
			PageImageWithBorder = ti_new TImage(TgaImage->GetFormat(), PPSize, PPSize);
			if (bAddBorder)
			{
				TImage* PageImageExpand = ProcessBorders1(TgaImage, SrcRegion, PPSize);
				if (Info.Srgb)
				{
					PageImageExpand->ConvertToLinearSpace();
				}
				PageImageExpand->CopyRegionTo(PageImageWithBorder, recti(0, 0, PPSize, PPSize), 0, recti(0, 0, PPSize + BorderWidth * 2, PPSize + BorderWidth * 2), 0);
				if (Info.Srgb)
				{
					PageImageWithBorder->ConvertToSrgbSpace();
				}
				ti_delete PageImageExpand;
			}
			else
			{
				TgaImage->CopyRegionTo(PageImageWithBorder, recti(0, 0, PPSize, PPSize), 0, SrcRegion, 0);
			}
		}
	};

	void TVTTextureBaker::SplitTextures(const TList<int32>& OrderArray)
	{
		TIMER_RECORDER("Split textures");
		const int32 TotalMips = CalcMips(VTRegion.GetRegionSize(), VTRegion.GetCellSize());
		const int32 RegionSize = VTRegion.GetRegionSize() / VTRegion.GetCellSize();

		MipPages.clear();
		MipPages.resize(TotalMips);
		MipPagesWithBorder.clear();
		MipPagesWithBorder.resize(TotalMips);
		
		int32 Size = RegionSize;
		for (int32 M = 0 ; M < (int32)MipPages.size(); ++ M)
		{
			for (int32 S = 0; S < Size * Size; ++S)
			{
				MipPages[M][S] = nullptr;
				MipPagesWithBorder[M][S] = nullptr;
			}
			Size /= 2;
		}

		THMap<int32, TImage*>& SplitedTextures = MipPages[0];
		THMap<int32, TImage*>& SplitedTexturesWithBorder = MipPagesWithBorder[0];
		//CurrentPages = MipPages[Mip];

		TVector<TBakeSplitTextureTask*> Tasks;
		TVector<TImage*> TgaImages;

		for (const auto& Index : OrderArray)
		{
			const TVTTextureBasicInfo& Info = TextureInfos[Index];
			const vector4di& Region = TextureRegionInVT[Index];

			if (Info.AddressMode != ETC_REPEAT)
			{
				TI_ASSERT(0);
				_LOG(Error, "unsupported address mode for VT. %s\n", Info.Name.c_str());
			}

			int32 SizeX = Info.Size.X >> Info.LodBias;
			int32 SizeY = Info.Size.Y >> Info.LodBias;

			TI_ASSERT(Region.Z == SizeX / PPSize && Region.W == SizeY / PPSize);
			
			TFile TgaFile;
			if (!TgaFile.Open(Info.Name, EFA_READ))
			{
				_LOG(Error, "failed to open tga file %s\n", Info.Name.c_str());
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
			TgaImages.push_back(TgaImage);

			int32 XCount = Region.Z;
			int32 YCount = Region.W;

			for (int32 y = 0 ; y < YCount ; ++ y)
			{
				for (int32 x = 0 ; x < XCount ; ++ x)
				{
					recti SrcRegion(x * PPSize, y * PPSize, (x + 1) * PPSize, (y + 1) * PPSize);

					TBakeSplitTextureTask * Task = ti_new TBakeSplitTextureTask(Info, Region, TgaImage, SrcRegion, PPSize, x, y);
					TResMTTaskExecuter::Get()->AddTask(Task);
					Tasks.push_back(Task);
				}
			}
		}

		TResMTTaskExecuter::Get()->StartTasks();
		TResMTTaskExecuter::Get()->WaitUntilFinished();

		for (auto Task : Tasks)
		{
			vector2di Pos;
			Pos.X = Task->Region.X + Task->X;
			Pos.Y = Task->Region.Y + Task->Y;

			int32 PageIndex = Pos.Y * RegionSize + Pos.X;
			TI_ASSERT(SplitedTextures[PageIndex] == nullptr);
			SplitedTextures[PageIndex] = Task->PageImage;
			SplitedTexturesWithBorder[PageIndex] = Task->PageImageWithBorder;

			ti_delete Task;
		}

		for (auto Img : TgaImages)
		{
			ti_delete Img;
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
			, ResultImageWithBorder(nullptr)
		{}

		THMap<int32, TImage*>* ParentPages;
		int32 X, Y;
		int32 MipSize;
		int32 PPSize;

		TImage* ResultImage;
		TImage* ResultImageWithBorder;

		virtual void Exec() override
		{
			ResultImage = DownSamplePages(*ParentPages, X, Y, MipSize, PPSize);
			if (ResultImage != nullptr)
			{
				TI_ASSERT(ResultImage->GetWidth() == PPSize);
				ResultImageWithBorder = ti_new TImage(ResultImage->GetFormat(), PPSize, PPSize);
				TImage* ResultImageExpand = ProcessBorders1(ResultImage, recti(0, 0, PPSize, PPSize), PPSize);
				ResultImageExpand->CopyRegionTo(ResultImageWithBorder, recti(0, 0, PPSize, PPSize), 0, recti(0, 0, PPSize + BorderWidth * 2, PPSize + BorderWidth * 2), 0);
				ti_delete ResultImageExpand;
			}
		}
	};

	void TVTTextureBaker::BakeMipmapsMT()
	{
		TIMER_RECORDER("Bake mipmaps MT");
		const int32 RegionSize = VTRegion.GetRegionSize() / VTRegion.GetCellSize();
		const int32 MaxThreads = TResMTTaskExecuter::Get()->GetMaxThreadCount();

		int32 MipSize = RegionSize;
		for (int32 Mip = 1; Mip < (int32)MipPages.size(); ++Mip)
		{
			THMap<int32, TImage*>& ParentPages = MipPages[Mip - 1];
			THMap<int32, TImage*>& CurrentPages = MipPages[Mip];
			THMap<int32, TImage*>& CurrentPagesWithBorder = MipPagesWithBorder[Mip];

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
				CurrentPagesWithBorder[PageIndex] = Task->ResultImageWithBorder;

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
		for (int32 M = 0; M < (int32)MipPagesWithBorder.size(); ++M)
		{
			THMap<int32, TImage*>& Pages = MipPagesWithBorder[M];

			for (auto& Page : Pages)
			{
				if (Page.second != nullptr)
				{
					ti_delete Page.second;
				}
			}
		}
		MipPagesWithBorder.clear();
	}

	class TComporessTextureTask : public TResMTTask
	{
	public:
		TComporessTextureTask(TImage * InTgaPage, int32 InMip, int32 InPageX, int32 InPageY, const TString& InPath)
			: TgaPage(InTgaPage)
			, Mip(InMip)
			, PageX(InPageX)
			, PageY(InPageY)
			, OutputPath(InPath)
		{}

		TImage * TgaPage;
		int32 Mip;
		int32 PageX, PageY;
		TString OutputPath;

		virtual void Exec() override
		{
			E_PIXEL_FORMAT SrcFormat = TgaPage->GetFormat();
			E_PIXEL_FORMAT DstFormat = EPF_UNKNOWN;

			if (SrcFormat == EPF_RGBA8)
			{
#if defined (TI_PLATFORM_WIN32)
				DstFormat = EPF_DDS_DXT5;
#elif defined (TI_PLATFORM_IOS)
                DstFormat = EPF_ASTC4x4;
#else
                TI_ASSERT(0);
#endif
			}
			else
			{
				_LOG(Error, "Un-expected format when compress texture. Mip: %d; PageX: %d; PageY: %d\n", Mip, PageX, PageY);
				return;
			}
			TImage * DstImage = ti_new TImage(DstFormat, TgaPage->GetWidth(), TgaPage->GetHeight());
			const TImage::TSurfaceData& MipData = TgaPage->GetMipmap(0);
			rgba_surface Surface;
			Surface.ptr = (uint8_t*)MipData.Data.GetBuffer();
			Surface.width = MipData.W;
			Surface.height = MipData.H;
			Surface.stride = MipData.RowPitch;

			uint8_t* Dst = (uint8_t*)DstImage->GetMipmap(0).Data.GetBuffer();
			// Call ISPC function to convert
			if (DstFormat == EPF_DDS_DXT5)
			{
				CompressBlocksBC3(&Surface, Dst);
			}
            else if (DstFormat == EPF_ASTC4x4)
            {
                astc_enc_settings AstcEncSetting;
                GetProfile_astc_alpha_fast(&AstcEncSetting, 4, 4);
                CompressBlocksASTC(&Surface, Dst, &AstcEncSetting);
            }
            else
            {
                TI_ASSERT(0);
            }

			// Write raw data
			char name[128];
			sprintf(name, "%s/%02d_%02d_%02d.page", OutputPath.c_str(), Mip, PageX, PageY);
			TFile PageFile;
			if (PageFile.Open(name, EFA_CREATEWRITE))
			{
				PageFile.Write(DstImage->GetMipmap(0).Data.GetBuffer(), DstImage->GetMipmap(0).Data.GetLength());
			}
			//_LOG(Log, " - Output page: %s\n", name);
		}
	};

	void OutputDebugPage(int32 Mip, int32 PageX, int32 PageY, int32 PPSize, const TString& OutPath)
	{
		if (EmptyPage == nullptr)
		{
			// Create empty page
			TImage* RGBEmpty = ti_new TImage(EPF_RGBA8, PPSize, PPSize);
			SColor EmptyColor(255, 0, 255, 255);
			for (int32 y = 0 ; y < PPSize ; ++ y)
			{
				for (int32 x = 0 ; x < PPSize ; ++ x)
				{
					RGBEmpty->SetPixel(x, y, EmptyColor);
				}
			}

			E_PIXEL_FORMAT DstFormat;
			// convert to target format
#if defined (TI_PLATFORM_WIN32)
			DstFormat = EPF_DDS_DXT5;
#elif defined (TI_PLATFORM_IOS)
			DstFormat = EPF_ASTC4x4;
#else
			TI_ASSERT(0);
#endif
			TImage* DstImage = ti_new TImage(DstFormat, PPSize, PPSize);
			uint8_t* Dst = (uint8_t*)DstImage->GetMipmap(0).Data.GetBuffer();

			const TImage::TSurfaceData& MipData = RGBEmpty->GetMipmap(0);
			rgba_surface Surface;
			Surface.ptr = (uint8_t*)MipData.Data.GetBuffer();
			Surface.width = MipData.W;
			Surface.height = MipData.H;
			Surface.stride = MipData.RowPitch;

			// Call ISPC function to convert
			if (DstFormat == EPF_DDS_DXT5)
			{
				CompressBlocksBC3(&Surface, Dst);
			}
			else if (DstFormat == EPF_ASTC4x4)
			{
				astc_enc_settings AstcEncSetting;
				GetProfile_astc_alpha_fast(&AstcEncSetting, 4, 4);
				CompressBlocksASTC(&Surface, Dst, &AstcEncSetting);
			}
			else
			{
				TI_ASSERT(0);
			}
			EmptyPage = DstImage;
			ti_delete RGBEmpty;
		}

		// save EmptyPage.
		char name[128];
		sprintf(name, "%s/%02d_%02d_%02d.page", OutPath.c_str(), Mip, PageX, PageY);
		TFile PageFile;
		if (PageFile.Open(name, EFA_CREATEWRITE))
		{
			PageFile.Write(EmptyPage->GetMipmap(0).Data.GetBuffer(), EmptyPage->GetMipmap(0).Data.GetLength());
		}
	}

	void TVTTextureBaker::CompressTextures()
	{
		TIMER_RECORDER("Compress textures");
		char EndChar = OutputPath.at(OutputPath.length() - 1);
		TString OutputFullPath;
		if (EndChar == '/')
		{
			OutputFullPath = OutputPath + "vt_pages";
		}
		else
		{
			OutputFullPath = OutputPath + "/" + "vt_pages";
		}
		CreateDirectoryIfNotExist(OutputFullPath);

		int32 RegionSize = VTRegion.GetRegionSize() / VTRegion.GetCellSize();

		TVector<TComporessTextureTask*> Tasks;

		TI_ASSERT(MipPages.size() == MipPagesWithBorder.size());
		for (int32 M = 0; M < (int32)MipPages.size(); ++M)
		{
			THMap<int32, TImage*>& Pages = MipPages[M];
			THMap<int32, TImage*>& PagesWithBorder = MipPagesWithBorder[M];

			if (bAddBorder)
			{
				for (auto& Page : PagesWithBorder)
				{
					int32 PageX = Page.first % RegionSize;
					int32 PageY = Page.first / RegionSize;

					if (Page.second != nullptr)
					{
						TComporessTextureTask * Task = ti_new TComporessTextureTask(Page.second, M, PageX, PageY, OutputFullPath);
						TResMTTaskExecuter::Get()->AddTask(Task);
						Tasks.push_back(Task);
					}
					else
					{
						if (bDebugFillAllPages)
						{
							OutputDebugPage(M, PageX, PageY, PPSize, OutputFullPath);
						}
					}
				}
			}
			else
			{
				for (auto& Page : Pages)
				{
					int32 PageX = Page.first % RegionSize;
					int32 PageY = Page.first / RegionSize;

					if (Page.second != nullptr)
					{
						TComporessTextureTask * Task = ti_new TComporessTextureTask(Page.second, M, PageX, PageY, OutputFullPath);
						TResMTTaskExecuter::Get()->AddTask(Task);
						Tasks.push_back(Task);
					}
					else
					{
						if (bDebugFillAllPages)
						{
							OutputDebugPage(M, PageX, PageY, PPSize, OutputFullPath);
						}
					}
				}
			}

			RegionSize /= 2;
		}

		TResMTTaskExecuter::Get()->StartTasks();
		TResMTTaskExecuter::Get()->WaitUntilFinished();

		for (auto T : Tasks)
		{
			ti_delete T;
		}
	}

	void TVTTextureBaker::OutputDebugTextures()
	{
		TIMER_RECORDER("Output debug textures");
		bool DebugOutputAllPages = bDumpAllPages;
		bool DebugOutputVTMips = bDumpAllVTs;

		if (DebugOutputAllPages)
		{
			int32 RegionSize = VTRegion.GetRegionSize() / VTRegion.GetCellSize();

			for (int32 M = 0; M < (int32)MipPages.size(); ++M)
			{
				THMap<int32, TImage*>* Pages;
				if (bIgnoreBorders)
				{
					Pages = &MipPages[M];
				}
				else
				{
					Pages = &MipPagesWithBorder[M];
				}

				for (auto& Page : *Pages)
				{
					if (Page.second != nullptr)
					{
						int32 PageX = Page.first % RegionSize;
						int32 PageY = Page.first / RegionSize;

						char name[256];
						sprintf(name, "page_%02d_%03d_%03d.tga", M, PageX, PageY);
						Page.second->SaveToTga(name, 0);
					}
				}

				RegionSize /= 2;
			}
		}

		if (DebugOutputVTMips)
		{
			_LOG(Log, "Dump virtual textures...\n");
			int32 RegionSize = VTRegion.GetRegionSize() / VTRegion.GetCellSize();

			for (int32 M = 0; M < (int32)MipPages.size(); ++M)
			{
				TImage * VTMip = ti_new TImage(EPF_RGBA8, RegionSize * PPSize, RegionSize * PPSize);

				THMap<int32, TImage*>* Pages;
				if (bIgnoreBorders)
				{
					Pages = &MipPages[M];
				}
				else
				{
					Pages = &MipPagesWithBorder[M];
				}

				for (auto& Page : *Pages)
				{
					if (Page.second != nullptr)
					{
						int32 PageX = Page.first % RegionSize;
						int32 PageY = Page.first / RegionSize;

						if (bDumpAllVTWithBorder)
						{
							// Add Border to page image
							SColor PinkC(255, 255, 0, 255);
							for (int32 i = 0; i < PPSize; ++i)
							{
								Page.second->SetPixel(i, 0, PinkC);
								Page.second->SetPixel(i, PPSize - 1, PinkC);
								Page.second->SetPixel(i, 1, PinkC);
								Page.second->SetPixel(i, PPSize - 2, PinkC);
								Page.second->SetPixel(0, i, PinkC);
								Page.second->SetPixel(PPSize - 1, i, PinkC);
								Page.second->SetPixel(1, i, PinkC);
								Page.second->SetPixel(PPSize - 2, i, PinkC);
							}
						}

						Page.second->CopyRegionTo(VTMip, recti(PageX * PPSize, PageY * PPSize, PageX * PPSize + PPSize, PageY * PPSize + PPSize), 0, recti(0, 0, PPSize, PPSize), 0);
					}
				}

				char name[128];
				sprintf(name, "vt_%d.tga", M);
				VTMip->SaveToTga(name);

				ti_delete VTMip;

				RegionSize /= 2;
			}
		}
	}
}
