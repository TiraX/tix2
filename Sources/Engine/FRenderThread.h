/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class FRenderer;
	class FRHI;

#if TIX_DEBUG_RENDER_TASK_NAME
#	define SET_TASK_NAME(n) SetTaskName(n)
#else
#	define SET_TASK_NAME(n) 
#endif

#define RENDERTHREAD_TASK_FUNCTION(Code) \
	virtual void Execute() override \
	{ \
		Code; \
	}

	// No parameter
#define ENQUEUE_UNIQUE_RENDER_COMMAND_DECLARE(TypeName, Code) \
	class FRTTask_##TypeName : public TTask \
	{ \
	public: \
		FRTTask_##TypeName() {SET_TASK_NAME(#TypeName);} \
		RENDERTHREAD_TASK_FUNCTION(Code) \
	};

#define ENQUEUE_UNIQUE_RENDER_COMMAND_CREATE(TypeName) \
	FRTTask_##TypeName * Task##TypeName = ti_new FRTTask_##TypeName(); \
	FRenderThread::Get()->AddTaskToFrame(Task##TypeName);

	// One parameters
#define ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER_DECLARE(TypeName, ParamType1, ParamName1, ParamValue1, Code) \
	class FRTTask_##TypeName : public TTask \
	{ \
	public: \
		FRTTask_##TypeName(ParamType1 In##ParamName1) : ParamName1(In##ParamName1) {SET_TASK_NAME(#TypeName);} \
		RENDERTHREAD_TASK_FUNCTION(Code) \
	private: \
		ParamType1 ParamName1; \
	};

#define ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER_CREATE(TypeName, ParamType1, ParamValue1) \
	FRTTask_##TypeName * Task##TypeName = ti_new FRTTask_##TypeName(ParamValue1); \
	FRenderThread::Get()->AddTaskToFrame(Task##TypeName);

	// Two parameters
#define ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER_DECLARE(TypeName, ParamType1, ParamName1, ParamValue1, ParamType2, ParamName2, ParamValue2, Code) \
	class FRTTask_##TypeName : public TTask \
	{ \
	public: \
		FRTTask_##TypeName(ParamType1 In##ParamName1, ParamType2 In##ParamName2) : ParamName1(In##ParamName1), ParamName2(In##ParamName2) {SET_TASK_NAME(#TypeName);} \
		RENDERTHREAD_TASK_FUNCTION(Code) \
	private: \
		ParamType1 ParamName1; \
		ParamType2 ParamName2; \
	};

#define ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER_CREATE(TypeName, ParamType1, ParamValue1, ParamType2, ParamValue2) \
	FRTTask_##TypeName * Task##TypeName = ti_new FRTTask_##TypeName(ParamValue1, ParamValue2); \
	FRenderThread::Get()->AddTaskToFrame(Task##TypeName);

	// Three parameters
#define ENQUEUE_UNIQUE_RENDER_COMMAND_THREEPARAMETER_DECLARE(TypeName, ParamType1, ParamName1, ParamValue1, ParamType2, ParamName2, ParamValue2, ParamType3, ParamName3, ParamValue3, Code) \
	class FRTTask_##TypeName : public TTask \
	{ \
	public: \
		FRTTask_##TypeName(ParamType1 In##ParamName1, ParamType2 In##ParamName2, ParamType3 In##ParamName3) : ParamName1(In##ParamName1), ParamName2(In##ParamName2), ParamName3(In##ParamName3) {SET_TASK_NAME(#TypeName);} \
		RENDERTHREAD_TASK_FUNCTION(Code) \
	private: \
		ParamType1 ParamName1; \
		ParamType2 ParamName2; \
		ParamType3 ParamName3; \
	};

#define ENQUEUE_UNIQUE_RENDER_COMMAND_THREEPARAMETER_CREATE(TypeName, ParamType1, ParamValue1, ParamType2, ParamValue2, ParamType3, ParamName3, ParamValue3) \
	FRTTask_##TypeName * Task##TypeName = ti_new FRTTask_##TypeName(ParamValue1, ParamValue2, ParamValue3); \
	FRenderThread::Get()->AddTaskToFrame(Task##TypeName);

/* Add a task to run in Render thread with ONE parameter.
 * RenderThread is built in parameter.
 * RHI is built in parameter.
 */
#define ENQUEUE_UNIQUE_RENDER_COMMAND(TypeName, Code) \
	ENQUEUE_UNIQUE_RENDER_COMMAND_DECLARE(TypeName, Code) \
	ENQUEUE_UNIQUE_RENDER_COMMAND_CREATE(TypeName)

#define ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(TypeName, ParamType1, ParamName1, ParamValue1, Code) \
	ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER_DECLARE(TypeName, ParamType1, ParamName1, ParamValue1, Code) \
	ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER_CREATE(TypeName, ParamType1, ParamValue1)

#define ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(TypeName, ParamType1, ParamName1, ParamValue1, ParamType2, ParamName2, ParamValue2, Code) \
	ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER_DECLARE(TypeName, ParamType1, ParamName1, ParamValue1, ParamType2, ParamName2, ParamValue2, Code) \
	ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER_CREATE(TypeName, ParamType1, ParamValue1, ParamType2, ParamValue2)

#define ENQUEUE_UNIQUE_RENDER_COMMAND_THREEPARAMETER(TypeName, ParamType1, ParamName1, ParamValue1, ParamType2, ParamName2, ParamValue2, ParamType3, ParamName3, ParamValue3, Code) \
	ENQUEUE_UNIQUE_RENDER_COMMAND_THREEPARAMETER_DECLARE(TypeName, ParamType1, ParamName1, ParamValue1, ParamType2, ParamName2, ParamValue2, ParamType3, ParamName3, ParamValue3, Code) \
	ENQUEUE_UNIQUE_RENDER_COMMAND_THREEPARAMETER_CREATE(TypeName, ParamType1, ParamValue1, ParamType2, ParamValue2, ParamType3, ParamName3, ParamValue3)

	struct FRenderFrame
	{
		typedef TThreadSafeQueue<TTask*> TTaskQueue;
		TTaskQueue FrameTasks;
	};

	class FRenderThread : public TThread
	{
	public:
		static void CreateRenderThread(bool ForceDisableThread = false);
		static void DestroyRenderThread();
		static FRenderThread* Get();
		static bool IsInited();

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

		FVTSystem* GetVTSystem()
		{
			return VTSystem;
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
		FRenderFrame RenderFrames[FRHIConfig::FrameBufferNum];

		// Render components
		FRHI * RHI;
		TVector<FRenderer*> Renderers;

		FScene * RenderScene;
		FVTSystem * VTSystem;

		static bool Inited;
		static bool ThreadEnabled;
	};
}
