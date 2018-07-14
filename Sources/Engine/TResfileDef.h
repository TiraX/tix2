/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	enum 
	{
		TIRES_VERSION_MAINFILE		= 1,
		TIRES_VERSION_CHUNK_MESH	= 1,
		TIRES_VERSION_CHUNK_SCEN	= 4,	// add TiMaterial in DAE converting, remove Material, use default TiMaterial instead;added trigger node
		TIRES_VERSION_CHUNK_IMAG	= 1,
		TIRES_VERSION_CHUNK_ANIM	= 2,	// add morph animation support
		TIRES_VERSION_CHUNK_CTRL	= 2,	// add morph controller support
		TIRES_VERSION_CHUNK_LIT		= 1,
		TIRES_VERSION_CHUNK_CAM		= 1,
		TIRES_VERSION_CHUNK_SHAD	= 5,	
		// in v4: add stencil options, add support for metal shader functions
		// in v5: add flag options for OpenGLES 3.0 shader
		TIRES_VERSION_CHUNK_MTRL	= 1,
		TIRES_VERSION_CHUNK_EMIT	= 14,	
		// in v14: add particle frame animation interval
		// add particle default fade time; add Particle Color Boost, add Emitter's BBox, add emitter param 'EmitLife' & 'EmitDelay', and GEOMETRY particle type, add Particle Life random and Particle Size random
		TIRES_VERSION_CHUNK_TIMT	= 5,
		// in v2: add material set, each model can have max to 5 material sets
		// in v3: add bind lights for material. to bind specified lights by names
		// in v4: add material texture sample method
		// in v5: add material flag for pbr materials
		TIRES_VERSION_CHUNK_TIUI	= 20,	
		// in v14: add scroll direction for UiScrollArea
		// add anim flag in AnimDesc; add a new symbol: event_mask; upgrade animation version; add default sfx and button sfx; add node alignment by viewport; add FAN ui node
		// in v15: add text gradient in ui fonts
		// in v16: add multi lines count limitation
		// in v17: add dialog 9 symbol
		// in v18: add text offset in easy button
		// in v19: add Camera info for draw box to render extra geometries; add child width and height for scroll area; add input box flag for text box
		// in v20: re-factor node flags for more usage

		TIRES_VERSION_CHUNK_RES		= 1,
	};

	enum 
	{
		TIRES_ID_RESFILE			= TI_MAKE_IDENTIFIER('T', 'I', 'R', 'S'),
		TIRES_ID_CHUNK_MESH			= TI_MAKE_IDENTIFIER('M', 'E', 'S', 'H'),
		//TIRES_ID_CHUNK_SCENE		= TI_MAKE_IDENTIFIER('S', 'C', 'E', 'N'),
		//TIRES_ID_CHUNK_IMAGES		= TI_MAKE_IDENTIFIER('I', 'M', 'A', 'G'),
		//TIRES_ID_CHUNK_ANIMS		= TI_MAKE_IDENTIFIER('A', 'N', 'I', 'M'),
		//TIRES_ID_CHUNK_CONTROLLER	= TI_MAKE_IDENTIFIER('C', 'T', 'R', 'L'),
		//TIRES_ID_CHUNK_LIGHT		= TI_MAKE_IDENTIFIER('L', 'I', 'T', '.'),
		//TIRES_ID_CHUNK_CAMERA		= TI_MAKE_IDENTIFIER('C', 'A', 'M', 'E'), 
		//TIRES_ID_CHUNK_SHADER		= TI_MAKE_IDENTIFIER('S', 'H', 'A', 'D'), 
		//TIRES_ID_CHUNK_MATERIAL		= TI_MAKE_IDENTIFIER('M', 'T', 'R', 'L'), 
		//TIRES_ID_CHUNK_TIMATERIAL	= TI_MAKE_IDENTIFIER('T', 'I', 'M', 'T'), 
		//TIRES_ID_CHUNK_EMITTER		= TI_MAKE_IDENTIFIER('E', 'M', 'I', 'T'), 
		//TIRES_ID_CHUNK_UI			= TI_MAKE_IDENTIFIER('T', 'I', 'U', 'I'), 
		//TIRES_ID_CHUNK_STRING		= TI_MAKE_IDENTIFIER('S', 'T', 'R', '.'), 
		//TIRES_ID_CHUNK_RESOURCES	= TI_MAKE_IDENTIFIER('R', 'E', 'S', '.'), 
		//TIRES_ID_TILEZONE			= TI_MAKE_IDENTIFIER('Z', 'O', 'N', 'E'), 
	};

	enum E_CHUNK_LIB
	{
		//ECL_MATERIALS,
		//ECL_CONTROLLERS,
		ECL_MESHES,
		ECL_TEXTURES,
		//ECL_SHADERS,
		//ECL_ANIMATIONS,
		//ECL_PARTICLEEFFECT,
		//ECL_TIMATERIAL,
		//ECL_UI,
		//ECL_VISUALSCENE,

		//ECL_RESOURCES,	// this is the last one

		ECL_COUNT,
	};
	
	struct TResfileHeader
	{
		union 
		{
			uint32 ID;
			int8 IDNAME[4];
		};
		int32 Version;
		int32 ChunkCount;
		int32 FileSize;
		int32 StringCount;
		int32 StringOffset;

		TResfileHeader()
			: ID(TIRES_ID_RESFILE)
			, Version(TIRES_VERSION_MAINFILE)
			, ChunkCount(0)
			, FileSize(0)
			, StringCount(0)
			, StringOffset(0)
		{}
	};

	struct TResfileChunkHeader
	{
		union 
		{
			uint32 ID;
			int8 IDNAME[4];
		};
		int32 Version;
		int32 ChunkSize;
		int32 ElementCount;

		TResfileChunkHeader()
			: ID(0)
			, Version(0)
			, ChunkSize(0)
			, ElementCount(0)
		{}
	};

	struct THeaderMesh
	{
		int32 StrId_Name;
		uint32 VertexFormat;
		int32 VertexCount;
		int32 PrimitiveCount;	//triangle count, means (index count) / 3. (triangle list)
		int32 IndexType;
		uint32 Flag;
		aabbox3df BBox;

		enum
		{
			FLAG_MORPH_SOURCE	= 1 << 0,
			FLAG_MORPH_TARGET	= 1 << 1,
		};

		THeaderMesh()
			: StrId_Name(-1)
			, VertexFormat(0)
			, VertexCount(0)
			, PrimitiveCount(0)
			, IndexType(0)
			, Flag(0)
		{
		}
	};
}