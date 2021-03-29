/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class FRenderer;
	class FRHI;

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
		TI_API static FRenderThread* Get();
		static bool IsInited();
		static uint32 GetFrameNum()
		{
			return FrameNum;
		}

		// Functions run on game thread.
		virtual void Stop() override;
		void TriggerRender();
		void TriggerRenderAndStop();
		int32 GetTriggerNum() const
		{
			return TriggerNum;
		}
		TI_API void AddTaskToFrame(TTask* Task);

		// Functions run on render thread
		virtual void Run() override;
		virtual void OnThreadStart() override;
		virtual void OnThreadEnd() override;

		void AssignRenderer(FRenderer* InRenderer);

		FScene* GetRenderScene()
		{
			return RenderScene;
		}

		FVTSystem* GetVTSystem()
		{
			return VTSystem;
		}

		float GetRenderThreadLiveTime() const
		{
			return RTLiveTime;
		}

		float GetRenderThreadFrameTime() const
		{
			return RTFrameTime;
		}

	private:
		static FRenderThread* RenderThread;
		FRenderThread();
		virtual ~FRenderThread();

		// Number of frames done in render thread.
		static uint32 FrameNum;

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

		// Time and Frame Time 
		uint64 LastRTFrameTime;
		float RTLiveTime;
		float RTFrameTime;

		// Frame Index that is filling by game thread, operate in game thread
		int32 PreFrameIndex;
		// Frame Index that is rendering (0~FRHIConfig::FrameBufferNum)
		int32 RenderFrameIndex;
		FRenderFrame RenderFrames[FRHIConfig::FrameBufferNum];

		// Render components
		FRHI * RHI;
		FRenderer* Renderer;

		FScene * RenderScene;
		FVTSystem * VTSystem;

		static bool Inited;
		static bool ThreadEnabled;
	};


	// From UE4.22
	template<typename TSTR, typename LAMBDA>
	class TEnqueueUniqueRenderCommandType : public TTask
	{
	public:
		TEnqueueUniqueRenderCommandType(LAMBDA&& InLambda) 
			: Lambda(tix_forward<LAMBDA>(InLambda)) 
		{
		}

		virtual void Execute()
		{
			Lambda();
		}

	private:
		LAMBDA Lambda;
	};

	template<typename TSTR, typename LAMBDA>
	inline void EnqueueUniqueRenderCommand(LAMBDA&& Lambda)
	{
		typedef TEnqueueUniqueRenderCommandType<TSTR, LAMBDA> EURCType;
		TTask* Task = ti_new EURCType(tix_forward<LAMBDA>(Lambda));
#if TIX_DEBUG_RENDER_TASK_NAME
		Task->SetTaskName(TSTR::CStr());
#endif
		FRenderThread::Get()->AddTaskToFrame(Task);
	}

#define ENQUEUE_RENDER_COMMAND(Type) \
		struct Type##Name \
		{  \
			static const char* CStr() { return #Type; } \
			static const TCHAR* TStr() { return TEXT(#Type); } \
		}; \
		EnqueueUniqueRenderCommand<Type##Name>

	template<typename LAMBDA>
	inline void EnqueueUniqueRenderCommand(LAMBDA& Lambda)
	{
		static_assert(sizeof(LAMBDA) == 0, "EnqueueUniqueRenderCommand enforces use of rvalue and therefore move to avoid an extra copy of the Lambda");
	}
}
