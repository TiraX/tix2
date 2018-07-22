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
#include "TResource.h"
#include "TTexture.h"
#include "TMesh.h"
#include "TResfileDef.h"
#include "TResFile.h"
#include "TNode.h"
#include "TNodeCamera.h"
#include "TNodeStaticMesh.h"
#include "TScene.h"

#include "FRenderResource.h"
#include "FMeshBuffer.h"
#include "FMeshRelevance.h"
#include "FShader.h"
#include "FTexture.h"
#include "FPipelineState.h"
#include "FRenderer.h"
#include "FFrameResources.h"
#include "FRHI.h"
#include "FNode.h"
#include "FNodeStaticMesh.h"
#include "FScene.h"
#include "FRenderThread.h"

#include "TResourceLibrary.h"
#include "TEngineConfig.h"
#include "TEngine.h"

using namespace tix;