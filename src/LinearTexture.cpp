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

#include "LinearTexture.h"
#include "FreeImage.h"
void PFileImage::loadFromPFS(string textureName)
{   
    if(PHYSFS_exists(textureName.c_str())){
        PHYSFS_File *file = PHYSFS_openRead(textureName.c_str());
        char *myBuf;
        myBuf = new char[PHYSFS_fileLength(file)];
        int length_read = PHYSFS_read (file, myBuf, 1, PHYSFS_fileLength(file));
        PHYSFS_close(file);
        
        FIMEMORY *hmem = NULL;   
        hmem = FreeImage_OpenMemory((Byte *)myBuf, length_read);   
        if (hmem == NULL){ printf("couldn't create memory handle! \n"); return; }   

        bool bLoaded = false;
        FIBITMAP * bmp = NULL;
        
        FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
        fif = FreeImage_GetFileTypeFromMemory(hmem);
        if((fif != FIF_UNKNOWN) && FreeImage_FIFSupportsReading(fif)) {
            bmp = FreeImage_LoadFromMemory(fif, hmem, 0);
            if (bmp != NULL){
                bLoaded = true;
            }
        }
        //-----------------------------
//        ofPixels pixels;

        if ( bLoaded ){
            putBmpIntoPixels(bmp, pixels, false);
            pixels.swapRgb();
        }
        
        FreeImage_FlipVertical(bmp);
        
        if (bmp != NULL){
            FreeImage_Unload(bmp);
        }
        FreeImage_CloseMemory(hmem);  
        
        if (bmp == NULL){ printf("couldn't create bmp! \n"); return; }   
        update();   
    }
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
void LinearTexture::loadTextureFromPVR(string textureName, int glType)
{
    
}
void LinearTexture::loadTextureFromPFS(string textureName, int  glType)
{
	PFileImage loader;
	loader.setUseTexture(false);
	loader.loadFromPFS(textureName);
	allocate(loader.getWidth(), loader.getHeight(), glType);
	loadData(loader.getPixels(), loader.getWidth(), loader.getHeight(), glType);
	loader.clear();
}

void LinearTexture::allocate(int w, int h, int internalGlDataType){
	allocate(w, h, internalGlDataType, false);
}

void LinearTexture::allocate(int w, int h, int internalGlDataType, bool bUseARBExtention){
	
	//our graphics card might not support arb so we have to see if it is supported.
#ifndef TARGET_OPENGLES
	if (bUseARBExtention && GL_ARB_texture_rectangle){
		texData.tex_w = w;
		texData.tex_h = h;
		texData.tex_t = w;
		texData.tex_u = h;
		//texData.textureTarget = GL_TEXTURE_RECTANGLE_ARB;
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
			texData.glType		= GL_RGBA;
			texData.pixelType	= GL_FLOAT;
			break;
			
		case GL_RGB32F_ARB:
			texData.glType		= GL_RGB;
			texData.pixelType	= GL_FLOAT;
			break;
			
		case GL_LUMINANCE32F_ARB:
			texData.glType		= GL_LUMINANCE;
			texData.pixelType	= GL_FLOAT;
			break;
#endif			
			
		default:
			texData.glType		= GL_LUMINANCE;
			texData.pixelType	= GL_UNSIGNED_BYTE;
	}
	
	// attempt to free the previous bound texture, if we can:
	clear();
	
	glGenTextures(1, (GLuint *)&texData.textureID);   // could be more then one, but for now, just one
	
	glEnable(texData.textureTarget);

	glBindTexture(texData.textureTarget, (GLuint)texData.textureID);
    glTexParameteri(texData.textureTarget, GL_GENERATE_MIPMAP, GL_TRUE);

    GLenum err;
    err = glGetError();
    if (err != GL_NO_ERROR)
    {
        cout << "1Error setting up texture " << err << endl;
    }

	glTexParameterf(texData.textureTarget, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(texData.textureTarget, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(texData.textureTarget, 0, texData.glTypeInternal, texData.tex_w, texData.tex_h, 0, texData.glTypeInternal, GL_UNSIGNED_BYTE, 0);
    err = glGetError();
    if (err != GL_NO_ERROR)
    {
        cout << "2Error setting up texture " << err << endl;
    }
	glTexParameterf(texData.textureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    err = glGetError();
    if (err != GL_NO_ERROR)
    {
        cout << "3Error setting up texture " << err << endl;
    }
	glTexParameterf(texData.textureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    err = glGetError();
    if (err != GL_NO_ERROR)
    {
        cout << "Error setting up texture " << err << endl;
    }

	
	
	
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	
	glDisable(texData.textureTarget);
	
	texData.width = w;
	texData.height = h;
	texData.bFlipTexture = false;
	texData.bAllocated = true;
}
