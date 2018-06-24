// MeshConverter.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ResMeshHelper.h"

// Test target: ../../Content/head.obj

int main(int argc, _TCHAR* argv[])
{
	if (argc < 2)
	{
		return 0;
	}

	TStream OutStream;
	TVector<TString> Strings;
	TResMeshHelper::LoadObjFile(argv[1], OutStream, Strings);

    return 0;
}

