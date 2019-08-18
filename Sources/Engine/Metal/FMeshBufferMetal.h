/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

#if COMPILE_WITH_RHI_METAL

namespace tix
{
	class FMeshBufferMetal : public FMeshBuffer
	{
	public:
		FMeshBufferMetal();
		virtual ~FMeshBufferMetal();
	protected:

	private:
        id<MTLBuffer> VertexBuffer;
        id<MTLBuffer> IndexBuffer;
		friend class FRHIMetal;
	};
    
    /////////////////////////////////////////////////////////////
    
    class FInstanceBufferMetal : public FInstanceBuffer
    {
    public:
        FInstanceBufferMetal();
        virtual ~FInstanceBufferMetal();
    protected:
        
    private:
        id<MTLBuffer> InstanceBuffer;
        friend class FRHIMetal;
    };
}

#endif	// COMPILE_WITH_RHI_METAL
