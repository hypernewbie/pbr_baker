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
#include "gloss_normal.h"
using namespace glm;

#define GLOSSNORMAL_SAMPLE_SIZE 8192

std::vector< float > s_glossToAvgNormalLength;

float glossNormal_IntegrateGlossNormalGGX( float gloss )
{
    vec3 N = vec3( 0, 0, 1 );
    float alpha2 = ggx_GlossToAlpha2( gloss );
    vec3 averageNormal = vec3( 0.0f );

    for( uint i = 0; i < GLOSSNORMAL_SAMPLE_SIZE; i++ )
    {
        auto xi = noise_getHammersleyAtIdx( i );
        auto H = ggx_ImportanceSampleGGX( xi, alpha2, N );
        averageNormal += H;
    }

    averageNormal /= GLOSSNORMAL_SAMPLE_SIZE;
    return length( averageNormal );
}

float glossNormal_NormalLengthToGloss( float normalLength )
{
    for( int i = 0; i < int( s_glossToAvgNormalLength.size() ) - 1; i++ ) {
        if ( s_glossToAvgNormalLength[ i + 1 ] > normalLength ) {
            return float( i ) / 255.0f;
        }
    }
    return 1.0f;
}

vec4 glossNormal_GenerateGlossCombineTable( float glossX, float glossY )
{
    // Need to call ggx_IntegrateGlossNormal to build table before calling this!
    assert( s_glossToAvgNormalLength.size() != 0 );

    auto glossIdxX = int( glossX * 255 );
    auto glossIdxY = int( glossY * 255 );
    float normalLenX = s_glossToAvgNormalLength[ glossIdxX ];
    float normalLenY = s_glossToAvgNormalLength[ glossIdxY ];
    float normanLenCombined = normalLenX * normalLenY;

    float combinedGloss = glossNormal_NormalLengthToGloss( normanLenCombined );
    return vec4( combinedGloss, combinedGloss, combinedGloss, 1.0f );
}

void bake_glossNormalTable()
{
    printf( "Baking gloss to avg normal length to output/gloss_normal_length.cpp...\n" );
    s_glossToAvgNormalLength.resize( 256 );

    // Bake normal lengths numerically.
    for( int i = 0; i < 256; i++ ) {
        float gloss = float ( i ) / 255.0f;
        s_glossToAvgNormalLength[i] = glossNormal_IntegrateGlossNormalGGX( gloss );
    }

    // Output to file as C code!
    FILE* fp = fopen( "output/gloss_normal_length.cpp", "w" );
    fprintf( fp, "static float s_averageGlossToNormalLength[] = {" );
    for( int i = 0; i < 256; i++ ) {
        if ( i % 16 == 0 ) {
            fprintf( fp, "\n    " );
        }
        fprintf( fp, "%.8ff, ", s_glossToAvgNormalLength[i] );
    }
    fprintf( fp, "\n};\n" );

    fclose( fp );

    // Output to file as CSV!
    fp = fopen( "output/gloss_normal_length.csv", "w" );
    for( int i = 0; i < 256; i++ ) {
        float gloss = float ( i ) / 255.0f;
        fprintf( fp, "%.4f,%.8f\n", gloss, s_glossToAvgNormalLength[i] );
    }
    fclose( fp );

    // Bake combined gloss table.
    baker_imageFunction2D( glossNormal_GenerateGlossCombineTable, 256, "output/gloss_combine.png" );
}