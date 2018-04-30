/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class FRenderer;
	class FRHI;

#define RENDERTHREAD_TASK_FUNCTION(Code) \
	virtual void Execute() override \
	{ \
		FRenderThread * RenderThread = TEngine::Get()->GetRenderThread(); \
		FRHI * RHI = RenderThread->GetRHI(); \
		Code; \
	}

#define ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER_DECLARE(TypeName, ParamType1, ParamName1, ParamValue1, Code) \
	class FRTTask_##TypeName : public TTask \
	{ \
	public: \
		FRTTask_##TypeName(ParamType1 In##ParamName1) : ParamName1(In##ParamName1) {} \
		RENDERTHREAD_TASK_FUNCTION(Code) \
	private: \
		ParamType1 ParamName1; \
	};

#define ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER_CREATE(TypeName, ParamType1, ParamValue1) \
	FRTTask_##TypeName * Task##TypeName = ti_new FRTTask_##TypeName(ParamValue1); \
	TEngine::Get()->GetRenderThread()->AddTask(Task##TypeName);

/* Add a task to run in Render thread with ONE parameter.
 * RenderThread is built in parameter.
 * RHI is built in parameter.
 */
#define ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(TypeName, ParamType1, ParamName1, ParamValue1, Code) \
	ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER_DECLARE(TypeName, ParamType1, ParamName1, ParamValue1, Code) \
	ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER_CREATE(TypeName, ParamType1, ParamValue1)

	class FRenderThread : public TTaskThread
	{
	public: 
		FRenderThread();
		virtual ~FRenderThread();

		FRHI * GetRHI()
		{
			return RHI;
		}

		virtual void Run() override;
		virtual void OnThreadStart() override;
		virtual void OnThreadEnd() override;

		void AddRenderer(FRenderer* Renderer);

	protected:
		void CreateRenderComponents();
		void DestroyRenderComponents();

	protected:
		FRHI * RHI;
		TVector<FRenderer*> Renderers;
	};
}
