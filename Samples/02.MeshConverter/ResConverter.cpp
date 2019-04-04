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
#include "PlatformUtils.h"

TString FilenameSrc;
TString FilenameDst;

void ShowUsage()
{
	printf("ResConverter src_filename dst_filename\n");
}

bool bShowExample = false;
void ShowExample()
{
	printf("{\n");
	printf("\t\"name\": \"M_AddSpecular\",\n");
	printf("\t\"type\": \"material\",\n");
	printf("\t\"version\": 1,\n");
	printf("\t\"desc\": \"\",\n");
	printf("\t\"shaders\": [\n");
	printf("\t\t\"S_AddSpecularVS\",\n");
	printf("\t\t\"S_AddSpecularPS\",\n");
	printf("\t\t\"\",\n");
	printf("\t\t\"\",\n");
	printf("\t\t\"\"\n");
	printf("\t],\n");
	printf("\t\"vs_format\": [\n");
	printf("\t\t\"EVSSEG_POSITION\",\n");
	printf("\t\t\"EVSSEG_TEXCOORD0\"\n");
	printf("\t],\n");
	printf("\t\"rt_colors\": [\n");
	printf("\t\t\"EPF_RGBA16F\"\n");
	printf("\t],\n");
	printf("\t\"rt_depth\": \"EPF_DEPTH24_STENCIL8\",\n");
	printf("\t\"blend_mode\": \"BLEND_MODE_OPAQUE\",\n");
	printf("\t\"depth_write\": false,\n");
	printf("\t\"depth_test\": false,\n");
	printf("\t\"two_sides\": false,\n");
	printf("\t\"stencil_enable\": true,\n");
	printf("\t\"stencil_read_mask\": 1,\n");
	printf("\t\"stencil_write_mask\": 1,\n");
	printf("\t\"front_stencil_fail\": \"ESO_KEEP\",\n");
	printf("\t\"front_stencil_depth_fail\": \"ESO_KEEP\",\n");
	printf("\t\"front_stencil_pass\": \"ESO_KEEP\",\n");
	printf("\t\"front_stencil_func\": \"ECF_EQUAL\",\n");
	printf("\t\"back_stencil_fail\": \"ESO_KEEP\",\n");
	printf("\t\"back_stencil_depth_fail\": \"ESO_KEEP\",\n");
	printf("\t\"back_stencil_pass\": \"ESO_KEEP\",\n");
	printf("\t\"back_stencil_func\": \"ECF_NEVER\"\n");
	printf("}\n");
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

			if (key == "example")
			{
				bShowExample = true;
			}

			//if (key == "texture_path")
			//{
			//	_config.TexturePath = value;
			//}
		}
		else if (FilenameSrc == (""))
		{
			FilenameSrc = argv[i];
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
			FilenameDst = FilenameSrc.substr(0, pos) + ".tres";
		}
		else
		{
			FilenameDst = FilenameSrc + ".tres";
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
			ti_delete[] content;
		}
		else
		{
			printf("Error: Failed to open file : %s.\n", FilenameSrc.c_str());
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
		printf("Failed to save resfile : %s\n", FilenameDst.c_str());
	}

    return 0;
}
