/*
    Copyright 2019 Xi Chen

    Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
    associated documentation files (the "Software"), to deal in the Software without restriction,
    including without limitation the rights to use, copy, modify, merge, publish, distribute,
    sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all copies or substantial
    portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
    NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES
    OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
    CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <functional>
#include "common.h"
#include "subsurface.h"
using namespace glm;

// ref: https://en.wikipedia.org/wiki/Gaussian_function
// ref: https://www.geeksforgeeks.org/gaussian-filter-generation-c/
//
float pss_Gaussian( float x, float sigma )
{
    float s = 2.0f * sigma * sigma;
    return ( exp( -( x * x ) / s ) ) * ( 1.0f / sqrt( 2.0f * PI * s ) );
}

// Standard gaussian integral.
vec3 pss_StandardGaussian( float dist, float w )
{
    return vec3( pss_Gaussian( dist, w ) );
}

// Standard smoothstep integral.
vec3 pss_Smoothstep( float dist, float w )
{
    float x = 1.0f - glm::smoothstep( 0.2f, 0.8f,  dist / w );
    return vec3( x );
}

// Gaussian fit to dipole subsurface diffuse model of skin by d'Eon et. al.
// This is effectively the lookup table from "Pre-Integrated Skin Scattering" by Penner et. al. 
//
// ref: https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch14.html
//
vec3 pss_NVIDIA_SumOfGaussiansFit( float dist, float w )
{
    static vec4 gaussianFitTable[] = {
        vec4( 0.233f, 0.455f, 0.649f, 0.0064f ),
        vec4( 0.100f, 0.336f, 0.344f, 0.0484f ),
        vec4( 0.118f, 0.198f, 0.0f,   0.187f ),
        vec4( 0.113f, 0.007f, 0.007f, 0.567f ),
        vec4( 0.358f, 0.004f, 0.0f,   1.99f ),
        vec4( 0.078f, 0.0f,   0.0f,   7.41f )
    };

    vec3 totalWeight;
    for( auto g : gaussianFitTable ) {
        auto weight = pss_Gaussian( dist, w * g.w );
        totalWeight += weight * vec3( g.r, g.g, g.b );
    }
    return totalWeight;
}

static std::function< vec3( float dist, float w ) > s_sssConvKernel = nullptr;

vec4 pss_BakeCurvatureTable( float x, float y )
{
    float theta = x * PI;
    float NdotL = cos( theta );
    float w = 0.001f + y * 0.5f;

    vec3 sum;
    vec3 norm;

    const int NUM_SAMPLES = 2048;
    for( int i = 0; i < NUM_SAMPLES; i++ ) {
        float x2 = float( i ) / float ( NUM_SAMPLES - 1 );
        float theta2 = x2 * PI;
        float NdotL2 = cos( theta2 );

        float dist = abs( NdotL2 - NdotL );
        vec3 weight = s_sssConvKernel( dist, w );
        
        sum += weight * clamp( NdotL2, 0.0f, 1.0f );
        norm += weight;
    }
    
    sum /= norm;
    sum.x = pow( sum.x, 1.0f / 2.2f );
    sum.y = pow( sum.y, 1.0f / 2.2f );
    sum.z = pow( sum.z, 1.0f / 2.2f );

    return vec4( sum, 1.0f );
}

void bake_subsurface()
{
    s_sssConvKernel = pss_StandardGaussian;
    baker_imageFunction2D( pss_BakeCurvatureTable, 256, "output/subsurface_gaussian.png" );
    
    s_sssConvKernel = pss_Smoothstep;
    baker_imageFunction2D( pss_BakeCurvatureTable, 256, "output/subsurface_smoothstep.png" );

    s_sssConvKernel = pss_NVIDIA_SumOfGaussiansFit;
    baker_imageFunction2D( pss_BakeCurvatureTable, 256, "output/subsurface_penner.png" );
}