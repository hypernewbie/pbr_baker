# PBR Baker

Simple open source multi-functional baking tool for PBR material ralted work, inspired by various techniques from the SIGGRAPH 2018 presentation "Material Advances in Call of Duty: WWII" by Danny Chan.

Written in mostly portable C++. Uses glm and stb libraries.

## Features
* Environment BRDF lookup table - Karis'13
* Simple noise texture generation ( only white noise for now )
* Black body radiation table
* GGX gloss to average normal length table bake - Chan'18
* GGX gloss combine lookup texture bake - Chan'18

## Usage
```
Simple open source multi-functional baking tool for PBR material related work.
Usage:
  pbr_baker [OPTION...]

  -m, --multiscatter_brdf  Bake multi-scatter BRDF components.
  -e, --env_brdf           Bake GGX NDF - environment BRDF table.
  -n, --noise              Output some noise textures.
  -b, --blackbody          Bake black body radiation lookup table and .
  -g, --gloss_normal       Bake gloss average normal table and gloss blend
                           table.
  -t, --test               Test random functionality.
  -h, --help               Display help
```

## Compiling
1. Install Visual Studio 2017 with C++ support
2. Open pbr_baker.sln
3. Compile and run!

## License
```
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
```