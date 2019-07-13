// MeshConverter.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "VTTextureBaker.h"
#include "Baker.h"

TString SceneNameSrc;

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

			//if (key == "texture_path")
			//{
			//	_config.TexturePath = value;
			//}
		}
		else if (SceneNameSrc == (""))
		{
			SceneNameSrc = argv[i];
			//GetPathAndName(FilenameSrc, TResSettings::GlobalSettings.SrcPath, TResSettings::GlobalSettings.SrcName);
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
		Baker.Bake(SceneNameSrc);
	}

    return 0;
}
