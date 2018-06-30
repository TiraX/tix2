// MeshConverter.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ResHelper.h"
#include "ResMeshHelper.h"

// Test target: ../../Content/head.obj

int main(int argc, _TCHAR* argv[])
{
	if (argc < 2)
	{
		return 0;
	}

	TResFileHelper Resfile;
	TStream& MeshStream = Resfile.GetChunk(ECL_MESHES);
	TResMeshHelper::LoadObjFile(argv[1], MeshStream, Resfile.Strings);

	Resfile.SaveFile("head.tix");

    return 0;
}

