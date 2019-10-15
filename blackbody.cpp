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
#include "blackbody.h"

#include <iostream>
#include <unordered_map>
#include <csv-parser/parser.hpp>

using namespace glm;

// ref: https://www.shadertoy.com/view/4tVBWW
//
double blackbody_PlancksLaw( double l, double K )
{
    const double h = 6.626070040e-16;
    const double k = 1.38064852e-5;
    const double c = 299792458.0e9;
    
    const double o0 = 2e-3 * h * (c * c);
    const double o1 = h * c / k * 1.442695;
    
    double l5 = ( ( l * l ) * ( l * l ) ) * l;
    return o0 / ( l5 * ( exp2( o1 / ( l * K ) ) - 1.0 ) );
}

float pow2( float x ) { return x * x; }

// ref: https://www.shadertoy.com/view/4tVBWW
// Simple Analytic Approximations to the CIE XYZ Color Matching Functions (https://www.shadertoy.com/view/4ttBRB)
// https://research.nvidia.com/publication/simple-analytic-approximations-cie-xyz-color-matching-functions
//
vec3 nvFit_XYZ10( float l )
{
    vec3 xyz;

    xyz.x = 0.4 * exp2( -866.433976 * pow2( log2( l * 0.000986 + 0.56213 ) ) );
    xyz.x += 1.13 * exp2( -162.19644 * pow2( log2( l * -0.001345 + 1.799597 ) ) );
    xyz.y = 1.011 * exp2( -1.442695 * pow2( l * 0.015325 - 8.522368 ) );
    xyz.z = 2.06 * exp2( -22.18071 * pow2( log2( l * 0.005543 - 1.474501 ) ) );

    return xyz;
}

// ref: http://brucelindbloom.com/index.html?Eqn_RGB_XYZ_Matrix.html
//
vec3 XYZ_to_sRGB_D50( vec3 XYZ )
{
    float r = dot( XYZ, vec3(  3.2406, -1.5372, -0.4986 ) );
    float g = dot( XYZ, vec3( -0.9689,  1.8758,  0.0415 ) );
    float b = dot( XYZ, vec3(  0.0557, -0.2040,  1.0570 ) );
    return vec3( r, g, b );
}

vec4 blackbody_Tonemap_Unclamped( vec4 val )
{
    vec3 c = max( vec3( 0.0f ), vec3( val.x, val.y, val.z ) );
    c.x = c.x / ( 1.0f + c.x );
    c.y = c.y / ( 1.0f + c.y );
    c.z = c.z / ( 1.0f + c.z );
    c.x = pow( c.x, 1.0f / 2.2f );
    c.y = pow( c.y, 1.0f / 2.2f );
    c.z = pow( c.z, 1.0f / 2.2f );
    return vec4( c, val.w );
}

vec4 blackbody_Tonemap( vec4 val )
{
    return clamp( blackbody_Tonemap_Unclamped( val ), 0.0f, 1.0f );
}

// ref: https://www.shadertoy.com/view/4tVBWW
//
vec4 blackbody_Integrate_Cached( float temperature )
{
    static std::unordered_map< float, vec4 > s_cache;
    if ( s_cache.find( temperature ) != s_cache.end() ) {
        return s_cache[ temperature ];
    }
    const int NUM_SAMPLES = 10000;

    vec3 XYZ;
    for( int i = 0; i < NUM_SAMPLES; i++ ) {
        double wavelen = mix( 380.0f, 720.0f, double( i ) / double( NUM_SAMPLES - 1 ) );
        auto L = double( blackbody_PlancksLaw( wavelen, temperature ) );
        XYZ += nvFit_XYZ10( wavelen ) * float( L );
    }
    XYZ /= NUM_SAMPLES;

    auto c = XYZ_to_sRGB_D50( XYZ );

    printf( "    temperature %.0fK RGB = %f %f %f XYZ = %f %f %f\n", temperature, c.r, c.g, c.b, XYZ.x, XYZ.y, XYZ.z );

    s_cache[ temperature ] = vec4( c, 1.0f );
    return s_cache[ temperature ];
}

vec4 blackbody_Integrate( float x, float y )
{
    float temperature = mix( 1000, 20000, y );
    return blackbody_Tonemap( blackbody_Integrate_Cached( temperature ) );
}

void blackbody_GraphPlanck()
{
    printf("Baking Planck equation 500k into output/planck_5k.csv...\n" );
    float temperature = 5000.0f;
    FILE *fp = fopen( "output/planck_5k.csv", "w" );
    const int NUM_SAMPLES = 10000;
    for( int i = 0; i < NUM_SAMPLES; i++ ) {
        double wavelen = mix( 380.0f, 720.0f, double( i ) / double( NUM_SAMPLES - 1 ) );
        auto intensity = float( blackbody_PlancksLaw( wavelen, temperature ) );
        fprintf( fp, "%d,%f\n", i, intensity );
    }
    fclose( fp );
}

// Polynomial fit to sRGB-D60 of Planck equation integrated over NVIDIA fit of 10-degree XYZ data
//
vec3 blackbody_FitSRGB( float K )
{
    float x = clamp( ( ( K - 1000.0f ) / 19000.0f ), 0.0f, 1.0f );
    float x2 = x * x; float x3 = x2 * x; float x4 = x2 *x2; float x5 = x3 *x2; float x6 = x3 * x3;
    vec3 y = vec3(
        -0.029972778695974256f + 7.669083294646621f * x +  2.214401294577103f * x2 - 49.42474372087125f  * x3 + 103.53915061107702f * x4 - 89.5574774811751f   * x5 + 28.588440157789307f * x6,
        -0.0374521902577345f   + 5.613905742381423f * x + 16.608160380839113f * x2 - 88.52816707661721f  * x3 + 157.8362751079448f  * x4 - 127.4927963970656f  * x5 + 39.14505232781722f * x6,
        -0.21282402472640047f  + 5.342342069940462f * x + 23.583032531526992f * x2 - 106.91106640514953f * x3 + 181.48860203094765f * x4 - 143.00625735899658f * x5 + 43.27750211769455f  * x6
    );
    return vec3( pow( y.x, 5.0f ), pow( y.y, 5.0f ), pow( y.z, 5.0f ) );
}

void blackbody_GraphSRGB()
{
    printf("Baking Planck equation SRGB into output/planck_srgb.csv...\n" );
    FILE *fp = fopen( "output/planck_srgb.csv", "w" );

    bool IMPORTANCE_SAMPLE = true;
    const int NUM_TEMPERATURE_SAMPLES = 1024;

    for( int i = 0; i < NUM_TEMPERATURE_SAMPLES; i++ ) {
        float y = float( i ) / float( NUM_TEMPERATURE_SAMPLES - 1 );
        float temperature = mix( 1000, 20000, y );

        vec4 c = blackbody_Integrate_Cached( temperature );
        vec3 cf = blackbody_FitSRGB( temperature );

        auto signc = sign( c ); c = abs( c );
        c.x = pow( c.x, 0.2f ) * signc.x;
        c.y = pow( c.y, 0.2f ) * signc.y;
        c.z = pow( c.z, 0.2f ) * signc.z;

        fprintf( fp, "%f,%f,%f,%f,%f,%f,%f\n", y, c.x, c.y, c.z, cf.x, cf.y, cf.z );
    }

    fclose( fp );
}

void bake_blackBody()
{
    blackbody_GraphPlanck();
    blackbody_GraphSRGB();
    baker_imageFunction2D( blackbody_Integrate, 256, "output/planck_blackbody.png" );
}