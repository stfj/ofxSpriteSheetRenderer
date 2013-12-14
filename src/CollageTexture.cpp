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

#include "CollageTexture.h"

CollageTexture::~CollageTexture()
{
	delete[] collage;
}

void CollageTexture::allocate(int w, int h, int internalGlDataType, int internalGLScaleMode){
	allocate(w, h, internalGlDataType, internalGLScaleMode, ofGetUsingArbTex());
}

void CollageTexture::allocate(int w, int h, int internalGlDataType, int internalGLScaleMode, bool bUseARBExtention){
	
	if(internalGlDataType==GL_RGB)
		collage = new unsigned char[w*h*3];
	else if(internalGlDataType==GL_RGBA)
		collage = new unsigned char[w*h*4];
	
	c_width = w;
	c_height = h;
	
	if(internalGlDataType == GL_RGB)
		c_bpp=3;
	else if(internalGlDataType == GL_RGBA)
		c_bpp=4;
	
	for(int i=0;i<w*h*c_bpp;i++)
	{
		collage[i]=0;
	}
		
	c_type = internalGlDataType;
	
	//our graphics card might not support arb so we have to see if it is supported.
#ifndef TARGET_OPENGLES
	if (bUseARBExtention && GL_ARB_texture_rectangle){
		texData.tex_w = w;
		texData.tex_h = h;
		texData.tex_t = w;
		texData.tex_u = h;
		texData.textureTarget = GL_TEXTURE_RECTANGLE_ARB;
	} else 
#endif
	{
		//otherwise we need to calculate the next power of 2 for the requested dimensions
		//ie (320x240) becomes (512x256)
		texData.tex_w = ofNextPow2(w);
		texData.tex_h = ofNextPow2(h);
		texData.tex_t = 1.0f;
		texData.tex_u = 1.0f;
		texData.textureTarget = GL_TEXTURE_2D;
	}
	
	texData.glTypeInternal = internalGlDataType;
	
	
	// MEMO: todo, add more types
	switch(texData.glTypeInternal) {
#ifndef TARGET_OPENGLES	
		case GL_RGBA32F_ARB:
		case GL_RGBA16F_ARB:
			internal.glType		= GL_RGBA;
			internal.pixelType = GL_FLOAT;
			break;
			
		case GL_RGB32F_ARB:
			internal.glType = GL_RGB;
			internal.pixelType = GL_FLOAT;
			break;
			
		case GL_LUMINANCE32F_ARB:
			internal.glType = GL_LUMINANCE;
			internal.pixelType = GL_FLOAT;
			break;
#endif			
			
		default:
			internal.glType = GL_LUMINANCE;
			internal.pixelType = GL_UNSIGNED_BYTE;
	}
	
	// attempt to free the previous bound texture, if we can:
	clear();
	
	glGenTextures(1, (GLuint *)&texData.textureID);   // could be more then one, but for now, just one
	
	glEnable(texData.textureTarget);
	
	glBindTexture(texData.textureTarget, (GLuint)texData.textureID);
	
	glTexParameterf(texData.textureTarget, GL_TEXTURE_MAG_FILTER,  internalGLScaleMode);
	glTexParameterf(texData.textureTarget, GL_TEXTURE_MIN_FILTER, internalGLScaleMode);
	glTexParameterf(texData.textureTarget, GL_TEXTURE_WRAP_S, internalGLScaleMode);
	glTexParameterf(texData.textureTarget, GL_TEXTURE_WRAP_T, internalGLScaleMode);
	
#ifndef TARGET_OPENGLES
	// can't do this on OpenGL ES: on full-blown OpenGL, 
	// internalGlDataType and glDataType (GL_LUMINANCE below)
	// can be different; on ES they must be exactly the same.
	//		glTexImage2D(texData.textureTarget, 0, texData.glTypeInternal, (GLint)texData.tex_w, (GLint)texData.tex_h, 0, GL_LUMINANCE, PIXEL_TYPE, 0);  // init to black...
	glTexImage2D(texData.textureTarget, 0, texData.glTypeInternal, (GLint) texData.tex_w, (GLint) texData.tex_h, 0, internal.glType, internal.pixelType, 0);  // init to black...
#else
	glTexImage2D(texData.textureTarget, 0, texData.glTypeInternal, texData.tex_w, texData.tex_h, 0, texData.glTypeInternal, GL_UNSIGNED_BYTE, 0);
#endif
	
	
	
	
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	
	glDisable(texData.textureTarget);
	
	texData.width = w;
	texData.height = h;
	texData.bFlipTexture = false;
	texData.bAllocated = true;
}

void CollageTexture::pasteImage(int x, int y, string textureName, int glType)
{
	ofImage loader;
	loader.setUseTexture(false);
	loader.loadImage(textureName);
	
	pasteImage(x, y, loader.getWidth(), loader.getHeight(), loader.getPixels(), glType);
	
	loader.clear();
}

void CollageTexture::pasteImage(int x, int y, int w, int h, unsigned char * pxls, int glType)
{	
	int bpp;
	
	if(glType == GL_RGB)
		bpp=3;
	else if(glType == GL_RGBA)
		bpp=4;
	
	
	int collageIndex = getIndex(x,y);
	int sourceIndex = 0;
	int sourceRow = 0;
	
	int collageMax = c_width*c_height*c_bpp;
	int sourceMax = w*h*bpp;
	
	int startX = x;
		
	while(1)
	{
		
		if(x >= c_width || x-startX >= w) // if we're passed the edge of the collage or done with a row of the image, go to the next row
		{
			x = startX;
			y++;
			sourceRow++;
			sourceIndex = (sourceRow * w)*bpp;
			collageIndex = getIndex(x,y);
			
			if(collageIndex >= collageMax) //if we're passed the edge of the collage, break!
				break;
		}
		
		if(sourceIndex >= sourceMax) // have we managed to go past the end of any files?
			break;
		
		//if we're still good, copy in the pixels
		
		if(c_bpp == 4) // do we blend?
		{
			if( collage[collageIndex + 3] != 0) // something non transparent exists as this pixel, therefore: blend!
			{
				float dest_pct = (float)collage[collageIndex + 3]/255.0f;
				
				float source_pct  = 1;
				if(bpp==4)
					source_pct = (float)pxls[sourceIndex + 3]/255.0f;
				
				//additive blend
				collage[collageIndex    ] = (float)collage[collageIndex    ] * (1.0-source_pct) + (float)pxls[sourceIndex    ] * source_pct;
				collage[collageIndex + 1] = (float)collage[collageIndex + 1] * (1.0-source_pct) + (float)pxls[sourceIndex + 1] * source_pct;
				collage[collageIndex + 2] = (float)collage[collageIndex + 2] * (1.0-source_pct) + (float)pxls[sourceIndex + 2] * source_pct;
				
				collage[collageIndex + 3] = 255 * (dest_pct * (1.0-source_pct) + source_pct );
			}
			else // don't blend! (blending with nothing ends up diluting the color
			{
				collage[collageIndex    ] = pxls[sourceIndex    ];
				collage[collageIndex + 1] = pxls[sourceIndex + 1];
				collage[collageIndex + 2] = pxls[sourceIndex + 2];
				if(bpp==4)
					collage[collageIndex + 3] = pxls[sourceIndex + 3];
				else
					collage[collageIndex + 3] = 255;
			}
			
		}
		else // just copy straight, theres no blend data being stored
		{
			collage[collageIndex    ] = pxls[sourceIndex    ];
			collage[collageIndex + 1] = pxls[sourceIndex + 1];
			collage[collageIndex + 2] = pxls[sourceIndex + 2];
		}
		
		x++;
		collageIndex+=c_bpp;
		sourceIndex+=bpp;
	}
}

int CollageTexture::getIndex(int x, int y)
{
	return (y*c_width+x)*c_bpp;
}

void CollageTexture::finish()
{
	loadData(collage, c_width, c_height, c_type);
	//delete[] collage;
	//clear collage
	for(int i=0;i<c_width*c_height*c_bpp;i++)
		collage[i]=0;
}