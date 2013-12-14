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
 Collage Texture provides simple methods for allocating and filling a texture
 with the contents of arbitrary files. I wrote it so that seperate images
 could be combined into a single sprite sheet on the fly, but it could be
 used for any purpose. It handles alpha blending of pasted images as well.
 */

#ifndef OFX_COLLAGE_TEXTURE_H
#define OFX_COLLAGE_TEXTURE_H

#include "ofMain.h"

class CollageTexture : public ofTexture
{
	
public:
	
	~CollageTexture();
	
	void allocate(int w, int h, int internalGlDataType, int internalGLScaleMode); // scale mode is GL_LINEAR or GL_NEAREST if you want pixel scaling
	void allocate(int w, int h, int internalGlDataType, int internalGLScaleMode, bool bUseARBExtention);
	
	void pasteImage(int x, int y, string textureName, int glType=GL_RGBA);
	void pasteImage(int x, int y, int w, int h, unsigned char * pxls, int glType=GL_RGBA);
	
	void finish();
		
	unsigned char * collage;
	int c_width;
	int c_height;
	int c_bpp;
	int c_type;
	
protected:

	struct {
		GLenum glType;
		GLenum pixelType;
	} internal;

	int getIndex(int x, int y);
};

#endif