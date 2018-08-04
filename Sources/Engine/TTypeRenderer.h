/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	enum E_FRAMEBUFFER
	{
		EFB_COLOR		= 0,
		EFB_DEPTH,
		EFB_STENCIL,
	};

	enum E_BUFFER_USAGE
	{
		EBU_STREAM_DRAW,
		EBU_STATIC_DRAW,
		EBU_DYNAMIC_DRAW,
	};

	//!
	enum E_FRAMEBUFFER_MASK
	{
		EFB_COLOR_MASK		= 1 << EFB_COLOR,
		EFB_DEPTH_MASK		= 1 << EFB_DEPTH,
		EFB_STENCIL_MASK	= 1 << EFB_STENCIL,
	};
	
	enum E_TRANSFORMATION_STATE
	{
		//! View transformation
		ETS_VIEW = 0,
		//! World transformation
		ETS_WORLD,
		//! Projection transformation
		ETS_PROJECTION,
		//! Not used
		ETS_COUNT
	};


	enum E_EFFECT_COMMON_TYPE
	{
		EECT_NON_TEXTURED_SOLID,
		EECT_TEXTURED_SOLID,
		EECT_TEXTURED_TRANSPARENT,
		EECT_TEXTURED_ADD,
		EECT_COLOR_TRANSPARENT,
		EECT_TEXTURED_2D,
		EECT_FONT,

		EECT_COUNT,
	};

}