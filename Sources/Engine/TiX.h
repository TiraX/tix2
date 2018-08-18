/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

// Include std symbols
#include <stdlib.h>
#include <string>
#include <list>
#include <vector>
#include <map>
#include <algorithm>
#include <assert.h>
#include <thread>
#include <mutex>
#include <condition_variable>

using namespace std;

// Disable std container warnings in DLL export
#pragma warning( disable : 4251 )  

// Include tix symbols
#include "TConfig.h"
#include "TDefines.h"
#include "IInstrusivePtr.hpp"
#include "IReferenceCounted.h"
#include "TTypes.h"
#include "TMath.h"
#include "TUtils.h"
#include "TThread.h"
#include "TQueue.h"
#include "TTaskThread.h"
#include "TInput.h"
#include "TDevice.h"
#include "TLog.h"
#include "TTimer.h"
#include "TFile.h"
#include "TStream.h"
#include "TTicker.h"
#include "FRHIConfig.h"
#include "TResource.h"
#include "TTexture.h"
#include "TMeshBuffer.h"
#include "TPipeline.h"
#include "TMaterial.h"
#include "TMaterialInstance.h"
#include "TResfileDef.h"
#include "TResFile.h"
#include "TNode.h"
#include "TNodeCamera.h"
#include "TNodeCameraNav.h"
#include "TNodeStaticMesh.h"
#include "TScene.h"

#include "FRenderResource.h"
#include "FMeshBuffer.h"
#include "FShader.h"
#include "FTexture.h"
#include "FPipeline.h"
#include "FUniformBuffer.h"
#include "FMeshRelevance.h"
#include "FFrameResources.h"
#include "FRHI.h"
#include "FNode.h"
#include "FNodeStaticMesh.h"
#include "FScene.h"
#include "FRenderer.h"
#include "FDefaultRenderer.h"
#include "FRenderThread.h"

#include "TResourceLibrary.h"
#include "TEngineConfig.h"
#include "TEngine.h"

using namespace tix;