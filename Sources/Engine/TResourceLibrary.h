/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class TResourceLibrary
	{
	public:
		TI_API static TResourceLibrary* Get();

		TI_API TResourcePtr LoadResource(const TString& ResFilename);
		TI_API TResourcePtr CreateShaderResource(const TShaderNames& ShaderNames);

		TI_API void RemoveUnusedResources();

	private:
		static TResourceLibrary* s_instance;
		TResourceLibrary();
		virtual ~TResourceLibrary();

		void RemoveAllResources();

	private:
		typedef TMap< TString, TResourcePtr >	MapResources;
		MapResources Resources;

		friend class TEngine;
	};
}