/*
 *  LinearTexture.cpp
 *  plsSendRenderer
 *
 *  Created by Zach Gage on 5/23/10.
 *  Copyright 2010 stfj. All rights reserved.
 *
 */

#include "LinearTexture.h"

void LinearTexture::loadImage(string textureName)
{
	ofImage loader;
	loader.setUseTexture(false);
	loader.loadImage(textureName);
	
	int glType = GL_RGB;
	
	if(loader.bpp==32){
		glType = GL_RGBA;

	}
	
	allocate(loader.getWidth(), loader.getHeight(), glType);
	loadData(loader.getPixels(), loader.getWidth(), loader.getHeight(), glType);
	
	loader.clear();
}

void LinearTexture::loadTexture(string textureName, int glType)
{
	ofImage loader;
	loader.setUseTexture(false);
	loader.loadImage(textureName);
	allocate(loader.getWidth(), loader.getHeight(), glType);
	loadData(loader.getPixels(), loader.getWidth(), loader.getHeight(), glType);
	
	loader.clear();
}

void LinearTexture::allocate(int w, int h, int internalGlDataType){
	allocate(w, h, internalGlDataType, ofGetUsingArbTex());
}

void LinearTexture::allocate(int w, int h, int internalGlDataType, bool bUseARBExtention){
	
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
			internal.glType = GL_RGBA;
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
	
	glTexParameterf(texData.textureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(texData.textureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(texData.textureTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(texData.textureTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
#ifndef TARGET_OPENGLES
	// can't do this on OpenGL ES: on full-blown OpenGL, 
	// internalGlDataType and glDataType (GL_LUMINANCE below)
	// can be different; on ES they must be exactly the same.
	//		glTexImage2D(texData.textureTarget, 0, texData.glTypeInternal, (GLint)texData.tex_w, (GLint)texData.tex_h, 0, GL_LUMINANCE, PIXEL_TYPE, 0);  // init to black...
	glTexImage2D(texData.textureTarget, 0, texData.glTypeInternal, (GLint) texData.tex_w, (GLint) texData.tex_h, 0, internal.glType, internal.pixelType, 0);  // init to black...
#else
	glTexImage2D(texData.textureTarget, 0, texData.glTypeInternal, texData.tex_w, texData.tex_h, 0, internal.glTypeInternal, GL_UNSIGNED_BYTE, 0);
#endif
	
	
	
	
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	
	glDisable(texData.textureTarget);
	
	texData.width = w;
	texData.height = h;
	texData.bFlipTexture = false;
	texData.bAllocated = true;
	
	width=w;
	height=h;
}
