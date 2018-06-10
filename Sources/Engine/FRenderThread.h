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
		FRenderThread * RenderThread = FRenderThread::Get(); \
		FRHI * RHI = FRHI::Get(); \
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
	FRenderThread::Get()->AddTaskToFrame(Task##TypeName);


#define ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER_DECLARE(TypeName, ParamType1, ParamName1, ParamValue1, ParamType2, ParamName2, ParamValue2, Code) \
	class FRTTask_##TypeName : public TTask \
	{ \
	public: \
		FRTTask_##TypeName(ParamType1 In##ParamName1, ParamType2 In##ParamName2) : ParamName1(In##ParamName1), ParamName2(In##ParamName2) {} \
		RENDERTHREAD_TASK_FUNCTION(Code) \
	private: \
		ParamType1 ParamName1; \
		ParamType2 ParamName2; \
	};

#define ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER_CREATE(TypeName, ParamType1, ParamValue1, ParamType2, ParamValue2) \
	FRTTask_##TypeName * Task##TypeName = ti_new FRTTask_##TypeName(ParamValue1, ParamValue2); \
	FRenderThread::Get()->AddTaskToFrame(Task##TypeName);

/* Add a task to run in Render thread with ONE parameter.
 * RenderThread is built in parameter.
 * RHI is built in parameter.
 */
#define ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(TypeName, ParamType1, ParamName1, ParamValue1, Code) \
	ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER_DECLARE(TypeName, ParamType1, ParamName1, ParamValue1, Code) \
	ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER_CREATE(TypeName, ParamType1, ParamValue1)

#define ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(TypeName, ParamType1, ParamName1, ParamValue1, ParamType2, ParamName2, ParamValue2, Code) \
	ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER_DECLARE(TypeName, ParamType1, ParamName1, ParamValue1, ParamType2, ParamName2, ParamValue2, Code) \
	ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER_CREATE(TypeName, ParamType1, ParamValue1, ParamType2, ParamValue2)

	struct FRenderFrame
	{
		typedef TThreadSafeQueue<TTask*> TTaskQueue;
		TTaskQueue FrameTasks;
	};

	class FRenderThread : public TThread
	{
	public:
		static void CreateRenderThread();
		static void DestroyRenderThread();
		static FRenderThread* Get();

		// Functions run on game thread.
		virtual void Stop() override;
		void TriggerRender();
		void TriggerRenderAndStop();
		int32 GetTriggerNum() const
		{
			return TriggerNum;
		}
		void AddTaskToFrame(TTask* Task);

		// Functions run on render thread
		virtual void Run() override;
		virtual void OnThreadStart() override;
		virtual void OnThreadEnd() override;

		void AddRenderer(FRenderer* Renderer);

		FScene* GetRenderScene()
		{
			return RenderScene;
		}

	private:
		static FRenderThread* RenderThread;
		FRenderThread();
		virtual ~FRenderThread();

	protected:
		void CreateRenderComponents();
		void DestroyRenderComponents();

		void WaitForRenderSignal();
		void DoRenderTasks();

	protected:
		// Variables for synchronize with game thread.
		int32 TriggerNum;
		TMutex RenderMutex;
		TCond RenderCond;

		// Frame Index that is filling by game thread, operate in game thread
		int32 PreFrameIndex;
		// Frame Index that is rendering
		int32 RenderFrameIndex;
		FRenderFrame RenderFrames[FRHI::FrameBufferNum];

		// Render components
		FRHI * RHI;
		TVector<FRenderer*> Renderers;
		FScene * RenderScene;
	};
}
