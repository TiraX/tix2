/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"

#if defined (TI_PLATFORM_IOS)
#include "ResHelper.h"
#include "ResTextureHelper.h"

namespace tix
{
	TResTextureDefine* TResTextureHelper::LoadAstcFile(const TString& Filename)
	{
		TFile f;
		if (!f.Open(Filename, EFA_READ))
		{
			return nullptr;
		}

		// Load file to memory
		const int32 FileSize = f.GetSize();
		uint8* FileBuffer = ti_new uint8[FileSize];
		f.Read(FileBuffer, FileSize, FileSize);
		f.Close();

        TI_ASSERT(0);
        
		//TResTextureDefine* Texture = CreateTextureFromDDS(header, FileBuffer + offset, FileSize - offset);
		//Texture->Name = Name;
		//Texture->Path = Path;

		return nullptr;
	}
}
#endif  //TI_PLATFORM_IOS
