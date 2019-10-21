/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	namespace EShaderPrecisionModifier
	{
		enum Type
		{
			Float,
			Half,
			Fixed
		};
	};

	// Macros for declaring uniform buffer structures.

/** Declares a member of a uniform buffer struct. */
#define DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER_EXPLICIT(MemberType,MemberName,ArrayDecl,Precision) \
    typedef MemberType zzA##MemberName ArrayDecl; \
    zzA##MemberName MemberName;

#define DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(MemberType,MemberName) DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER_EXPLICIT(MemberType,MemberName,,EShaderPrecisionModifier::Float)
#define DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER_EX(MemberType,MemberName,Precision) DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER_EXPLICIT(MemberType,MemberName,,Precision)
#define DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER_ARRAY(MemberType,MemberName,ArrayDecl) DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER_EXPLICIT(MemberType,MemberName,ArrayDecl,EShaderPrecisionModifier::Float)
#define DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER_ARRAY_EX(MemberType,MemberName,ArrayDecl,Precision) DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER_EXPLICIT(MemberType,MemberName,ArrayDecl,Precision)

	/** Begins a uniform buffer struct declaration. */
#define BEGIN_UNIFORM_BUFFER_STRUCT_FIX_SIZE(StructTypeName,ArrayElements) \
	class StructTypeName : public IReferenceCounted \
	{ \
	public: \
		static const uint32 Elements = ArrayElements; \
		StructTypeName () { UniformBufferData.resize(StructTypeName::Elements); }\
		struct FUniformBufferStruct \
		{ \

#define BEGIN_UNIFORM_BUFFER_STRUCT_DYNAMIC_SIZE(StructTypeName) \
	class StructTypeName : public IReferenceCounted \
	{ \
	public: \
		StructTypeName (uint32 ElementSize) { UniformBufferData.resize(ElementSize); }\
		struct FUniformBufferStruct \
		{ \

#define END_UNIFORM_BUFFER_STRUCT(StructTypeName) \
		}; \
		uint32 GetElementsCount() const \
		{ \
			return (uint32)UniformBufferData.size(); \
		} \
		uint32 GetStructureStrideInBytes() const \
		{ \
			return sizeof(StructTypeName::FUniformBufferStruct); \
		} \
		TVector<FUniformBufferStruct> UniformBufferData; \
		void InitToZero() \
		{ \
			memset(UniformBufferData.data(), 0, GetStructureStrideInBytes() * GetElementsCount() ); \
		} \
		FUniformBufferPtr UniformBuffer; \
		FUniformBufferPtr InitUniformBuffer(uint32 UBFlag = 0) \
		{ \
			TI_ASSERT(IsRenderThread()); \
			FRHI * RHI = FRHI::Get(); \
			UniformBuffer = RHI->CreateUniformBuffer(sizeof(StructTypeName::FUniformBufferStruct), GetElementsCount(), UBFlag); \
			RHI->UpdateHardwareResourceUB(UniformBuffer, UniformBufferData.data()); \
			return UniformBuffer; \
		} \
	}; \
	typedef TI_INTRUSIVE_PTR(StructTypeName) StructTypeName##Ptr;

#define BEGIN_UNIFORM_BUFFER_STRUCT(StructTypeName) BEGIN_UNIFORM_BUFFER_STRUCT_FIX_SIZE(StructTypeName,1)
#define BEGIN_UNIFORM_BUFFER_STRUCT_ARRAY(StructTypeName, ArrayElements) BEGIN_UNIFORM_BUFFER_STRUCT_FIX_SIZE(StructTypeName, ArrayElements)
#define BEGIN_UNIFORM_BUFFER_STRUCT_ARRAY_DYNAMIC(StructTypeName) BEGIN_UNIFORM_BUFFER_STRUCT_DYNAMIC_SIZE(StructTypeName)
	
	enum E_UNIFORMBUFFER_FLAG
	{
		UB_FLAG_COMPUTE_WRITABLE = 1 << 0,
		UB_FLAG_COMPUTE_WITH_COUNTER = 1 << 1,
		UB_FLAG_READBACK = 1 << 2,

		// For metal, do not create buffer (usually less than 4k bytes), bind raw memory to gpu as apple doc recommended.
		// https://developer.apple.com/library/archive/documentation/3DDrawing/Conceptual/MTLBestPracticesGuide/BufferBindings.html
		// For Dx12, use a D3D12_HEAP_TYPE_UPLOAD heap to manage data directly
		UB_FLAG_INTERMEDIATE = 1 << 3,	

		// For Dx12, transition the resource state to D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT
		UB_FLAG_GPU_COMMAND_BUFFER = 1 << 4,
		// For Dx12, transition the resource state to D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE
		UB_FLAG_GPU_COMMAND_BUFFER_RESOURCE = 1 << 5,
	};
	class FUniformBuffer : public FRenderResource
	{
	public:
		FUniformBuffer(uint32 InStructureSizeInBytes, uint32 InElements, uint32 InUBFlag);
		virtual ~FUniformBuffer();

		virtual TStreamPtr ReadBufferData() { return nullptr; }

		uint32 GetTotalBufferSize() const
		{
			return StructureSizeInBytes * Elements;
		}
		uint32 GetStructureSizeInBytes() const
		{
			return StructureSizeInBytes;
		}
		uint32 GetElements() const
		{
			return Elements;
		}
		uint32 GetFlag() const
		{
			return Flag;
		}
		virtual uint32 GetCounterOffset() const
		{
			TI_ASSERT(0);
			return 0;
		}
	protected:

	protected:
		uint32 StructureSizeInBytes;
		uint32 Elements;
		uint32 Flag;
	};
}
