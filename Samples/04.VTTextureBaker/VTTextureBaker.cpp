// MeshConverter.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "VTTextureBaker.h"
#include "Baker.h"

TString SceneNameSrc;
TString OutputPath;

bool bDumpAllPages = false;
bool bDumpAllVTs = false;
bool bDumpAllVTWithBorder = false;
bool bIgnoreBorders = false;
bool bDebugBorders = false;
int32 InputVTSize = -1;
int32 InputPPSize = -1;

void ShowUsage()
{
}

bool ParseParams(int argc, VT_TEXTURE_BAKER_CONST int8* argv[])
{
	for (int i = 1; i < argc; ++i)
	{
		if (argv[i][0] == '-')
		{
			// optional parameters
			TString param = argv[i] + 1;
			size_t pos = param.find('=');
			TString key, value;

			if (pos != TString::npos)
			{
				key = param.substr(0, pos);
				value = param.substr(pos + 1);
			}
			else
			{
				key = param;
			}

			//tolower(key);
			//tolower(value);

			if (key == "DumpAllPages")
			{
				bDumpAllPages = true;
			}
			else if (key == "DumpAllVTs")
			{
				bDumpAllVTs = true;
			}
			else if (key == "DumpAllVTWithBorder")
			{
				bDumpAllVTWithBorder = true;
			}
			else if (key == "IgnoreBorders")
			{
				bIgnoreBorders = true;
			}
			else if (key == "DebugBorders")
			{
				bDebugBorders = true;
			}
			else if (key == "VTSize")
			{
				InputVTSize = atoi(value.c_str());
			}
			else if (key == "PPSize")
			{
				InputPPSize = atoi(value.c_str());
			}
		}
		else if (SceneNameSrc == (""))
		{
			SceneNameSrc = argv[i];
			//GetPathAndName(FilenameSrc, TResSettings::GlobalSettings.SrcPath, TResSettings::GlobalSettings.SrcName);
		}
		else if (OutputPath == (""))
		{
			OutputPath = argv[i];
		}
	}
	return true;
}

int32 DoBake(int32 argc, VT_TEXTURE_BAKER_CONST int8* argv[])
{
	if (argc < 2 || !ParseParams(argc, argv))
	{
		ShowUsage();
		return 0;
	}
	{
		TVTTextureBaker Baker;
		Baker.bDumpAllPages = bDumpAllPages;
		Baker.bDumpAllVTs = bDumpAllVTs;
		Baker.bDumpAllVTWithBorder = bDumpAllVTWithBorder;
		Baker.bIgnoreBorders = bIgnoreBorders;
		Baker.bDebugBorders = bDebugBorders;
		if (InputVTSize > 0)
		{
			Baker.VTSize = InputVTSize;
		}
		if (InputPPSize > 0)
		{
			Baker.PPSize = InputPPSize;
		}
		Baker.Bake(SceneNameSrc, OutputPath);
	}

    return 0;
}
