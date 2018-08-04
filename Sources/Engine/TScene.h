/*
TiX Engine v2.0 Copyright (C) 2018
By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	enum E_STAGE_LIST_TYPE
	{
		ESLT_SOLID,
		ESLT_TRANSPARENT,
		ESLT_PARTICLES,

		ESLT_LIGHTS,

		ESLT_UI_3D,

		ESLT_SKYBOX,

		ESLT_COUNT,
	};

	enum E_STAGE_FLAG
	{
		STAGE_FLAG_RESERVED = 1 << 0,
		STAGE_PAUSE_UPDATE_ANIM = 1 << 1,
		STAGE_CAMERA_DIRTY = 1 << 2,
	};

	class TNodeSceneRoot
		: public TNode
	{
		DECLARE_NODE_WITH_CONSTRUCTOR(SceneRoot);
	public:
		virtual ~TNodeSceneRoot();

		virtual void CreateRenderThreadNode() override;
	};

	class TNodeCamera;

	class TScene
	{
	public:
		TScene();
		virtual ~TScene();

		TI_API void TickAllNodes(float dt, TNode* root = nullptr);

		void SetStageFlag(E_STAGE_FLAG flag, bool enable)
		{
			if (enable)
				Flag |= flag;
			else
				Flag &= ~flag;
		}
		bool IsEnabled(E_STAGE_FLAG flag)
		{
			return (Flag & flag) != 0;
		}

		TNodeSceneRoot* GetRoot()
		{
			return NodeRoot;
		}

		TI_API void SetActiveCamera(TNodeCamera* camera);
		TI_API TNodeCamera* GetActiveCamera();

		// Temp method to test rendering, scene management will be in Scene Loading
		TI_API void AddMeshToScene(TMeshBufferPtr InMesh, TPipelinePtr InPipeline, int32 InMaterialInFuture);

	protected:


	protected:
		TNodeSceneRoot * NodeRoot;
		uint32 Flag;

		TNodeCamera* DefaultCamera;
		TNodeCamera* ActiveCamera;
	};

} // end namespace tix
