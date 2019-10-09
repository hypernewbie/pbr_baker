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

#include "common.h"
#include "multiscatter_brdf.h"
using namespace glm;

float g_gloss = 0.0f;

vec4 multiscatterBRDF_roughFoundationFunction( float x, float y )
{
    float LdotH = x, NdotH = y;
    float Fd0 = LdotH + pow( 1.0 - LdotH, 5 );
    return vec4( Fd0, Fd0, Fd0, 1 );
}

float multiscatterBRDF_disneySchlick( float x )
{
    return 1.0 - 0.75 * pow( 1.0 - x, 5 );
}

vec4 multiscatterBRDF_disneyDiffuseRough( float x, float y )
{
    float NdotL = x, NdotV = y; 
    float Fd1 = multiscatterBRDF_disneySchlick( NdotL ) * multiscatterBRDF_disneySchlick( NdotV );
    return vec4( Fd1, Fd1, Fd1, 1 );
}

vec4 multiscatterBRDF_retroReflectiveBump( float x, float y )
{
    float LdotH = x, NdotH = y;
    float FdR = ( 34.5f * g_gloss * g_gloss - 59.0f * g_gloss + 24.5f ) * LdotH * pow( 2.0f, -max( 73.2f * g_gloss - 21.2f, 8.9f ) * pow( NdotH, 0.5 ) );
    return vec4( FdR, FdR, FdR, 1 );
}

void bake_multiscatterBRDF()
{
    baker_imageFunction2D( multiscatterBRDF_roughFoundationFunction, 128, "output/brdf_Fd0.png" );
    baker_imageFunction2D( multiscatterBRDF_disneyDiffuseRough, 128, "output/brdf_Fd1.png" );
    baker_imageFunction2D( multiscatterBRDF_retroReflectiveBump, 128, "output/brdf_FdR.png" );
}