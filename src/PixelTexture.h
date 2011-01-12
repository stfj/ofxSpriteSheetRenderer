/***********************************************************************
 
 Copyright (C) 2011 by Zach Gage
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 
 ************************************************************************/ 

/*
 Pixel Texture is a simple texture loader that will load an image into a gl
 texture without retaining a memory consuming array of pixels. when it scales
 the texture uses gl nearest to retain sharp pixel edges. For smooth scale
 interpolation, use LinearTexture.
 */

#ifndef OFX_PIXEL_TEXTURE_H
#define OFX_PIXEL_TEXTURE_H

#include "ofMain.h"

class PixelTexture : public ofTexture
{
public:
	void allocate(int w, int h, int internalGlDataType);
	void allocate(int w, int h, int internalGlDataType, bool bUseARBExtention);
	
	void loadTexture(string textureName, int glType=GL_RGBA);
};

#endif