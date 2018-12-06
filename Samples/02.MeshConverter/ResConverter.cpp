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

// Test target: ../../Content/head.obj
// Test target: ../../Content/DiffuseMap.dds
// Test target: ../../Content/Material.tjs
// Test target: ../../Content/MaterialInstanceTest.tjs

TString FilenameSrc;
TString FilenameDst;

void ShowUsage()
{
	printf("ResConverter src_filename dst_filename\n");
}

bool ParseParams(int argc, _TCHAR* argv[])
{
	for (int i = 1; i < argc; ++i)
	{
		if (FilenameSrc == _T(""))
		{
			FilenameSrc = argv[i];
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

	if (!FilenameSrc.empty())
	{
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

enum {
	EXT_UNKNOWN,
	EXT_MESH,
	EXT_TEXTURE,
	EXT_JSON,
};

bool CompareString(const TString& A, const TString& B)
{
	TString StrLower = A;
	transform(StrLower.begin(), StrLower.end(), StrLower.begin(), tolower);
	return StrLower == B;
}

static const _TCHAR* mesh_ext[] =
{
	"obj"
};

static const _TCHAR* tex_ext[] =
{
	"dds"
};

static const _TCHAR* json_ext[] =
{
	"tjs"
};

int32 CheckExtension(const TString& Filename)
{
	TString Ext = Filename.substr(Filename.rfind('.') + 1);
	const int32 mesh_ext_count = sizeof(mesh_ext) / sizeof(_TCHAR*);
	const int32 tex_ext_count = sizeof(tex_ext) / sizeof(_TCHAR*);
	const int32 json_ext_count = sizeof(json_ext) / sizeof(_TCHAR*);

	//return EXT_MATERIAL;
	for (int32 i = 0; i < mesh_ext_count; ++i)
	{
		if (CompareString(Ext, mesh_ext[i]))
		{
			return EXT_MESH;
		}
	}
	for (int32 i = 0; i < tex_ext_count; ++i)
	{
		if (CompareString(Ext, tex_ext[i]))
		{
			return EXT_TEXTURE;
		}
	}
	for (int32 i = 0; i < json_ext_count; ++i)
	{
		if (CompareString(Ext, json_ext[i]))
		{
			return EXT_JSON;
		}
	}
	return EXT_UNKNOWN;
}

int main(int argc, _TCHAR* argv[])
{
	if (argc < 2 || !ParseParams(argc, argv))
	{
		ShowUsage();
		return 0;
	}

	TResFileHelper Resfile;
	int32 ExtResult = CheckExtension(FilenameSrc);

	if (ExtResult == EXT_MESH)
	{
		printf("Unknown ext.\n");
		//TStream& MeshStream = Resfile.GetChunk(ECL_MESHES);
		//TResMeshHelper::LoadObjFile(FilenameSrc, MeshStream, Resfile.Strings);
	}
	else if (ExtResult == EXT_TEXTURE)
	{
		printf("Unknown ext.\n");
		//TStream& TextureStream = Resfile.GetChunk(ECL_TEXTURES);
		//TResTextureHelper::LoadDdsFile(FilenameSrc, TextureStream, Resfile.Strings);
	}
	else if (ExtResult == EXT_JSON)
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
	else
	{
		printf("Error: Unknown file extension: %s\n", FilenameSrc.substr(FilenameSrc.rfind('.')).c_str());
	}

	Resfile.SaveFile(FilenameDst);

    return 0;
}