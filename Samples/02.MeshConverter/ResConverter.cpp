// MeshConverter.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "ResConverter.h"
#include "ResHelper.h"
#include "ResMeshHelper.h"
#include "ResTextureHelper.h"
#include "ResPipelineHelper.h"
#include "ResMaterialHelper.h"
#include "ResMaterialInstanceHelper.h"
#include "ResSceneHelper.h"
#include "ResInstancesHelper.h"
#include "PlatformUtils.h"
#include "ResMultiThreadTask.h"

TString FilenameSrc;
TString FilenameDst;

TResSettings TResSettings::GlobalSettings;

void ShowUsage()
{
	_LOG(Log, "ResConverter src_filename dst_filename\n");
}

bool bShowExample = false;
void ShowExample()
{
	_LOG(Log, "{\n");
	_LOG(Log, "\t\"name\": \"M_AddSpecular\",\n");
	_LOG(Log, "\t\"type\": \"material\",\n");
	_LOG(Log, "\t\"version\": 1,\n");
	_LOG(Log, "\t\"desc\": \"\",\n");
	_LOG(Log, "\t\"shaders\": [\n");
	_LOG(Log, "\t\t\"S_AddSpecularVS\",\n");
	_LOG(Log, "\t\t\"S_AddSpecularPS\",\n");
	_LOG(Log, "\t\t\"\",\n");
	_LOG(Log, "\t\t\"\",\n");
	_LOG(Log, "\t\t\"\"\n");
	_LOG(Log, "\t],\n");
	_LOG(Log, "\t\"vs_format\": [\n");
	_LOG(Log, "\t\t\"EVSSEG_POSITION\",\n");
	_LOG(Log, "\t\t\"EVSSEG_TEXCOORD0\"\n");
	_LOG(Log, "\t],\n");
	_LOG(Log, "\t\"rt_colors\": [\n");
	_LOG(Log, "\t\t\"EPF_RGBA16F\"\n");
	_LOG(Log, "\t],\n");
	_LOG(Log, "\t\"rt_depth\": \"EPF_DEPTH24_STENCIL8\",\n");
	_LOG(Log, "\t\"blend_mode\": \"BLEND_MODE_OPAQUE\",\n");
	_LOG(Log, "\t\"depth_write\": false,\n");
	_LOG(Log, "\t\"depth_test\": false,\n");
	_LOG(Log, "\t\"two_sides\": false,\n");
	_LOG(Log, "\t\"stencil_enable\": true,\n");
	_LOG(Log, "\t\"stencil_read_mask\": 1,\n");
	_LOG(Log, "\t\"stencil_write_mask\": 1,\n");
	_LOG(Log, "\t\"front_stencil_fail\": \"ESO_KEEP\",\n");
	_LOG(Log, "\t\"front_stencil_depth_fail\": \"ESO_KEEP\",\n");
	_LOG(Log, "\t\"front_stencil_pass\": \"ESO_KEEP\",\n");
	_LOG(Log, "\t\"front_stencil_func\": \"ECF_EQUAL\",\n");
	_LOG(Log, "\t\"back_stencil_fail\": \"ESO_KEEP\",\n");
	_LOG(Log, "\t\"back_stencil_depth_fail\": \"ESO_KEEP\",\n");
	_LOG(Log, "\t\"back_stencil_pass\": \"ESO_KEEP\",\n");
	_LOG(Log, "\t\"back_stencil_func\": \"ECF_NEVER\"\n");
	_LOG(Log, "}\n");
}

bool ParseParams(int argc, RES_CONVERTER_CONST int8* argv[])
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

			tolower(key);
			tolower(value);

			if (key == "example")
			{
				bShowExample = true;
			}
			else if (key == "ispccompress")
			{
				TResSettings::GlobalSettings.ISPCCompress = true;
			}
			else if (key == "forcealphachannel")
			{
				TResSettings::GlobalSettings.ForceAlphaChannel = true;
			}
			else if (key == "ignoretexture")
			{
				TResSettings::GlobalSettings.IgnoreTexture = true;
			}
			else if (key == "vtinfo")
			{
				TResSettings::GlobalSettings.VTInfoFile = value;
			}

			//if (key == "texture_path")
			//{
			//	_config.TexturePath = value;
			//}
		}
		else if (FilenameSrc == (""))
		{
			FilenameSrc = argv[i];
			GetPathAndName(FilenameSrc, TResSettings::GlobalSettings.SrcPath, TResSettings::GlobalSettings.SrcName);
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

int32 DoConvert(int32 argc, RES_CONVERTER_CONST int8* argv[])
{
	if (argc < 2 || !ParseParams(argc, argv))
	{
		ShowUsage();
		return 0;
	}

	if (bShowExample)
	{
		ShowExample();
		return 0;
	}

	TResMTTaskExecuter::Create();
	TResFileHelper Resfile;
	{
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

			if (strcmp(type, "static_mesh") == 0)
			{
				// Static Mesh
				TStream& MeshStream = Resfile.GetChunk(ECL_MESHES);
				TResMeshHelper::LoadMeshFile(JsonDoc, MeshStream, Resfile.Strings);
			}
			else if (strcmp(type, "texture") == 0)
			{
				if (TResSettings::GlobalSettings.IgnoreTexture)
				{
					TResMTTaskExecuter::Destroy();
					return 0;
				}
				// Texture
				TStream& TextureStream = Resfile.GetChunk(ECL_TEXTURES);
				TResTextureHelper::LoadTextureFile(JsonDoc, TextureStream, Resfile.Strings);
			}
			else if (strcmp(type, "material") == 0)
			{
				// Material
				TStream& MaterialStream = Resfile.GetChunk(ECL_MATERIAL);
				TResMaterialHelper::LoadMaterial(JsonDoc, MaterialStream, Resfile.Strings);
			}
			else if (strcmp(type, "material_instance") == 0)
			{
				// Material Instance
				TStream& MIStream = Resfile.GetChunk(ECL_MATERIAL_INSTANCE);
				TResMaterialInstanceHelper::LoadMaterialInstance(JsonDoc, MIStream, Resfile.Strings);
			}
			else if (strcmp(type, "scene") == 0)
			{
				// Scene
				TStream& MIStream = Resfile.GetChunk(ECL_SCENE);
				TResSceneHelper::LoadScene(JsonDoc, MIStream, Resfile.Strings);
			}
			else if (strcmp(type, "instances") == 0)
			{
				// Instances
				TStream& InsStream = Resfile.GetChunk(ECL_INSTANCES);
				TResInstancesHelper::LoadInstances(JsonDoc, InsStream, Resfile.Strings);
			}
			else
			{
				_LOG(Error, "Unknown asset type - %s.\n", type);
			}
			ti_delete[] content;
		}
		else
		{
			_LOG(Error, "Failed to open file : %s.\n", FilenameSrc.c_str());
		}
	}

	// Find path
	TStringReplace(FilenameDst, "\\", "/");
	TString DstPath;
	TString::size_type SlashPos = FilenameDst.rfind('/');
	if (SlashPos != TString::npos)
	{
		DstPath = FilenameDst.substr(0, SlashPos);
		CreateDirectoryIfNotExist(DstPath);
	}

	if (!Resfile.SaveFile(FilenameDst))
	{
		_LOG(Error, "Failed to save resfile : %s\n", FilenameDst.c_str());
	}
	TResMTTaskExecuter::Destroy();

    return 0;
}
