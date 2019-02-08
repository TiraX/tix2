/**
 * Copyright (C) 2012 Jorge Jimenez (jorge@iryoku.com)
 * Copyright (C) 2012 Diego Gutierrez (diegog@unizar.es)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the following disclaimer
 *       in the documentation and/or other materials provided with the 
 *       distribution:
 *
 *       "Uses Separable SSS. Copyright (C) 2012 by Jorge Jimenez and Diego
 *        Gutierrez."
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS 
 * IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS OR CONTRIBUTORS 
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are 
 * those of the authors and should not be interpreted as representing official
 * policies, either expressed or implied, of the copyright holders.
 */


#include "stdafx.h"
#include "SeparableSSS.h"
using namespace std;


SeparableSSS::SeparableSSS(float fovy,
                           float sssWidth,
                           bool stencilInitialized,
                           bool followSurface,
                           bool separateStrengthSource)
	: sssWidth(sssWidth)
	, fov(fovy)
	, maxOffsetMm(0.f)
	, stencilInitialized(stencilInitialized)
	, strength(vector3df(0.48f, 0.41f, 0.28f))
	, falloff(vector3df(1.0f, 0.37f, 0.3f)) 
{
	loadKernelData();
}

SeparableSSS::~SeparableSSS() 
{
}

void SeparableSSS::loadKernelData()
{
	static const char* KernelDataFile = "Skin1_PreInt_DISCSEP.bn";

	TVector<float> Data;

	// read float count
	TFile DataFile;
	if (DataFile.Open(KernelDataFile, EFA_READ))
	{
		char sv[4];
		DataFile.Read(sv, 4, 4);
		int32 fc = (int32)(floor(*((float*)sv)));
		Data.resize(fc);
		DataFile.Read(&Data[0], fc * 4, fc * 4);
	}

	// setup sss kernel
	overrideSsssDiscrSepKernel(Data); 
	//overrideTranslucencyParameters(_data.kernels[_data.kernels.size() - 1].Data);
}

void SeparableSSS::calculateOffsets(float _range, float _exponent, int32 _offsetCount, vector<float> & _offsets)
{
	// Calculate the offsets:
    float step = 2.0f * _range / (_offsetCount - 1);
    for (int32 i = 0; i < _offsetCount; i++) {
        float o = -_range + float(i) * step;
        float sign = o < 0.0f? -1.0f : 1.0f;
        float ofs = _range * sign * abs(pow(o, _exponent)) / pow(_range, _exponent);
		_offsets.push_back(ofs);
    }
}

void SeparableSSS::calculateAreas(vector<float> & _offsets, vector<float> & _areas)
{
	int32 size = (int32)_offsets.size();

	for (int32 i = 0; i < size; i++) {
        float w0 = i > 0? abs(_offsets[i] - _offsets[i - 1]) : 0.0f;
        float w1 = i < size - 1? abs(_offsets[i] - _offsets[i + 1]) : 0.0f;
        float area = (w0 + w1) / 2.0f;
		_areas.push_back(area);
    }
}

/**
 *	This function computes a naive linear interpolation from the supplied 1D kernel samples
 */
vector3df SeparableSSS::linInterpol1D(vector<KernelSample> _kernelData, float _x)
{
	// naive, a lot to improve here
	
	if(_kernelData.size()<1) throw "_kernelData empty";
		
	uint32 i = 0;
	while(i < _kernelData.size())
	{
		if(_x > _kernelData[i].X) i++;
		else break;
	}

	vector3df v;

	if(i<1)
	{
		v.X = _kernelData[0].r;
		v.Y = _kernelData[0].g;
		v.Z = _kernelData[0].b;
	}
	else if(i>_kernelData.size()-1)
	{
		v.X = _kernelData[_kernelData.size()-1].r;
		v.Y = _kernelData[_kernelData.size()-1].g;
		v.Z = _kernelData[_kernelData.size()-1].b;
	}
	else
	{
		KernelSample b = _kernelData[i];
		KernelSample a = _kernelData[i-1];

		float d = b.X-a.X;
		float dx = _x-a.X;

		float t = dx/d;

		v.X = a.r * (1-t) + b.r * t;
		v.Y = a.g * (1-t) + b.g * t;
		v.Z = a.b * (1-t) + b.b * t;
	}	

	return v;
}

/**
 *	This function computes the downsampled separable 1D rendering kernel (using impoartance sampling) 
 */
void SeparableSSS::calculateSsssDiscrSepKernel(const vector<KernelSample> & _kernelData)
{	
    const float EXPONENT = 2.0f; // used for impartance sampling

	float RANGE = _kernelData[_kernelData.size()-1].X; // get max. sample location
	
	// calculate offsets
    vector<float> offsets;
	calculateOffsets(RANGE, EXPONENT, SampleCount, offsets);

	// calculate areas (using importance-sampling) 
	vector<float> areas;
	calculateAreas(offsets, areas);

	kernel.resize(SampleCount);

	vector3df sum = vector3df(0,0,0); // weights sum for normalization
	
	// compute interpolated weights
	for(int32 i=0; i<SampleCount; i++)
	{
		float sx = offsets[i];

		vector3df v = linInterpol1D(_kernelData, sx);
		kernel[i].X = v.X * areas[i];
		kernel[i].Y = v.Y * areas[i];
		kernel[i].Z = v.Z * areas[i];
		kernel[i].W = sx;

		sum.X += kernel[i].X;
		sum.Y += kernel[i].Y;
		sum.Z += kernel[i].Z;
	}

	// Normalize
    for (int32 i = 0; i < SampleCount; i++) {
        kernel[i].X /= sum.X;
        kernel[i].Y /= sum.Y;
        kernel[i].Z /= sum.Z;
    }

	// TEMP put center at first
    vector4df t = kernel[SampleCount / 2];
    for (int32 i = SampleCount / 2; i > 0; i--)
        kernel[i] = kernel[i - 1];
    kernel[0] = t;

	maxOffsetMm = RANGE;
	//TI_ASSERT(0);
	// set shader vars
	//HRESULT hr;
	//V(effect->GetVariableByName("maxOffsetMm")->AsScalar()->SetFloat(RANGE));
	//V(kernelVariable->SetFloatVectorArray((float *) &kernel.front(), 0, nSamples));
}

void SeparableSSS::go(FRHI * RHI, FFullScreenRender& FSRenderer)
{
 //   HRESULT hr;

 //   // Save the state:
 //   SaveViewportsScope saveViewport(device);
 //   SaveRenderTargetsScope saveRenderTargets(device);
 //   SaveInputLayoutScope saveInputLayout(device);

 //   // Clear the temporal render target:
 //   float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
 //   device->ClearRenderTargetView(*tmpRT, clearColor);

 //   // Clear the stencil buffer if it was not available, and thus one must be 
 //   // initialized on the fly:
 //   if (!stencilInitialized)
 //       device->ClearDepthStencilView(depthDSV, D3D10_CLEAR_STENCIL, 1.0, 0);

 //   // Set variables:
 //   V(idVariable->SetInt(id));
 //   V(sssWidthVariable->SetFloat(sssWidth));
 //   V(depthTexVariable->SetResource(depthSRV));
 //   V(strengthTexVariable->SetResource(stregthSRV));

 //   // Set input layout and viewport:
 //   quad->setInputLayout();
 //   tmpRT->setViewport();	

	//string xshader;
	//string yshader;

	//if(useImg2DKernel)
	//{
	//	// 2D convolution shaders for reference
	//	xshader = "SSSSBlurTex";
	//	yshader = "SSSSBlurTexCopy";
	//}
	//else
	//{
	//	// separable convolution shaders
	//	xshader = "SSSSBlurX";
	//	yshader = "SSSSBlurY";
	//}

	//// Run the horizontal pass:
	//V(colorTexVariable->SetResource(mainSRV));
	//technique->GetPassByName(xshader.c_str())->Apply(0);
	//device->OMSetRenderTargets(1, *tmpRT, depthDSV);
	//quad->draw();
	//device->OMSetRenderTargets(0, NULL, NULL);
	//
	//// And finish with the vertical one:
	//V(colorTexVariable->SetResource(*tmpRT));
	//technique->GetPassByName(yshader.c_str())->Apply(0);
	//device->OMSetRenderTargets(1, &mainRTV, depthDSV);
	//quad->draw();
	//device->OMSetRenderTargets(0, NULL, NULL);
}

/**
 *	Helper function to convert the linear parameter vector used for the translucency profile to its exponential weights and variances
 */
double SeparableSSS::convertLinParamVector(const TVector<float> & _kernelData, TVector<vector3df> & weights, TVector<vector3df> & vars)
{	
	uint32 size = (uint32)_kernelData.size() / 6;
	uint32 offset = (uint32)_kernelData.size() / 3;

	for(uint32 i=0; i<size; i++)
	{
		vector3df w;
		w.X = _kernelData[i];
		w.Y = _kernelData[i+offset];
		w.Z = _kernelData[i+2*offset];

		weights.push_back(w);

		vector3df v;
		v.X = _kernelData[i+size];
		v.Y = _kernelData[i+size+offset];
		v.Z = _kernelData[i+size+2*offset];

		vars.push_back(v);
	}

	return _kernelData[_kernelData.size()-1];
}

/**
 *	Calculates and overrides the separable filter kernel 
 */
void SeparableSSS::overrideSsssDiscrSepKernel(const TVector<float> & _kernelData)
{	
	// conversion of linear kernel data to sample array
	vector<KernelSample> k;
	uint32 size = (uint32)_kernelData.size() / 4;

	uint32 i = 0;
	for(uint32 s = 0; s < size; s++)
	{
		KernelSample ks;
		ks.r = _kernelData[i++];
		ks.g = _kernelData[i++];
		ks.b = _kernelData[i++];
		ks.X = _kernelData[i++];
		k.push_back(ks);
	}

	// kernel computation
    calculateSsssDiscrSepKernel(k);
}

/**
 *	Overrides the translucency profile 
 */
void SeparableSSS::overrideTranslucencyParameters(const TVector<float> & _kernelData)
{
	TVector<vector3df> translucencyWeights;
	TVector<vector3df> translucencyVariances;
	
	convertLinParamVector(_kernelData, translucencyWeights, translucencyVariances);

	TI_ASSERT(0);
	//HRESULT hr;
	//V(shader->GetVariableByName("weights")->AsVector()->SetFloatVectorArray((float *) &translucencyWeights.front(), 0, 5));
	//V(shader->GetVariableByName("variances")->AsVector()->SetFloatVectorArray((float *) &translucencyVariances.front(), 0, 5));			
}

///**
// *	Overrides the texture based 2D filter kernel used for reference 
// */
//void SeparableSSS::overrideSsssImage2DKernel(ID3D10ShaderResourceView * tex, float maxOffsetMm)
//{
//	useImg2DKernel = true;
//
//	effect->GetVariableByName("maxOffsetMm")->AsScalar()->SetFloat(maxOffsetMm);
//	effect->GetVariableByName("kernelImgTex2D")->AsShaderResource()->SetResource(tex);
//}
