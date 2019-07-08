// MeshConverter.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "VTTextureBaker.h"

TString FilenameSrc;
TString FilenameDst;

void ShowUsage()
{
	//printf("ResConverter src_filename dst_filename\n");
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
		else if (FilenameSrc == (""))
		{
			FilenameSrc = argv[i];
			//GetPathAndName(FilenameSrc, TResSettings::GlobalSettings.SrcPath, TResSettings::GlobalSettings.SrcName);
		}
		else if (FilenameDst == (""))
		{
			FilenameDst = argv[i];
		}
	}

	if (!FilenameSrc.empty() && FilenameDst.empty())
	{
		// Save dst file to the same directory with src file.
		size_t pos = FilenameSrc.rfind('.');
		if (pos != TString::npos)
		{
			FilenameDst = FilenameSrc.substr(0, pos) + ".tasset";
		}
		else
		{
			FilenameDst = FilenameSrc + ".tasset";
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

	// Read json file.
	TFile f;
	if (f.Open(FilenameSrc, EFA_READ))
	{
		int8* content = ti_new int8[f.GetSize() + 1];
		f.Read(content, f.GetSize(), f.GetSize());
		content[f.GetSize()] = 0;
		f.Close();

		TJSON JsonDoc;
		JsonDoc.Parse(content);

		const int8* type = JsonDoc["type"].GetString();
	}
	else
	{
		printf("Error: failed to open JSON file %s.\n", FilenameSrc.c_str());
	}

    return 0;
}
