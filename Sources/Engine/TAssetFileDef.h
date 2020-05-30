/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	enum
	{
		TIRES_VERSION_MAINFILE = 1,
		TIRES_VERSION_CHUNK_MESH = 1,
		TIRES_VERSION_CHUNK_TEXTURE = 1,
		TIRES_VERSION_CHUNK_MATERIAL = 1,
		TIRES_VERSION_CHUNK_MINSTANCE = 1,
		TIRES_VERSION_CHUNK_SCENE = 1,
		TIRES_VERSION_CHUNK_SCENETILE = 1,
		//TIRES_VERSION_CHUNK_ANIM	= 2,	// add morph animation support
		//TIRES_VERSION_CHUNK_CTRL	= 2,	// add morph controller support
		//TIRES_VERSION_CHUNK_LIT		= 1,
		//TIRES_VERSION_CHUNK_CAM		= 1,
		//TIRES_VERSION_CHUNK_SHAD	= 5,	
		// in v4: add stencil options, add support for metal shader functions
		// in v5: add flag options for OpenGLES 3.0 shader
		//TIRES_VERSION_CHUNK_MTRL	= 1,
		//TIRES_VERSION_CHUNK_EMIT	= 14,	
		// in v14: add particle frame animation interval
		// add particle default fade time; add Particle Color Boost, add Emitter's BBox, add emitter param 'EmitLife' & 'EmitDelay', and GEOMETRY particle type, add Particle Life random and Particle Size random
		//TIRES_VERSION_CHUNK_TIMT	= 5,
		// in v2: add material set, each model can have max to 5 material sets
		// in v3: add bind lights for material. to bind specified lights by names
		// in v4: add material texture sample method
		// in v5: add material flag for pbr materials
		//TIRES_VERSION_CHUNK_TIUI	= 20,	
		// in v14: add scroll direction for UiScrollArea
		// add anim flag in AnimDesc; add a new symbol: event_mask; upgrade animation version; add default sfx and button sfx; add node alignment by viewport; add FAN ui node
		// in v15: add text gradient in ui fonts
		// in v16: add multi lines count limitation
		// in v17: add dialog 9 symbol
		// in v18: add text offset in easy button
		// in v19: add Camera info for draw box to render extra geometries; add child width and height for scroll area; add input box flag for text box
		// in v20: re-factor node flags for more usage

		//TIRES_VERSION_CHUNK_RES		= 1,
	};

	enum 
	{
		TIRES_ID_RESFILE			= TI_MAKE_IDENTIFIER('T', 'I', 'R', 'S'),
		TIRES_ID_CHUNK_MESH			= TI_MAKE_IDENTIFIER('M', 'E', 'S', 'H'),
		TIRES_ID_CHUNK_TEXTURE		= TI_MAKE_IDENTIFIER('T', 'E', 'X', 'E'),
		TIRES_ID_CHUNK_MATERIAL		= TI_MAKE_IDENTIFIER('M', 'A', 'T', 'R'),
		TIRES_ID_CHUNK_MINSTANCE	= TI_MAKE_IDENTIFIER('M', 'A', 'T', 'I'),
		TIRES_ID_CHUNK_SCENE		= TI_MAKE_IDENTIFIER('S', 'C', 'E', 'N'),
		TIRES_ID_CHUNK_SCENETILE	= TI_MAKE_IDENTIFIER('S', 'T', 'I', 'L'),
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
		ECL_MESHES,
		ECL_TEXTURES,
		ECL_MATERIAL,
		ECL_MATERIAL_INSTANCE,
		ECL_SCENE,

		// Contain mesh instances information
		ECL_SCENETILE,

		//ECL_MATERIALS,
		//ECL_CONTROLLERS,
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
		uint32 VertexFormat;
		int32 VertexCount;
		int32 PrimitiveCount;	//triangle count, means (index count) / 3. (triangle list)
		int32 IndexType;
		int32 Sections;
		uint16 Clusters, ClusterSize;
		uint32 Flag;
		aabbox3df BBox;

		enum
		{
			FLAG_MORPH_SOURCE	= 1 << 0,
			FLAG_MORPH_TARGET	= 1 << 1,
		};

		THeaderMesh()
			: VertexFormat(0)
			, VertexCount(0)
			, PrimitiveCount(0)
			, IndexType(0)
			, Sections(0)
			, Clusters(0)
			, ClusterSize(0)
			, Flag(0)
		{
		}
	};

	struct THeaderMeshSection
	{
		int32 StrId_Name;
		int32 StrMaterialInstance;
		uint32 IndexStart;
		uint32 Triangles;

		THeaderMeshSection()
			: StrId_Name(0)
			, StrMaterialInstance(0)
			, IndexStart(0)
			, Triangles(0)
		{}
	};

	struct TMeshClusterDef
	{
		aabbox3df BBox;
		vector4df Cone;
	};

	struct THeaderCollisionSet
	{
		uint32 NumSpheres;
		uint32 NumBoxes;
		uint32 NumCapsules;
		uint32 NumConvexes;
		uint32 SpheresSizeInBytes;
		uint32 BoxesSizeInBytes;
		uint32 CapsulesSizeInBytes;
		uint32 ConvexesSizeInBytes;

		THeaderCollisionSet()
			: NumSpheres(0)
			, NumBoxes(0)
			, NumCapsules(0)
			, NumConvexes(0)
			, SpheresSizeInBytes(0)
			, BoxesSizeInBytes(0)
			, CapsulesSizeInBytes(0)
			, ConvexesSizeInBytes(0)
		{}
	};

	struct THeaderTexture
	{
		int32 StrId_Name;
		int32 Format;
		int32 Width;
		int32 Height;

		uint8 Type;
		uint8 AddressMode;
		uint8 SRGB;
		uint8 Mips;
		uint8 Surfaces;
		uint8 padding[3];
	};

	struct THeaderMaterial
	{
		int32 ShaderNames[ESS_COUNT];
		int32 ShaderCodeLength[ESS_COUNT];

		uint32 Flags;
		uint32 BlendMode;
		TBlendState BlendState;
		TRasterizerDesc RasterizerDesc;
		TDepthStencilDesc DepthStencilDesc;
		uint32 VsFormat;
		uint32 InsFormat;

		uint8 ColorBuffers[ERTC_COUNT];
		int32 DepthBuffer;
	};

	struct THeaderMaterialInstance
	{
		int32 NameIndex;
		int32 LinkedMaterialIndex;
		int32 ParamDataCount;
		int32 ParamTextureCount;
	};

	struct THeaderShaderBinding
	{
		int32 BindingCount;
		uint32 BindingCrc;

		THeaderShaderBinding()
			: BindingCount(0)
			, BindingCrc(0)
		{}
	};

	struct THeaderScene
	{
		int32 NameIndex;

		// Environment
		vector3df MainLightDirection;
		SColorf MainLightColor;
		float MainLightIntensity;

		// Camera Info
		int32 NumCameras;
		
		// Scene Tiles
		int32 NumTiles;

		// VT Info
		int32 VTSize;
		int32 PageSize;
	};

	struct TVTRegionInfo
	{
		union
		{
			struct 
			{
				int32 X : 10;
				int32 Y : 10;
				int32 W : 6;
				int32 H : 6;

			};
			uint32 Value;
		};
	};

	struct THeaderCameraInfo
	{
		vector3df Location;
		vector3df Target;
		float FOV;
		vector3df Rotate;	// Pitch, Yaw, Roll in degrees
		float Aspect;
	};

	struct THeaderSceneTile
	{
		int32 LevelNameIndex;
		vector2di16 Position;
		aabbox3df BBox;

		// Dependency num
		int32 NumTextures;
		int32 NumMaterials;
		int32 NumMaterialInstances;
		int32 NumMeshes;
		int32 NumMeshSections;
		int32 NumInstances;
	};

	struct THeaderSceneMeshInstance
	{
		vector3df Position;
		quaternion Rotation;
		vector3df Scale;
	};
}