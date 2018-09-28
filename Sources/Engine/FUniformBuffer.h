/*
	TiX Engine v2.0 Copyright (C) 2018
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
#define DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER_EXPLICIT(MemberType,MemberName,ArrayDecl,Precision) MemberType MemberName##ArrayDecl;

#define DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(MemberType,MemberName) DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER_EXPLICIT(MemberType,MemberName,,EShaderPrecisionModifier::Float)
#define DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER_EX(MemberType,MemberName,Precision) DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER_EXPLICIT(MemberType,MemberName,,Precision)
#define DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER_ARRAY(MemberType,MemberName,ArrayDecl) DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER_EXPLICIT(MemberType,MemberName,ArrayDecl,EShaderPrecisionModifier::Float)
#define DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER_ARRAY_EX(MemberType,MemberName,ArrayDecl,Precision) DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER_EXPLICIT(MemberType,MemberName,ArrayDecl,Precision)

	/** Begins a uniform buffer struct declaration. */
#define BEGIN_UNIFORM_BUFFER_STRUCT_EX(StructTypeName,ConstructorSuffix,UniformHeapType) \
	class StructTypeName : public IReferenceCounted \
	{ \
	public: \
		static const E_RENDER_RESOURCE_HEAP_TYPE UBHeap = UniformHeapType; \
		StructTypeName () ConstructorSuffix \
		struct FUniformBufferStruct \
		{ \

#define END_UNIFORM_BUFFER_STRUCT(StructTypeName) \
		}; \
		FUniformBufferStruct UniformBufferData; \
		FUniformBufferPtr UniformBuffer; \
		FUniformBufferPtr InitUniformBuffer() \
		{ \
			TI_ASSERT(IsRenderThread()); \
			FRHI * RHI = FRHI::Get(); \
			UniformBuffer = RHI->CreateUniformBuffer(StructTypeName::UBHeap); \
			RHI->UpdateHardwareResource(UniformBuffer, &UniformBufferData, sizeof(StructTypeName::FUniformBufferStruct)); \
			return UniformBuffer; \
		} \
	}; \
	typedef TI_INTRUSIVE_PTR(StructTypeName) StructTypeName##Ptr;

#define BEGIN_UNIFORM_BUFFER_STRUCT(StructTypeName,UniformHeapType) BEGIN_UNIFORM_BUFFER_STRUCT_EX(StructTypeName,{},UniformHeapType)

	class FUniformBuffer : public FRenderResourceInHeap
	{
	public:
		FUniformBuffer(E_RENDER_RESOURCE_HEAP_TYPE HeapType);
		virtual ~FUniformBuffer();

	protected:

	protected:
	};
}
