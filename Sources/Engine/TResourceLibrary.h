/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class TiRefRes : public IReferenceCounted
	{
	public:
		TiRefRes();
		virtual ~TiRefRes();

#if defined (TIX_DEBUG)
		TString ResourceFilename;
#endif
	};

	//////////////////////////////////////////////////////////////////////////

	class TResourceLibrary
	{
	public:
		TResourceLibrary();
		virtual ~TResourceLibrary();

		TI_API virtual void RemoveUnusedResouces() {};

	protected:
		typedef TMap< TString, TResourcePtr >	MapResources;
		MapResources Resources;
	};
}