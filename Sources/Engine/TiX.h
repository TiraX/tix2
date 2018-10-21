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
#include <unordered_map>
#include <algorithm>
#include <assert.h>
#include <thread>
#include <mutex>
#include <condition_variable>

using namespace std;

// Disable std container warnings in DLL export
#pragma warning( disable : 4251 )

// Include tix symbols
// Basic define and types
#include "TConfig.h"
#include "TDefines.h"
#include "IInstrusivePtr.hpp"
#include "IReferenceCounted.h"
#include "TTypes.h"
#include "TPtrTypes.h"
#include "TStringExt.h"
#include "TMath.h"
#include "TUtils.h"

// Game thread components
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
#include "TNodeLight.h"
#include "TScene.h"

// RHI things
#include "FRenderResource.h"
#include "FRenderResourceTable.h"
#include "FMeshBuffer.h"
#include "FShader.h"
#include "FShaderBinding.h"
#include "FTexture.h"
#include "FPipeline.h"
#include "FUniformBuffer.h"
#include "FRenderTarget.h"
#include "FFrameResources.h"
#include "FRHI.h"

// Render thread things
#include "FPrimitive.h"
#include "FLight.h"
#include "FSceneLights.h"
#include "FScene.h"
#include "FRenderer.h"
#include "FDefaultRenderer.h"
#include "FFullScreenRender.h"
#include "FRenderThread.h"

#include "TResourceLibrary.h"
#include "TEngineDesc.h"
#include "TConsoleVariable.h"
#include "TPath.h"
#include "TEngine.h"

using namespace tix;