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
#include "env_brdf.h"
#include "multiscatter_brdf.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

using namespace glm;

vec4 baker_testFunction( float x, float y )
{
    return vec4( 1.0f, 0.5f, 0.2f, 1.0f );
}

void baker_imageFunction2D( std::function< vec4( float x, float y ) > func, int res, std::string outputFileName )
{
    std::vector< vec4 > pixels;
    pixels.resize( res * res );
    
    printf( "Baking 2D image table %s ...\n", outputFileName.c_str() );
    for( int i = 0; i < res; i++ ) {
        for( int j = 0; j < res; j++ ) {
            float x = float( i ) / ( res - 1 );
            float y = float( j ) / ( res - 1 );
            pixels[i * res + j] = func( x, y );
        }
    }

    printf( "    Converting %s to uint8 ...\n", outputFileName.c_str() );
    std::vector< u8vec4 > pixels_u8;
    pixels_u8.resize( res * res );
    for( int i = 0; i < res; i++ ) {
        for( int j = 0; j < res; j++ ) {
            pixels_u8[i * res + j] = glm::clamp( pixels[i * res + j], vec4( 0.0f ), vec4( 1.0f ) ) * 255.0f;
        }
    }
    
    namespace fs = std::experimental::filesystem;
    auto ext = fs::path( outputFileName ).extension().u8string();
    
    printf( "    Writing %s ...\n", outputFileName.c_str() );
    if ( ext == ".png" ) {
        auto result = stbi_write_png( outputFileName.c_str(), res, res, 4, pixels_u8.data(), res * sizeof( u8vec4 ) );
        assert( result );
    } else if ( ext == ".bmp" ) {
        auto result = stbi_write_bmp( outputFileName.c_str(), res, res, 4, pixels_u8.data() );
        assert( result );
    } else if ( ext == ".tga" ) {
        auto result = stbi_write_tga( outputFileName.c_str(), res, res, 4, pixels_u8.data() );
        assert( result );
    } else if ( ext == ".hdr" ) {
        auto result = stbi_write_hdr( outputFileName.c_str(), res, res, 4, reinterpret_cast< float* >( pixels.data() ) );
        assert( result );
    } else {
        assert( !" Unknown file format!" );
    }
    printf( "    Output to %s OK.\n\n", outputFileName.c_str() );
}

int main()
{
    // baker_imageFunction2D( baker_testFunction, 64, "output/test_output.png" );
    // bake_multiscatterBRDF();
    bake_envBRDF();
}

