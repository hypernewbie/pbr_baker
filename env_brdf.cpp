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

#include "env_brdf.h"
using namespace glm;

#include <hammersley/hammersley.h>
#include <hammersley/hammersley.c>

#define ENVBRDF_SAMPLE_SIZE 1024
#define HAMMERSLEY_SEQUENCE_N ENVBRDF_SAMPLE_SIZE
#define HAMMERSLEY_SEQUENCE_M 2
#define TEST_HAMMERSLEY false

// Gloss parameterization similar to Call of Duty: Advanced Warfare
//
float ggx_GlossToAlpha2( float gloss )
{
    return 2.0f / ( 1.0f + pow( 2.0f, 18.0f * gloss ) );
}

// src : https://cdn2.unrealengine.com/Resources/files/2013SiggraphPresentationsNotes-26915738.pdf
//
vec3 ggx_ImportanceSampleGGX( vec2 xi, float alpha2, vec3 N )
{
    float phi = 2.0f * PI * xi.x;
    float cosTheta = sqrt( ( 1 - xi.y ) / ( 1 + ( alpha2 - 1 ) * xi.y ) );
    float sinTheta = sqrt( 1 - cosTheta * cosTheta );
    vec3 H( sinTheta * cos( phi ), sinTheta * sin( phi ), cosTheta );
    vec3 up = abs( N.z ) < 0.999f ? vec3( 0, 0, 1 ) : vec3( 1, 0, 0 );
    vec3 tangentX = normalize( cross( up, N ) );
    vec3 tangentY = cross( N, tangentX );    return tangentX * H.x + tangentY * H.y + N * H.z;
}

// src: https://people.sc.fsu.edu/~jburkardt/cpp_src/hammersley/hammersley.html
//
vec2 noise_getHammersleyAtIdx( int idx )
{
    static std::vector< vec2 > hmValues;

    if ( !hmValues.size() ) {

        // Pre-calculate hammersley sequence.

        hmValues.resize( HAMMERSLEY_SEQUENCE_N );
        auto v = hammersley_sequence( 0, HAMMERSLEY_SEQUENCE_N, HAMMERSLEY_SEQUENCE_M, HAMMERSLEY_SEQUENCE_N );
        assert( v );

        for( int i = 0; i < HAMMERSLEY_SEQUENCE_N; i++ )
        {
            hmValues[i].x = v[i * 2 + 0];
            hmValues[i].y = v[i * 2 + 1];
        }
        free( v );
    }

    return hmValues[ idx % HAMMERSLEY_SEQUENCE_N ];
}

// src: https://schuttejoe.github.io/post/ggximportancesamplingpart1/
//
float ggx_SmithGeom( float NdotL, float NdotV, float alpha2 )
{
    float denomA = NdotV * sqrt( alpha2 + ( 1.0f - alpha2 ) * NdotL * NdotL );
    float denomB = NdotL * sqrt( alpha2 + ( 1.0f - alpha2 ) * NdotV * NdotV );
    return 2.0f * NdotL * NdotV / ( denomA + denomB );
}

// src : https://cdn2.unrealengine.com/Resources/files/2013SiggraphPresentationsNotes-26915738.pdf
//
vec2 ggx_IntegrateBRDF( float gloss, float NdotV )
{
    vec3 V = vec3(
        sqrt( 1.0f - NdotV * NdotV ), // sin
        0.0f,
        NdotV                         // cos
    );
    vec3 N = vec3( 0, 0, 1 );

    float A = 0.0;
    float B = 0.0;
    float alpha2 = ggx_GlossToAlpha2( gloss );

    for( uint i = 0; i < ENVBRDF_SAMPLE_SIZE; i++ )
    {
        auto xi = noise_getHammersleyAtIdx( i );
        auto H = ggx_ImportanceSampleGGX( xi, alpha2, N );
        vec3 L = 2.0f * dot( V, H ) * H - V;

        float NdotL = saturate( L.z );
        float NdotH = saturate( H.z );
        float VdotH = saturate( dot( V, H ) );

        if( NdotL > 0.0f ) {
            float G = ggx_SmithGeom( NdotL, NdotV, alpha2 );
            float Gvis = G * VdotH / ( NdotH * NdotV );
            float Fc = pow( 1 - VdotH, 5.0f );
            A += ( 1 - Fc ) * Gvis;
            B += Fc * Gvis;            // printf( "x %f y %f i %d = { A %f B %f G %f Gvis %f Fc %f } { NdotH %f NdotV %f } \n", gloss, NdotV, i, A, B, G, Gvis, Fc, NdotH, NdotV );
        }
    }

    // printf( "x %f y %f = { A %f B %f }\n", gloss, NdotV, A / ENVBRDF_SAMPLE_SIZE, B / ENVBRDF_SAMPLE_SIZE );
    return vec2( A, B ) / float( ENVBRDF_SAMPLE_SIZE );
}

vec4 ggx_IntegrateBRDF_Function( float x, float y )
{
    float NdotV = max( y, EPS );
    float gloss = x;
    auto v = ggx_IntegrateBRDF( gloss, NdotV );
    return vec4( v.x, v.y, 0.0f, 1.0f );
}


void bake_envBRDF()
{
#if TEST_HAMMERSLEY
    for( int i = 0; i < 64; i++ )
    {
        auto xi = noise_getHammersleyAtIdx( i );
        printf("{ %.2f %.2f }\n", xi.x, xi.y );
    }
#endif // #if TEST_HAMMERSLEY

    baker_imageFunction2D( ggx_IntegrateBRDF_Function, 128, "output/env_brdf.png" );
}