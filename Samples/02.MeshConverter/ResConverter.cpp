// MeshConverter.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ResHelper.h"
#include "ResMeshHelper.h"
#include "ResTextureHelper.h"
#include "ResPipelineHelper.h"
#include "ResMaterialHelper.h"
#include "ResMaterialInstanceHelper.h"

// Test target: ../../Content/head.obj
// Test target: ../../Content/DiffuseMap.dds

TString FilenameSrc;
TString FilenameDst;

void ShowUsage()
{
	printf("ResConverter src_filename dst_filename\n");
}

bool ParseParams(int argc, _TCHAR* argv[])
{
	if (argc < 3)
	{
		return false;
	}

	for (int i = 1; i < argc; ++i)
	{
		if (FilenameSrc == _T(""))
		{
			FilenameSrc = argv[i];
		}
		else if (FilenameDst == _T(""))
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
	return true;
}

enum {
	EXT_UNKNOWN,
	EXT_MESH,
	EXT_TEXTURE,
	EXT_MATERIAL,
	EXT_MATERIAL_INSTANCE,
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

static const _TCHAR* mat_ext[] =
{
	"mat"
};

int32 CheckExtension(const TString& Filename)
{
	TString Ext = Filename.substr(Filename.rfind('.') + 1);
	const int32 mesh_ext_count = sizeof(mesh_ext) / sizeof(_TCHAR*);
	const int32 tex_ext_count = sizeof(tex_ext) / sizeof(_TCHAR*);

	return EXT_MATERIAL;
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
	return EXT_UNKNOWN;
}

int main(int argc, _TCHAR* argv[])
{
	if (argc < 3 || !ParseParams(argc, argv))
	{
		ShowUsage();
		return 0;
	}

	TResFileHelper Resfile;
	int32 ExtResult = CheckExtension(FilenameSrc);

	if (ExtResult == EXT_MESH)
	{
		TStream& MeshStream = Resfile.GetChunk(ECL_MESHES);
		TResMeshHelper::LoadObjFile(FilenameSrc, MeshStream, Resfile.Strings);
	}
	else if (ExtResult == EXT_TEXTURE)
	{
		TStream& TextureStream = Resfile.GetChunk(ECL_TEXTURES);
		TResTextureHelper::LoadDdsFile(FilenameSrc, TextureStream, Resfile.Strings);
	}
	else if (ExtResult == EXT_MATERIAL)
	{
		TStream& MaterialStream = Resfile.GetChunk(ECL_MATERIAL);

		FilenameSrc = "Material.input";
		FilenameDst = "Material.tix";

		TResMaterialHelper Helper;
		Helper.SetShaderName(ESS_VERTEX_SHADER, "SampleVertexShader.cso");
		Helper.SetShaderName(ESS_PIXEL_SHADER, "SamplePixelShader.cso");
		Helper.SetBlendMode(TMaterial::MATERIAL_BLEND_OPAQUE);
		Helper.SetShaderVsFormat((EVSSEG_POSITION | EVSSEG_NORMAL | EVSSEG_TEXCOORD0 | EVSSEG_TANGENT));
		Helper.EnableDepthWrite(true);
		Helper.EnableDepthTest(true);
		Helper.EnableTwoSides(false);

		Helper.OutputMaterial(MaterialStream, Resfile.Strings);
	}
	else if (ExtResult == EXT_MATERIAL_INSTANCE)
	{
		TStream& MIStream = Resfile.GetChunk(ECL_MATERIAL_INSTANCE);
		TResMaterialInstanceHelper Helper;

		FilenameSrc = "MaterialInstanceTest.input";
		FilenameDst = "MaterialInstanceTest.tix";

		Helper.SetMaterialInstanceName("MaterialInstanceTest");
		Helper.SetMaterialRes("TestMaterial");
		Helper.AddParameter("TestParam", 1.f);
		Helper.AddParameter("TestParamVec", vector3df(1, 2, 3));
		Helper.AddParameter("TestTexture", "Test.texture");

		Helper.OutputMaterialInstance(MIStream, Resfile.Strings);
	}
	else
	{
		printf("Error: Unknown file extension: %s\n", FilenameSrc.substr(FilenameSrc.rfind('.')).c_str());
	}

	Resfile.SaveFile(FilenameDst);

    return 0;
}

