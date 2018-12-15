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

#pragma once

struct KernelSample
{
	float r,g,b,X,Y;
};

class SeparableSSS {
    public:
        /**
         * width, height: should match the backbuffer's size.
         *
         * fovy: field of view angle used for rendering the scene, in degrees.
         *
         * sssWidth: specifies the global level of subsurface scattering, or in
         *     other words, the width of the filter.
         *
         * nSamples: number of samples of the kernel convolution.
         *
         * followShape: irradiance diffusion should occur on the surface of the
         *     object, not in a screen oriented plane. Setting 'followShape' to
         *     'true' will ensure that diffusion is more accurately calculated,
         *     at the expense of more memory accesses. This is usually only
         *     noticeable under specific lighting configurations.
         *
         * separateStrengthSource: this parameter enables to use a specific
         *     buffer for fetching the SSS strength, instead of using the alpha
         *     channel of the color buffer.
         *
         * stencilInitialized: if an stencil is initialized for selectively
         *     processing the frame buffer, set this to 'true'. For example,
         *     this cannot be easily done, when using multisampling. See
         *     @STENCIL below for more info.
         *
         * format: format of the temporal framebuffer. Should be left as is,
         *     unless an HDR format is desired.
         */
        SeparableSSS(int width, int height,
                     float fovy,
                     float sssWidth,
                     int nSamples=17,
                     bool stencilInitialized=true,
                     bool followShape=true,
                     bool separateStrengthSource=false);
        ~SeparableSSS();

        /**
         * IMPORTANT NOTICE: all render targets below must not be multisampled.
         *
         * mainRTV and mainSRV: render target and shader resources of the
         *     final rendered image. The filter works on place, so they should
         *     be associated to the same resource. The SSS intensity should be
         *     stored in the alpha channel.
         *
         * depthSRV: shader resource of the the linear depth buffer of the
         *     scene, resolved in case of using multisampling. The resolve
         *     should be a simple average to avoid artifacts in the silhouette
         *     of objects.
         *
         * @STENCIL
         * depthDSV: two possibilities over here:
         *     a) if 'stencilInitialized' was set to 'true', then only the
         *        pixels marked in the stencil buffer with the value 'id' will
         *        be processed in both the horizontal and vertical passes.
         *     b) if 'stencilInitialized' was set to 'false' (for example when
         *        using multisampling), then it will be initialized on the fly
         *        for optimizing the second vertical pass.
         *
         * stregthSRV: if 'separateStrengthSource' was set to 'true' when
         *     creating this object, the SSS stregth will be fetched from the
         *     alpha channel of this texture instead of the alpha channel of
         *     the color buffer.
         *
         * id: stencil value used to render the objects we must apply
         *     subsurface scattering on. Should be left as is in case
         *     'stencilInitialized' was set to false when creating the object.
         *
         * Note that 'depthSRV' and 'depthDSV' cannot be the views of the same
         * depth-stencil buffer.
         */
        void go(FRHI * RHI, FFullScreenRender& FSRenderer);

        /**
         * This parameter specifies the global level of subsurface scattering,
         * or in other words, the width of the filter.
         */
        void setWidth(float width) { this->sssWidth = width; }
        float getWidth() const { return sssWidth; }

		float getFOV() const { return fov; }
		float getMaxOffset() const { return maxOffsetMm; }
		
		void overrideTranslucencyParameters(const TVector<float> & _kernelData);
		void overrideSsssDiscrSepKernel(const TVector<float> & _kernelData);
		//void overrideSsssImage2DKernel(ID3D10ShaderResourceView * tex, float maxOffsetMm);

    private:
		void loadKernelData();
		void calculateOffsets(float _range, float _exponent, int _offsetCount, TVector<float> & _offsets);
		void calculateAreas(TVector<float> & _offsets, TVector<float> & _areas);
		vector3df linInterpol1D(TVector<KernelSample> _kernelData, float _x);	
		void calculateSsssDiscrSepKernel(const TVector<KernelSample> & _kernelData);

		double convertLinParamVector(const TVector<float> & _kernelData, TVector<vector3df> & weights, TVector<vector3df> & vars);

        vector3df gaussian(float variance, float r);
        vector3df profile(float r);
        void calculateKernel(double _maxSampleMm = 0);

		float singleGaussian(float variance, float r);
		vector3df sumOfGaussians(TVector<vector3df> const & weights, TVector<vector3df> const & variances, vector3df r);
		void calculateGaussianKernel(double _maxSampleMm = 0);

		float sssWidth;
		float fov;
		float maxOffsetMm;
        int nSamples;
        bool stencilInitialized;
        vector3df strength;
        vector3df falloff;

		TVector<vector3df> ssssWeights;
		TVector<vector3df> ssssVariances;

		TVector<vector3df> translucencyWeights;
		TVector<vector3df> translucencyVariances;
		
        TVector<vector4df> kernel;
};
