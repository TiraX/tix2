/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class TResourceLibrary
	{
	public:
		TI_API static TResourceLibrary* Get();

		TI_API TResourceObjectPtr LoadResource(const TString& ResFilename);
		TI_API TResourceObjectPtr LoadResourceAysc(const TString& ResFilename);
		TI_API TResourceObjectPtr CreateShaderResource(const TShaderNames& ShaderNames);
		TI_API void LoadScene(const TString& ResFilename);

		TI_API void RemoveUnusedResources();

	private:
		static TResourceLibrary* s_instance;
		TResourceLibrary();
		virtual ~TResourceLibrary();

		void RemoveAllResources();

	private:
		typedef TMap< TString, TResourceObjectPtr >	MapResourceObjects;
		MapResourceObjects ResourceObjects;

		friend class TEngine;
	};
}