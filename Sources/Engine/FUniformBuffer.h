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
#define BEGIN_UNIFORM_BUFFER_STRUCT_EX(StructTypeName,ConstructorSuffix,ArrayElements) \
	class StructTypeName : public IReferenceCounted \
	{ \
	public: \
		static const uint32 Elements = ArrayElements; \
		StructTypeName () ConstructorSuffix \
		struct FUniformBufferStruct \
		{ \

#define END_UNIFORM_BUFFER_STRUCT(StructTypeName) \
		}; \
		uint32 GetElementsSize() const \
		{ \
			return StructTypeName::Elements; \
		} \
		uint32 GetStructureStrideInBytes() const \
		{ \
			return sizeof(StructTypeName::FUniformBufferStruct); \
		} \
		FUniformBufferStruct UniformBufferData[StructTypeName::Elements]; \
		FUniformBufferPtr UniformBuffer; \
		FUniformBufferPtr InitUniformBuffer() \
		{ \
			TI_ASSERT(IsRenderThread()); \
			FRHI * RHI = FRHI::Get(); \
			UniformBuffer = RHI->CreateUniformBuffer(sizeof(StructTypeName::FUniformBufferStruct), StructTypeName::Elements); \
			RHI->UpdateHardwareResource(UniformBuffer, UniformBufferData); \
			return UniformBuffer; \
		} \
	}; \
	typedef TI_INTRUSIVE_PTR(StructTypeName) StructTypeName##Ptr;

#define BEGIN_UNIFORM_BUFFER_STRUCT(StructTypeName) BEGIN_UNIFORM_BUFFER_STRUCT_EX(StructTypeName,{}, 1)
#define BEGIN_UNIFORM_BUFFER_STRUCT_ARRAY(StructTypeName, ArrayElements) BEGIN_UNIFORM_BUFFER_STRUCT_EX(StructTypeName,{}, ArrayElements)

	class FUniformBuffer : public FRenderResource
	{
	public:
		FUniformBuffer(uint32 InStructureSizeInBytes, uint32 InElements);
		virtual ~FUniformBuffer();

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
	protected:

	protected:
		uint32 StructureSizeInBytes;
		uint32 Elements;
	};
}
