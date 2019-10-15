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
#include "noise.h"
using namespace glm;

#include <random>

vec4 noisegen_whiteNoise( float x, float y )
{
    static std::random_device s_randomDevice;
    static std::mt19937 s_mersenneTwisterEngine( s_randomDevice() );
    static std::uniform_real_distribution< float > s_uniformDist( 0.0f, 1.0f );
    return vec4(
        s_uniformDist( s_mersenneTwisterEngine ),
        s_uniformDist( s_mersenneTwisterEngine ),
        s_uniformDist( s_mersenneTwisterEngine ),
        1.0f
    );
}

void bake_noiseTextures()
{
    baker_imageFunction2D( noisegen_whiteNoise, 128, "output/whiteNoise.png" );
}