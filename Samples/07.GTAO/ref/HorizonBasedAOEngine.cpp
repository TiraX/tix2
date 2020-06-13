//----------------------------------------------------------------------------------
// File:   HorizonBasedAOEngine.cpp
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

#include "HorizonBasedAOEngine.h"
#include "MersenneTwister.h"

#define MAX_PATH_STR 512

MTRand rng;

enum {
    IDC_RADIUS_STATIC = 4,
    IDC_RADIUS_SLIDER,
    IDC_ANGLE_BIAS_STATIC,
    IDC_ANGLE_BIAS_SLIDER,
    IDC_NUM_DIRS_STATIC,
    IDC_NUM_DIRS_SLIDER,
    IDC_NUM_STEPS_STATIC,
    IDC_NUM_STEPS_SLIDER,
    IDC_ATTENUATION_STATIC,
    IDC_ATTENUATION_SLIDER,
    IDC_CONTRAST_STATIC,
    IDC_CONTRAST_SLIDER,
    IDC_QUALITY_MODE_0,
    IDC_QUALITY_MODE_1,
    IDC_QUALITY_MODE_2,
};

//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
HorizonBasedAOEngine::HorizonBasedAOEngine()
{
    m_BBWidth       = 0;
    m_BBHeight      = 0;

    m_Effect        = NULL;
    m_SSAOBuffer	= NULL; 
    m_D3DDevice		= NULL;
    m_pRndTexture	= NULL;
    m_pRndTexSRV    = NULL;

    m_RadiusMultiplier  = 1.0f;
    m_AngleBias         = 30;
    m_NumDirs		    = 16;
    m_NumSteps		    = 8;
    m_Contrast          = 1.4f;
    m_Attenuation       = 1.0f;
    m_QualityMode       = 1;
}

//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
HRESULT HorizonBasedAOEngine::OnResizedSwapChain( int width, int height, float fovy )
{
    HRESULT hr = S_OK;

    // Recalculate buffer sizes
    m_BBWidth    = width;
    m_BBHeight   = height;
 
    RecompileShader();
    UpdateDirs();

    // Create the RT
    D3D10_TEXTURE2D_DESC pDesc;
    pDesc.Width              = m_BBWidth;
    pDesc.Height             = m_BBHeight;
    pDesc.MipLevels          = 1;
    pDesc.ArraySize          = 1;
    pDesc.Format             = DXGI_FORMAT_R8_UNORM;
    pDesc.SampleDesc.Count   = 1;
    pDesc.SampleDesc.Quality = 0;
    pDesc.Usage              = D3D10_USAGE_DEFAULT;
    pDesc.BindFlags          = D3D10_BIND_SHADER_RESOURCE | D3D10_BIND_RENDER_TARGET;
    pDesc.CPUAccessFlags     = 0;
    pDesc.MiscFlags          = 0;

    SAFE_DELETE(m_SSAOBuffer);
    m_SSAOBuffer = new SimpleRT( m_D3DDevice , &pDesc);

    m_FocalLen[0]      = 1.0f / tanf(fovy * 0.5f) *  (float)m_BBHeight / (float)m_BBWidth;
    m_FocalLen[1]      = 1.0f / tanf(fovy * 0.5f);
    m_InvFocalLen[0]   = 1.0f / m_FocalLen[0];
    m_InvFocalLen[1]   = 1.0f / m_FocalLen[1];
    m_InvResolution[0] = 1.0f / m_BBWidth;
    m_InvResolution[1] = 1.0f / m_BBHeight;
    m_Resolution[0]    = (float)m_BBWidth;
    m_Resolution[1]    = (float)m_BBHeight;

    m_pFocalLen->SetFloatVector     ( m_FocalLen );
    m_pInvFocalLen->SetFloatVector  ( m_InvFocalLen );
    m_pInvResolution->SetFloatVector( m_InvResolution );
    m_pResolution->SetFloatVector   ( m_Resolution );

    m_Viewport.TopLeftX = 0;
    m_Viewport.TopLeftY = 0;
    m_Viewport.MinDepth = 0;
    m_Viewport.MaxDepth = 1;
    m_Viewport.Width    = m_BBWidth;
    m_Viewport.Height   = m_BBHeight;


    if( FAILED( hr ) )
    {
        MessageBox(NULL, L"Initialization of HorizonBasedAOEngine failed!!!", L"ERROR", MB_OK|MB_SETFOREGROUND|MB_TOPMOST);		
        return FALSE;
    }
    return hr;
}
//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
void HorizonBasedAOEngine::OnReleasingSwapChain( )
{
    m_BBWidth  = 0;
    m_BBHeight = 0;
   
    SAFE_DELETE(m_SSAOBuffer);
}

//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
HRESULT HorizonBasedAOEngine::OnDestroyDevice( )
{
    SAFE_RELEASE(m_Effect);
    SAFE_RELEASE(m_pRndTexSRV);
    SAFE_RELEASE(m_pRndTexture);
    SAFE_RELEASE(m_Effect);

    return S_OK;
}

//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
void HorizonBasedAOEngine::UpdateRadius()
{
    float R = m_RadiusMultiplier * m_AORadius;
    m_pRadius->SetFloat( R );
    m_pInvRadius->SetFloat( 1.0f / R );
    m_pSqrRadius->SetFloat( R * R );
}

//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
void HorizonBasedAOEngine::UpdateAngleBias()
{
    float angle = m_AngleBias * D3DX_PI / 180;
    m_pAngleBias->SetFloat( angle );
    m_pTanAngleBias->SetFloat( tan(angle) );
    UpdateContrast();
}

//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
void HorizonBasedAOEngine::UpdateContrast()
{
    float contrast = m_Contrast / (1.0f - sin(m_AngleBias * D3DX_PI / 180));
    m_pContrast->SetFloat( contrast );
}

//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
HRESULT HorizonBasedAOEngine::OnFrameRender( ID3D10ShaderResourceView *Depth, ID3D10ShaderResourceView *Normal )
{
    HRESULT hr = S_OK;

    UpdateRadius();

    if( Normal )
        m_Technique_HBAO = m_Technique_HBAO_NLD[m_QualityMode];
    else
        m_Technique_HBAO = m_Technique_HBAO_LD[m_QualityMode];

    m_D3DDevice->RSSetViewports(1, &m_Viewport);
    m_D3DDevice->OMSetRenderTargets( 1, &m_SSAOBuffer->pRTV, NULL );

    V( m_DepthTexVar->SetResource  ( Depth ) );
    V( m_NormalTexVar->SetResource ( Normal ) );
    V( m_RandTexVar->SetResource   ( m_pRndTexSRV ) );

    m_Technique_HBAO->GetPassByIndex(0)->Apply( 0 );
    
    m_D3DDevice->Draw( 3, 0 );

    m_DepthTexVar->SetResource  ( NULL );
    m_NormalTexVar->SetResource ( NULL );
    m_RandTexVar->SetResource   ( NULL );

    m_Technique_HBAO->GetPassByIndex(0)->Apply( 0 );

    return hr;
}


//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
void HorizonBasedAOEngine::CreateMenu ( CDXUTDialog *myMenu )
{
    int iY = 0;
    WCHAR sz[100];

    int dy = 22;
    StringCchPrintf( sz, 100, L"Radius multiplier: %0.2f", 1.0f ); 
    myMenu->AddStatic( IDC_RADIUS_STATIC, sz, 35, iY += dy + 20, 125, 22 );
    myMenu->AddSlider( IDC_RADIUS_SLIDER, 50, iY += dy, 100, 22, 0, 100, 50 );

    StringCchPrintf( sz, 100, L"Angle Bias: %0.2f", m_AngleBias ); 
    myMenu->AddStatic( IDC_ANGLE_BIAS_STATIC, sz, 35, iY += dy, 125, 22 );
    myMenu->AddSlider( IDC_ANGLE_BIAS_SLIDER, 50, iY += dy, 100, 22, 0, 60, (int)m_AngleBias );

    StringCchPrintf( sz, 100, L"Number of directions: %d", m_NumDirs ); 
    myMenu->AddStatic( IDC_NUM_DIRS_STATIC, sz, 35, iY += dy, 125, 22 );
    myMenu->AddSlider( IDC_NUM_DIRS_SLIDER, 50, iY += dy, 100, 22, 0, 100, 4*m_NumDirs );

    StringCchPrintf( sz, 100, L"Number of steps: %d", m_NumSteps ); 
    myMenu->AddStatic( IDC_NUM_STEPS_STATIC, sz, 35, iY += dy, 125, 22 );
    myMenu->AddSlider( IDC_NUM_STEPS_SLIDER, 50, iY += dy, 100, 22, 0, 128, 4*m_NumSteps );

    StringCchPrintf( sz, 100, L"Attenuation: %0.2f", m_Attenuation ); 
    myMenu->AddStatic( IDC_ATTENUATION_STATIC, sz, 35, iY += dy, 125, 22 );
    myMenu->AddSlider( IDC_ATTENUATION_SLIDER, 50, iY += dy, 100, 22, 0, 100, (int)(100.0f*m_Attenuation) );

    StringCchPrintf( sz, 100, L"Contrast: %0.2f", m_Contrast ); 
    myMenu->AddStatic( IDC_CONTRAST_STATIC, sz, 35, iY += dy, 125, 22 );
    myMenu->AddSlider( IDC_CONTRAST_SLIDER, 50, iY += dy, 100, 22, 0, 200, (int)(50.0f*m_Contrast) );

    myMenu->AddRadioButton( IDC_QUALITY_MODE_0, 0, L"Low Quality" , 35, iY += 24, 125, 22, (m_QualityMode == 0) );
    myMenu->AddRadioButton( IDC_QUALITY_MODE_1, 0, L"Normal Quality" , 35, iY += 24, 125, 22, (m_QualityMode == 1) );
    myMenu->AddRadioButton( IDC_QUALITY_MODE_2, 0, L"High Quality" , 35, iY += 24, 125, 22, (m_QualityMode == 2) );
}

void HorizonBasedAOEngine::OnGUIEvent( CDXUTDialog *myMenu, UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext )
{
    switch( nControlID )
    {
        case IDC_RADIUS_SLIDER: 
        {
            m_RadiusMultiplier = (float) myMenu->GetSlider( IDC_RADIUS_SLIDER )->GetValue() / 50.0f;

            WCHAR sz[100];
            StringCchPrintf( sz, 100, L"Radius multiplier: %0.2f", m_RadiusMultiplier ); 
            myMenu->GetStatic( IDC_RADIUS_STATIC )->SetText( sz );

            break;
        }

        case IDC_ANGLE_BIAS_SLIDER: 
        {
            m_AngleBias = (float) myMenu->GetSlider( IDC_ANGLE_BIAS_SLIDER )->GetValue();

            WCHAR sz[100];
            StringCchPrintf( sz, 100, L"Angle Bias: %0.2f", m_AngleBias ); 
            myMenu->GetStatic( IDC_ANGLE_BIAS_STATIC )->SetText( sz );

            UpdateAngleBias();
            break;
        }

        case IDC_NUM_DIRS_SLIDER: 
        {
            m_NumDirs = myMenu->GetSlider( IDC_NUM_DIRS_SLIDER )->GetValue() / 4;

            WCHAR sz[100];
            StringCchPrintf( sz, 100, L"Number of directions: %d", m_NumDirs ); 
            myMenu->GetStatic( IDC_NUM_DIRS_STATIC )->SetText( sz );

            m_pNumDirs->SetFloat( (float)m_NumDirs );
            UpdateDirs();
            break;
        }
        case IDC_NUM_STEPS_SLIDER: 
        {
            m_NumSteps = myMenu->GetSlider( IDC_NUM_STEPS_SLIDER )->GetValue() / 4;

            WCHAR sz[100];
            StringCchPrintf( sz, 100, L"Number of steps: %d", m_NumSteps ); 
            myMenu->GetStatic( IDC_NUM_STEPS_STATIC )->SetText( sz );

            m_pNumSteps->SetFloat( (float)m_NumSteps );
            break;
        }
        case IDC_ATTENUATION_SLIDER: 
        {
            m_Attenuation = (float)myMenu->GetSlider( IDC_ATTENUATION_SLIDER )->GetValue() / 100.0f;

            WCHAR sz[100];
            StringCchPrintf( sz, 100, L"Attenuation: %0.2f", m_Attenuation ); 
            myMenu->GetStatic( IDC_ATTENUATION_STATIC )->SetText( sz );

            m_pAttenuation->SetFloat( m_Attenuation );
            break;
        }
        case IDC_CONTRAST_SLIDER: 
        {
            m_Contrast = (float)myMenu->GetSlider( IDC_CONTRAST_SLIDER )->GetValue() / 50.0f;

            WCHAR sz[100];
            StringCchPrintf( sz, 100, L"Contrast: %0.2f", m_Contrast );  
            myMenu->GetStatic( IDC_CONTRAST_STATIC )->SetText( sz );

            UpdateContrast();
            break;
        }
        case IDC_QUALITY_MODE_0:
        case IDC_QUALITY_MODE_1:
        case IDC_QUALITY_MODE_2:
        {
            m_QualityMode = myMenu->GetRadioButton( IDC_QUALITY_MODE_0 )->GetChecked() ? 0:
                            myMenu->GetRadioButton( IDC_QUALITY_MODE_1 )->GetChecked() ? 1 : 2;
            break;
        }
    }
}

#define SCALE ((1<<15))

void HorizonBasedAOEngine::UpdateDirs()
{
    rng.seed((unsigned)0);

    float dirs[64];
    if(m_NumDirs > 32)
        m_NumDirs = 32;
    float inc = 2.0f * D3DX_PI / (float)m_NumDirs;
    for(int i=0; i < 2*m_NumDirs; i+=2)
    {
        float angle = inc*(float)(i/2);
        dirs[i]     = cos(angle);
        dirs[i+1]   = sin(angle);
    }

    m_pNumDirs->SetFloat( (float)m_NumDirs );
    m_pDirsArray->SetFloatVectorArray( dirs, 0, m_NumDirs );

    SAFE_RELEASE(m_pRndTexture);
    SAFE_RELEASE(m_pRndTexSRV);

    signed short f[64*64*4];
    for(int i=0; i<64*64*4; i+=4)
    {
        float angle = 2.0f*D3DX_PI * rng.randExc() / (float)m_NumDirs;
        f[i  ] = (signed short)(SCALE*cos(angle));
        f[i+1] = (signed short)(SCALE*sin(angle));
        f[i+2] = (signed short)(SCALE*rng.randExc());
        f[i+3] = 0;
    }

    D3D10_TEXTURE2D_DESC tex_desc;
    tex_desc.Width            = 64;
    tex_desc.Height           = 64;
    tex_desc.MipLevels        = tex_desc.ArraySize = 1;
    tex_desc.Format           = DXGI_FORMAT_R16G16B16A16_SNORM;
    tex_desc.SampleDesc.Count = 1;
    tex_desc.SampleDesc.Quality = 0;
    tex_desc.Usage            = D3D10_USAGE_IMMUTABLE;
    tex_desc.BindFlags        = D3D10_BIND_SHADER_RESOURCE;
    tex_desc.CPUAccessFlags   = 0;
    tex_desc.MiscFlags        = 0;

    D3D10_SUBRESOURCE_DATA sr_desc;
    sr_desc.pSysMem          = f;
    sr_desc.SysMemPitch      = 64*4*sizeof(signed short);
    sr_desc.SysMemSlicePitch = 0;

    D3D10_SHADER_RESOURCE_VIEW_DESC srv_desc;
    srv_desc.Format                    = tex_desc.Format;
    srv_desc.ViewDimension             = D3D10_SRV_DIMENSION_TEXTURE2D;
    srv_desc.Texture2D.MostDetailedMip = 0;
    srv_desc.Texture2D.MipLevels       = 1;

    m_D3DDevice->CreateTexture2D(&tex_desc, &sr_desc, &m_pRndTexture);
    m_D3DDevice->CreateShaderResourceView(m_pRndTexture, &srv_desc, &m_pRndTexSRV);
}

HRESULT HorizonBasedAOEngine::OnCreateDevice( ID3D10Device* pd3dDevice )
{
    HRESULT hr = S_OK;

    m_D3DDevice = pd3dDevice;

    return hr;
}

HorizonBasedAOEngine::~HorizonBasedAOEngine()
{
    SAFE_RELEASE(m_pRndTexture);
    SAFE_RELEASE(m_pRndTexSRV);
}

void HorizonBasedAOEngine::RecompileShader()
{
    ID3D10Effect *tempEffect = NULL;

    WCHAR str[MAX_PATH_STR+1];
    DXUTFindDXSDKMediaFileCch( str, MAX_PATH_STR, DEFAULT_SHADERPATH L"\\ScreenSpaceAO\\HorizonBasedAOEngine.fx" );
    D3DX10CreateEffectFromFile( str, NULL, NULL, "fx_4_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, m_D3DDevice, NULL, NULL, &tempEffect, NULL, NULL);
    if( tempEffect && tempEffect->IsValid() )
    {
        SAFE_RELEASE(m_Effect);
        m_Effect = tempEffect;
    }
    else
    {
        MessageBox(NULL, L"Effect Compilation failed. Review your code a*#$%le", L"ERROR", MB_OK|MB_SETFOREGROUND|MB_TOPMOST);	
        return;
    }
    
    // Obtain the technique
    m_Technique_HBAO_NLD[0] = m_Effect->GetTechniqueByName( "HORIZON_BASED_AO_NLD_LOWQUALITY_Pass" );
    m_Technique_HBAO_LD[0]  = m_Effect->GetTechniqueByName( "HORIZON_BASED_AO_LD_LOWQUALITY_Pass" );
    m_Technique_HBAO_NLD[1] = m_Effect->GetTechniqueByName( "HORIZON_BASED_AO_NLD_Pass" );
    m_Technique_HBAO_LD[1]  = m_Effect->GetTechniqueByName( "HORIZON_BASED_AO_LD_Pass" );
    m_Technique_HBAO_NLD[2] = m_Effect->GetTechniqueByName( "HORIZON_BASED_AO_NLD_QUALITY_Pass" );
    m_Technique_HBAO_LD[2]  = m_Effect->GetTechniqueByName( "HORIZON_BASED_AO_LD_QUALITY_Pass" );
    
    // Obtain pointers to the shader variales
    m_DepthTexVar           = m_Effect->GetVariableByName( "tLinDepth"   )->AsShaderResource();
    m_NormalTexVar          = m_Effect->GetVariableByName( "tNormal"     )->AsShaderResource();
    m_RandTexVar            = m_Effect->GetVariableByName( "tRandom"     )->AsShaderResource();

    m_pNumSteps       = m_Effect->GetVariableByName( "g_NumSteps"      )->AsScalar();
    m_pNumDirs        = m_Effect->GetVariableByName( "g_NumDir"        )->AsScalar();
    m_pRadius         = m_Effect->GetVariableByName( "g_R"             )->AsScalar();
    m_pInvRadius      = m_Effect->GetVariableByName( "g_inv_R"         )->AsScalar();
    m_pSqrRadius      = m_Effect->GetVariableByName( "g_sqr_R"         )->AsScalar();
    m_pAngleBias      = m_Effect->GetVariableByName( "g_AngleBias"     )->AsScalar();
    m_pTanAngleBias   = m_Effect->GetVariableByName( "g_TanAngleBias"  )->AsScalar();
    m_pAttenuation    = m_Effect->GetVariableByName( "g_Attenuation"   )->AsScalar();
    m_pContrast       = m_Effect->GetVariableByName( "g_Contrast"      )->AsScalar();
    m_pFocalLen       = m_Effect->GetVariableByName( "g_FocalLen"      )->AsVector();
    m_pInvFocalLen    = m_Effect->GetVariableByName( "g_InvFocalLen"   )->AsVector();
    m_pDirsArray      = m_Effect->GetVariableByName( "g_Dirs"          )->AsVector();
    m_pInvResolution  = m_Effect->GetVariableByName( "g_InvResolution" )->AsVector();
    m_pResolution     = m_Effect->GetVariableByName( "g_Resolution"    )->AsVector();
 
    // Set Defaults
    UpdateDirs();
    UpdateRadius();
    UpdateAngleBias();

    m_pNumSteps->SetFloat      ( (float)m_NumSteps );
    m_pAttenuation->SetFloat   ( m_Attenuation );

    m_pFocalLen->SetFloatVector     ( m_FocalLen );
    m_pInvFocalLen->SetFloatVector  ( m_InvFocalLen );
    m_pInvResolution->SetFloatVector( m_InvResolution );
    m_pResolution->SetFloatVector   ( m_Resolution );
}
