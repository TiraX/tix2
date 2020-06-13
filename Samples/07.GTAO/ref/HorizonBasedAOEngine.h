//----------------------------------------------------------------------------------
// File:   HorizonBasedAOEngine.h
// Author: Miguel Sainz & Louis Bavoil
// Email:  sdkfeedback@nvidia.com
// 
// Copyright (c) 2007 NVIDIA Corporation. All rights reserved.
//
// TO  THE MAXIMUM  EXTENT PERMITTED  BY APPLICABLE  LAW, THIS SOFTWARE  IS PROVIDED
// *AS IS*  AND NVIDIA AND  ITS SUPPLIERS DISCLAIM  ALL WARRANTIES,  EITHER  EXPRESS
// OR IMPLIED, INCLUDING, BUT NOT LIMITED  TO, IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL  NVIDIA OR ITS SUPPLIERS
// BE  LIABLE  FOR  ANY  SPECIAL,  INCIDENTAL,  INDIRECT,  OR  CONSEQUENTIAL DAMAGES
// WHATSOEVER (INCLUDING, WITHOUT LIMITATION,  DAMAGES FOR LOSS OF BUSINESS PROFITS,
// BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS)
// ARISING OUT OF THE  USE OF OR INABILITY  TO USE THIS SOFTWARE, EVEN IF NVIDIA HAS
// BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
//
//
//----------------------------------------------------------------------------------

#pragma once

#include <DXUT.h>
#include <SDKmisc.h>
#include "ScreenSpaceAOEngine.h"

class HorizonBasedAOEngine : public ScreenSpaceAOEngine
{
public:
                    HorizonBasedAOEngine();
                    ~HorizonBasedAOEngine();

    HRESULT			OnCreateDevice( ID3D10Device* pd3dDevice );
    HRESULT			OnDestroyDevice();
    HRESULT			OnResizedSwapChain( int width, int height, float fovy );
    void			OnReleasingSwapChain();
    
    HRESULT	        OnFrameRender( ID3D10ShaderResourceView *Depth, 
                                   ID3D10ShaderResourceView *Normal = NULL );

    void			RecompileShader();
    void			CreateMenu( CDXUTDialog *myMenu );
    void			OnGUIEvent( CDXUTDialog *myMenu, UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext );

protected:
    void			UpdateDirs();
    void            UpdateRadius();
    void            UpdateAngleBias();
    void            UpdateContrast();

    // Buffer sizes
    int				m_BBWidth;
    int				m_BBHeight;

    // D3D Device  
    ID3D10Device*	m_D3DDevice;
    D3D10_VIEWPORT	m_Viewport;

    // The effects and rendering techniques
    ID3D10Effect          *m_Effect;
    ID3D10EffectTechnique *m_Technique_HBAO;
    ID3D10EffectTechnique *m_Technique_HBAO_LD[3];
    ID3D10EffectTechnique *m_Technique_HBAO_NLD[3];

    // Effect variable pointers
    ID3D10EffectShaderResourceVariable* m_DepthTexVar;
    ID3D10EffectShaderResourceVariable* m_NormalTexVar;
    ID3D10EffectShaderResourceVariable* m_RandTexVar;
    
    ID3D10EffectScalarVariable*	m_pNumSteps;
    ID3D10EffectScalarVariable*	m_pNumDirs;
    ID3D10EffectScalarVariable*	m_pRadius;
    ID3D10EffectScalarVariable*	m_pAngleBias;
    ID3D10EffectScalarVariable*	m_pTanAngleBias;
    ID3D10EffectScalarVariable*	m_pInvRadius;
    ID3D10EffectScalarVariable*	m_pSqrRadius;
    ID3D10EffectScalarVariable*	m_pAttenuation;
    ID3D10EffectScalarVariable*	m_pContrast;
    ID3D10EffectScalarVariable*	m_pAspectRatio;
    ID3D10EffectScalarVariable*	m_pInvAspectRatio;
    ID3D10EffectVectorVariable*	m_pInvFocalLen;
    ID3D10EffectVectorVariable*	m_pFocalLen;
    ID3D10EffectVectorVariable*	m_pInvResolution;
    ID3D10EffectVectorVariable*	m_pResolution;
    ID3D10EffectVectorVariable*	m_pDirsArray;

    int							m_NumSteps;
    int							m_NumDirs;
    float						m_RadiusMultiplier;
    float						m_AngleBias;
    float						m_Attenuation;
    float						m_Contrast;
    float						m_AspectRatio;
    float						m_InvAspectRatio;
    float						m_InvFocalLen[2];
    float						m_FocalLen[2];
    float						m_InvResolution[2];
    float						m_Resolution[2];
    UINT8                       m_QualityMode;

    ID3D10ShaderResourceView*   m_DepthBuffer;

    ID3D10Texture2D*			m_pRndTexture;
    ID3D10ShaderResourceView*	m_pRndTexSRV;
};
