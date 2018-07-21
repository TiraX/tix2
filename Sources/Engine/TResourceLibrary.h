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
		TI_API void RemoveUnusedResouces();

	private:
		static TResourceLibrary* s_instance;
		TResourceLibrary();
		virtual ~TResourceLibrary();

	private:
		typedef TMap< TString, TResourcePtr >	MapResources;
		MapResources Resources;

		friend class TEngine;
	};
}