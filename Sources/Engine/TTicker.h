/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class TTicker
	{
	public:
        TTicker() {}
        virtual ~TTicker() {}
        
		virtual void Tick(float Dt) = 0;
	};
}
