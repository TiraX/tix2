// MeshConverter.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "ResHelper.h"
#include "ResMeshHelper.h"
#include "ResTextureHelper.h"
#include "ResPipelineHelper.h"
#include "ResMaterialHelper.h"
#include "ResMaterialInstanceHelper.h"
#include "ResShaderBindingHelper.h"

TString FilenameSrc;
TString FilenameDst;

void ShowUsage()
{
	printf("ResConverter src_filename dst_filename\n");
}

bool ParseParams(int argc, int8* argv[])
{
	for (int i = 1; i < argc; ++i)
	{
		if (FilenameSrc == (""))
		{
			FilenameSrc = argv[i];
		}
		else if (FilenameDst == (""))
		{
			FilenameDst = argv[i];
		}
		else if (argv[i][0] == '-')
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

			//if (key == "texture_path")
			//{
			//	_config.TexturePath = value;
			//}
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

int32 DoConvert(int32 argc, int8* argv[])
{
	if (argc < 2 || !ParseParams(argc, argv))
	{
		ShowUsage();
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

			Document tjs;
			tjs.Parse(content);
			Value& s = tjs["type"];
			const char* type = s.GetString();

			if (strcmp(type, "static_mesh") == 0)
			{
				// Static Mesh
				TStream& MeshStream = Resfile.GetChunk(ECL_MESHES);
				TResMeshHelper::LoadMeshFile(tjs, MeshStream, Resfile.Strings);
			}
			else if (strcmp(type, "texture") == 0)
			{
				// Texture
				TStream& TextureStream = Resfile.GetChunk(ECL_TEXTURES);
				TResTextureHelper::LoadTextureFile(tjs, TextureStream, Resfile.Strings);
			}
			else if (strcmp(type, "material") == 0)
			{
				// Material
				TStream& MaterialStream = Resfile.GetChunk(ECL_MATERIAL);
				TResMaterialHelper::LoadMaterial(tjs, MaterialStream, Resfile.Strings);
			}
			else if (strcmp(type, "material_instance") == 0)
			{
				// Material Instance
				TStream& MIStream = Resfile.GetChunk(ECL_MATERIAL_INSTANCE);
				TResMaterialInstanceHelper::LoadMaterialInstance(tjs, MIStream, Resfile.Strings);
			}
			else if (strcmp(type, "shader_binding") == 0)
			{
				// Material Parameter Binding
				TStream& MIStream = Resfile.GetChunk(ECL_SHADER_BINDING);
				TResShaderBindingHelper::LoadShaderBinding(tjs, MIStream, Resfile.Strings);
			}
			ti_delete[] content;
		}
		else
		{
			printf("Error: Failed to open file : %s.\n", FilenameSrc.c_str());
		}
	}

	Resfile.SaveFile(FilenameDst);

    return 0;
}
