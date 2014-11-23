/***********************************************************************
 
 Copyright (C) 2011 by Zach Gage and Ramsey Nasser
 
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

#include "ofxSpriteSheetRenderer.h"

float _sin[360];
float _cos[360];

ofxSpriteSheetRenderer::ofxSpriteSheetRenderer(int _numLayers, int _tilesPerLayer, int _defaultLayer, int _tileSize)
{
	texture = NULL;
	verts = NULL;
	coords = NULL;
	colors = NULL;
	numSprites = NULL;
	
	textureIsExternal = false;
	
	safeMode = true;
	
	gameTime = ofGetElapsedTimeMillis();
	
	reAllocateArrays(_numLayers, _tilesPerLayer, _defaultLayer, _tileSize);
		
	for(int i=0;i<360;i++)
	{
		_sin[i] = sin(ofMap(i,0,360,0,TWO_PI));
		_cos[i] = cos(ofMap(i,0,360,0,TWO_PI));
	}
	
	//shape stuff
	setCircleResolution(22);
	shapeR=shapeG=shapeB=shapeA=255;
	brushIndex = -1;
}

ofxSpriteSheetRenderer::~ofxSpriteSheetRenderer()
{
	if(texture != NULL && textureIsExternal)
		texture->clear();
	
	if(verts != NULL)
		delete[] verts;
	if(coords != NULL)
		delete[] coords;
	if(colors != NULL)
		delete[] colors;
	if(numSprites != NULL)
		delete[] numSprites;
}

void ofxSpriteSheetRenderer::reAllocateArrays(int _numLayers, int _tilesPerLayer, int _defaultLayer, int _tileSize)
{
	numLayers = _numLayers;
	tileSize = _tileSize;
	tilesPerLayer = _tilesPerLayer;
	defaultLayer = _defaultLayer;
	
	if(verts != NULL)
		delete[] verts;
	if(coords != NULL)
		delete[] coords;
	if(colors != NULL)
		delete[] colors;
	if(numSprites != NULL)
		delete[] numSprites;
	
	verts = new float[numLayers*tilesPerLayer*18];
	coords = new float[numLayers*tilesPerLayer*12];
	colors = new unsigned char[numLayers*tilesPerLayer*24];
	numSprites = new int[numLayers];
	
	clear();
	clearTexture();
}

void ofxSpriteSheetRenderer::clearTexture()
{
	if(texture != NULL)
	{
		if(textureIsExternal)
			texture = NULL; // just remove the refernece
		else
		{
			delete texture;
			texture = NULL;
		}
	}
}

void ofxSpriteSheetRenderer::allocate(int widthHeight, int internalGLScaleMode)
{
	if(texture == NULL)
	{
		tileSize_f = tileSize;
		#ifdef TARGET_OPENGLES	// if we don't have arb, it's crazy important that things are power of 2 so that this float is set properly
		tileSize_f /= widthHeight;
		#endif
		
		spriteSheetWidth = widthHeight/tileSize;
		
		CollageTexture * newTexture = new CollageTexture();
		
		newTexture->allocate(widthHeight, widthHeight, GL_RGBA, internalGLScaleMode);
		
		texture = (ofTexture*) newTexture;
	}
	else
		cerr<<"cannot double allocate ofxSpriteSheetRenderer Texture! Please clearTexture() first"<<endl;
}

void ofxSpriteSheetRenderer::addMisc(string fileName, int x, int y, int glType)
{
	CollageTexture *cTexture = dynamic_cast<CollageTexture*>(texture);
	cTexture->pasteImage(x, y, fileName, glType);
}

void ofxSpriteSheetRenderer::finishTexture()
{
	CollageTexture *cTexture = dynamic_cast<CollageTexture*>(texture);
	cTexture->finish();
}

void ofxSpriteSheetRenderer::loadTexture(string fileName, int widthHeight, int internalGLScaleMode)
{
	clearTexture();
	clear();
	allocate(widthHeight, internalGLScaleMode);
	addMisc(fileName, 0, 0);
	finishTexture();
	textureIsExternal = false;
}

void ofxSpriteSheetRenderer::loadTexture(ofTexture * _texture)
{
	clear();
	clearTexture();
	textureIsExternal = true;
	texture = _texture;
}

void ofxSpriteSheetRenderer::loadTexture(CollageTexture * _texture){
	loadTexture((ofTexture *)_texture);}

void ofxSpriteSheetRenderer::loadTexture(PixelTexture * _texture){
	loadTexture((ofTexture *)_texture);}

void ofxSpriteSheetRenderer::loadTexture(LinearTexture * _texture){
	loadTexture((ofTexture *)_texture);}

void ofxSpriteSheetRenderer::clear(int layer)
{
	if(layer==-1)
		for(int i = 0; i < numLayers; i++) numSprites[i] = 0;
	else
		numSprites[layer]=0;
}

// nasser added, not trying to match zach's godless coding style
bool ofxSpriteSheetRenderer::addTile(animation_t* sprite, float x, float y, int layer, flipDirection f, int r, int g, int b, int alpha) {
	int index, frame;
	
	if(layer==-1)
		layer=defaultLayer;
	
	// animation
	if(sprite->total_frames > 1)
		// still animating
		if(sprite->loops != 0)
			// time to advance frame
			if(gameTime > sprite->next_tick) {
				sprite->frame += sprite->frame_skip;
				// increment frame and keep it within range
				if(sprite->frame < 0) sprite->frame = sprite->total_frames;
				if(sprite->frame >= sprite->total_frames) sprite->frame = 0;
				sprite->next_tick = gameTime + sprite->frame_duration;
				// decrement loop count if cycle complete
				if( ((sprite->frame_skip > 0 && sprite->frame == sprite->total_frames-1) || (sprite->frame_skip < 0 && sprite->frame == 0)) && sprite->loops > 0) sprite->loops--;
			}
	
	if(sprite->loops == 0 && sprite->final_index >= 0) {
		index = sprite->final_index;
		frame = 0;
	} else {
		index = sprite->index;
		frame = sprite->frame;
	}
	
	return addTile(index, frame, x, y, layer, sprite->w, sprite->h, f, r, g, b, alpha);
}

bool ofxSpriteSheetRenderer::addRotatedTile(animation_t* sprite, float x, float y, float rX, float rY, int layer, flipDirection f, float scale, int rot, CollisionBox_t* collisionBox, int r, int g, int b, int alpha){
	int index, frame;
	
	if(layer==-1)
		layer=defaultLayer;
	
	// animation
	if(sprite->total_frames > 1)
		// still animating
		if(sprite->loops != 0)
			// time to advance frame
			if(gameTime > sprite->next_tick) {
				sprite->frame += sprite->frame_skip;
				// increment frame and keep it within range
				if(sprite->frame < 0) sprite->frame = sprite->total_frames;
				if(sprite->frame >= sprite->total_frames) sprite->frame = 0;
				sprite->next_tick = gameTime + sprite->frame_duration;
				// decrement loop count if cycle complete
				if( ((sprite->frame_skip > 0 && sprite->frame == sprite->total_frames-1) || (sprite->frame_skip < 0 && sprite->frame == 0)) && sprite->loops > 0) sprite->loops--;
			}
	
	if(sprite->loops == 0 && sprite->final_index >= 0) {
		index = sprite->final_index;
		frame = 0;
	} else {
		index = sprite->index;
		frame = sprite->frame;
	}
	
	return addRotatedTile(index, frame, x, y, rX, rY, layer, sprite->w, sprite->h, f, scale, rot, collisionBox, r, g, b, alpha);
}

bool ofxSpriteSheetRenderer::addCenteredTile(animation_t* sprite, float x, float y, int layer, flipDirection f, float scale, int r, int g, int b, int alpha) {
	int index, frame;
	
	if(layer==-1)
		layer=defaultLayer;
	
	// animation
	if(sprite->total_frames > 1)
		// still animating
		if(sprite->loops != 0)
			// time to advance frame
			if(gameTime > sprite->next_tick) {
				sprite->frame += sprite->frame_skip;
				// increment frame and keep it within range
				if(sprite->frame < 0) sprite->frame = sprite->total_frames;
				if(sprite->frame >= sprite->total_frames) sprite->frame = 0;
				sprite->next_tick = gameTime + sprite->frame_duration;
				// decrement loop count if cycle complete
				if( ((sprite->frame_skip > 0 && sprite->frame == sprite->total_frames-1) || (sprite->frame_skip < 0 && sprite->frame == 0)) && sprite->loops > 0) sprite->loops--;
			}
	
	if(sprite->loops == 0 && sprite->final_index >= 0) {
		index = sprite->final_index;
		frame = 0;
	} else {
		index = sprite->index;
		frame = sprite->frame;
	}
	
	return addCenteredTile(index, frame, x, y, layer, sprite->w, sprite->h, f, scale, r, g, b, alpha);
}

bool ofxSpriteSheetRenderer::addCenterRotatedTile(animation_t* sprite, float x, float y, int layer, flipDirection f, float scale, int rot, CollisionBox_t* collisionBox, int r, int g, int b, int alpha){
	int index, frame;
	
	if(layer==-1)
		layer=defaultLayer;
	
	// animation
	if(sprite->total_frames > 1)
		// still animating
		if(sprite->loops != 0)
			// time to advance frame
			if(gameTime > sprite->next_tick) {
				sprite->frame += sprite->frame_skip;
				// increment frame and keep it within range
				if(sprite->frame < 0) sprite->frame = sprite->total_frames;
				if(sprite->frame >= sprite->total_frames) sprite->frame = 0;
				sprite->next_tick = gameTime + sprite->frame_duration;
				// decrement loop count if cycle complete
				if( ((sprite->frame_skip > 0 && sprite->frame == sprite->total_frames-1) || (sprite->frame_skip < 0 && sprite->frame == 0)) && sprite->loops > 0) sprite->loops--;
			}
	
	if(sprite->loops == 0 && sprite->final_index >= 0) {
		index = sprite->final_index;
		frame = 0;
	} else {
		index = sprite->index;
		frame = sprite->frame;
	}
	
	return addCenterRotatedTile(index, frame, x, y, layer, sprite->w, sprite->h, f, scale, rot, collisionBox, r, g, b, alpha);
}

bool ofxSpriteSheetRenderer::addTile(int tile_name, int frame, float x, float y, int layer, float w, float h, flipDirection f, int r, int g, int b, int alpha)
{
	if(layer==-1)
		layer=defaultLayer;
	
	if(texture == NULL)
	{
		cerr << "RENDER ERROR: No texture loaded!"  << endl;
		return false;
	}
	
	if(numSprites[layer] >= tilesPerLayer)
	{
		cerr << "RENDER ERROR: Layer " << layer << " over allocated! Max " << tilesPerLayer << " sprites per layer!"  << endl;
		return false;
	}
	
	if(layer > numLayers)
	{
		cerr << "RENDER ERROR: Bogus layer '" << layer << "'! Only " << numLayers << " layers compiled!"  << endl;
		return false;
	}
	
	float frameX;
	float frameY;
	int layerOffset = layer*tilesPerLayer;
	int vertexOffset = (layerOffset + numSprites[layer])*18;
	int colorOffset = (layerOffset + numSprites[layer])*24;
	
	getFrameXandY(tile_name, frameX, frameY);
	
	frameX += frame*w*tileSize_f;
	
	addTexCoords(f, frameX, frameY, layer, w, h);
	
	w*=tileSize;
	h*=tileSize;
	
	//verticies ------------------------------------
	verts[vertexOffset     ] = x;
	verts[vertexOffset + 1 ] = y;
	verts[vertexOffset + 2 ] = 0;
	
	verts[vertexOffset + 3 ] = x+w;
	verts[vertexOffset + 4 ] = y;
	verts[vertexOffset + 5 ] = 0;
	
	verts[vertexOffset + 6 ] = x;
	verts[vertexOffset + 7 ] = y+h;
	verts[vertexOffset + 8 ] = 0;
	
	
	
	verts[vertexOffset + 9 ] = x+w;
	verts[vertexOffset + 10] = y;
	verts[vertexOffset + 11] = 0;
	
	verts[vertexOffset + 12] = x;
	verts[vertexOffset + 13] = y+h;
	verts[vertexOffset + 14] = 0;
	
	verts[vertexOffset + 15] = x+w;
	verts[vertexOffset + 16] = y+h;
	verts[vertexOffset + 17] = 0;
	
	//colors ---------------------------------------
	
	colors[colorOffset	 ] = r;
	colors[colorOffset + 1 ] = g;
	colors[colorOffset + 2 ] = b;
	colors[colorOffset + 3 ] = alpha;
	
	colors[colorOffset + 4 ] = r;
	colors[colorOffset + 5 ] = g;
	colors[colorOffset + 6 ] = b;
	colors[colorOffset + 7 ] = alpha;
	
	colors[colorOffset + 8 ] = r;
	colors[colorOffset + 9 ] = g;
	colors[colorOffset + 10] = b;
	colors[colorOffset + 11] = alpha;
	
	
	
	colors[colorOffset + 12] = r;
	colors[colorOffset + 13] = g;
	colors[colorOffset + 14] = b;
	colors[colorOffset + 15] = alpha;
	
	colors[colorOffset + 16] = r;
	colors[colorOffset + 17] = g;
	colors[colorOffset + 18] = b;
	colors[colorOffset + 19] = alpha;
	
	colors[colorOffset + 20] = r;
	colors[colorOffset + 21] = g;
	colors[colorOffset + 22] = b;
	colors[colorOffset + 23] = alpha;
	
	//----------------------------------------------
	
	numSprites[layer]++;
	
	return true;
}

bool ofxSpriteSheetRenderer::addRotatedTile(int tile_name, int frame, float x, float y, float rX, float rY, int layer, float w, float h, flipDirection f, float scale, int rot, CollisionBox_t* collisionBox, int r, int g, int b, int alpha)
{
	if(layer==-1)
		layer=defaultLayer;
	
	if(texture == NULL)
	{
		cerr << "RENDER ERROR: No texture loaded!"  << endl;
		return false;
	}
	
	if(numSprites[layer] >= tilesPerLayer)
	{
		cerr << "RENDER ERROR: Layer " << layer << " over allocated! Max " << tilesPerLayer << " sprites per layer!"  << endl;
		return false;
	}
	
	if(layer > numLayers)
	{
		cerr << "RENDER ERROR: Bogus layer '" << layer << "'! Only " << numLayers << " layers compiled!"  << endl;
		return false;
	}
	
	float frameX;
	float frameY;
	int layerOffset = layer*tilesPerLayer;
	int vertexOffset = (layerOffset + numSprites[layer])*18;
	int colorOffset = (layerOffset + numSprites[layer])*24;
	
	getFrameXandY(tile_name, frameX, frameY);
	
	frameX += frame*w*tileSize_f;
	//add a check here to make animations wrap around
	
	addTexCoords(f, frameX, frameY, layer, w, h);
	
	w*=scale*tileSize;
	h*=scale*tileSize;
	
	float nW = w * -rX;
	float nH = h * -rY;
	float W = w*(1.0-rX);
	float H = h*(1.0-rY);
	
	//verticies ------------------------------------
	verts[vertexOffset     ] = x + getX(nW, nH,  rot);//x+scale*rotA->ul[rot  ]; //ul ur ll
	verts[vertexOffset + 1 ] = y + getY(nW, nH,  rot);//y+scale*rotA->ul[rot+1];
	verts[vertexOffset + 2 ] = 0;
	
	verts[vertexOffset + 3 ] = x + getX(W, nH,  rot);//x+scale*rotA->ur[rot  ];
	verts[vertexOffset + 4 ] = y + getY(W, nH,  rot);//y+scale*rotA->ur[rot+1];
	verts[vertexOffset + 5 ] = 0;
	
	verts[vertexOffset + 6 ] = x + getX(nW, H,  rot);//x+scale*rotA->ll[rot  ];
	verts[vertexOffset + 7 ] = y + getY(nW, H,  rot);//y+scale*rotA->ll[rot+1];
	verts[vertexOffset + 8 ] = 0;
	
	verts[vertexOffset + 9 ] = x + getX(W, nH,  rot);//x+scale*rotA->ur[rot  ]; //ur ll lr
	verts[vertexOffset + 10] = y + getY(W, nH,  rot);//y+scale*rotA->ur[rot+1];
	verts[vertexOffset + 11] = 0;
	
	verts[vertexOffset + 12] = x + getX(nW, H,  rot);//x+scale*rotA->ll[rot  ];
	verts[vertexOffset + 13] = y + getY(nW, H,  rot);//y+scale*rotA->ll[rot+1];
	verts[vertexOffset + 14] = 0;
	
	verts[vertexOffset + 15] = x + getX(W, H,  rot);//x+scale*rotA->lr[rot  ];
	verts[vertexOffset + 16] = y + getY(W, H,  rot);//y+scale*rotA->lr[rot+1];
	verts[vertexOffset + 17] = 0;
	verts[vertexOffset + 17] = 0;
	//colors ---------------------------------------
	
	colors[colorOffset	 ] = r;
	colors[colorOffset + 1 ] = g;
	colors[colorOffset + 2 ] = b;
	colors[colorOffset + 3 ] = alpha;
	
	colors[colorOffset + 4 ] = r;
	colors[colorOffset + 5 ] = g;
	colors[colorOffset + 6 ] = b;
	colors[colorOffset + 7 ] = alpha;
	
	colors[colorOffset + 8 ] = r;
	colors[colorOffset + 9 ] = g;
	colors[colorOffset + 10] = b;
	colors[colorOffset + 11] = alpha;
	
	
	
	colors[colorOffset + 12] = r;
	colors[colorOffset + 13] = g;
	colors[colorOffset + 14] = b;
	colors[colorOffset + 15] = alpha;
	
	colors[colorOffset + 16] = r;
	colors[colorOffset + 17] = g;
	colors[colorOffset + 18] = b;
	colors[colorOffset + 19] = alpha;
	
	colors[colorOffset + 20] = r;
	colors[colorOffset + 21] = g;
	colors[colorOffset + 22] = b;
	colors[colorOffset + 23] = alpha;
	
	//pass back collision box-----------------------
	
	if(collisionBox != NULL)
	{
		collisionBox->UL.set(verts[vertexOffset     ], verts[vertexOffset + 1 ]);
		collisionBox->UR.set(verts[vertexOffset + 3 ], verts[vertexOffset + 4 ]);
		collisionBox->LL.set(verts[vertexOffset + 6 ], verts[vertexOffset + 7 ]);
		collisionBox->LR.set(verts[vertexOffset + 15], verts[vertexOffset + 16]);
	}
	
	//----------------------------------------------
	numSprites[layer]++;
	
	return true;
}

bool ofxSpriteSheetRenderer::addCenteredTile(int tile_name, int frame, float x, float y, int layer, float w, float h, flipDirection f, float scale, int r, int g, int b, int alpha)
{
	if(layer==-1)
		layer=defaultLayer;
	
	if(texture == NULL)
	{
		cerr << "RENDER ERROR: No texture loaded!"  << endl;
		return false;
	}
	
	if(numSprites[layer] >= tilesPerLayer)
	{
		cerr << "RENDER ERROR: Layer " << layer << " over allocated! Max " << tilesPerLayer << " sprites per layer!"  << endl;
		return false;
	}
	
	if(layer > numLayers)
	{
		cerr << "RENDER ERROR: Bogus layer '" << layer << "'! Only " << numLayers << " layers compiled!"  << endl;
		return false;
	}
	
	float frameX;
	float frameY;
	int layerOffset = layer*tilesPerLayer;
	int vertexOffset = (layerOffset + numSprites[layer])*18;
	int colorOffset = (layerOffset + numSprites[layer])*24;
	
	getFrameXandY(tile_name, frameX, frameY);
	
	frameX += frame*w*tileSize_f;
	
	addTexCoords(f, frameX, frameY, layer, w, h);
	
	//rot*=2;
	
	w*=tileSize*scale;
	w/=2;
	h*=tileSize*scale;
	h/=2;
	
	//verticies ------------------------------------
	verts[vertexOffset     ] = x-w; //ul ur ll
	verts[vertexOffset + 1 ] = y-h;
	verts[vertexOffset + 2 ] = 0;
	
	verts[vertexOffset + 3 ] = x+w;
	verts[vertexOffset + 4 ] = y-h;
	verts[vertexOffset + 5 ] = 0;
	
	verts[vertexOffset + 6 ] = x-w;
	verts[vertexOffset + 7 ] = y+h;
	verts[vertexOffset + 8 ] = 0;
	
	
	
	verts[vertexOffset + 9 ] = x+w; //ur ll lr
	verts[vertexOffset + 10] = y-h;
	verts[vertexOffset + 11] = 0;
	
	verts[vertexOffset + 12] = x-w;
	verts[vertexOffset + 13] = y+h;
	verts[vertexOffset + 14] = 0;
	
	verts[vertexOffset + 15] = x+w;
	verts[vertexOffset + 16] = y+h;
	verts[vertexOffset + 17] = 0;
	
	//colors ---------------------------------------
	
	colors[colorOffset	 ] = r;
	colors[colorOffset + 1 ] = g;
	colors[colorOffset + 2 ] = b;
	colors[colorOffset + 3 ] = alpha;
	
	colors[colorOffset + 4 ] = r;
	colors[colorOffset + 5 ] = g;
	colors[colorOffset + 6 ] = b;
	colors[colorOffset + 7 ] = alpha;
	
	colors[colorOffset + 8 ] = r;
	colors[colorOffset + 9 ] = g;
	colors[colorOffset + 10] = b;
	colors[colorOffset + 11] = alpha;
	
	
	
	colors[colorOffset + 12] = r;
	colors[colorOffset + 13] = g;
	colors[colorOffset + 14] = b;
	colors[colorOffset + 15] = alpha;
	
	colors[colorOffset + 16] = r;
	colors[colorOffset + 17] = g;
	colors[colorOffset + 18] = b;
	colors[colorOffset + 19] = alpha;
	
	colors[colorOffset + 20] = r;
	colors[colorOffset + 21] = g;
	colors[colorOffset + 22] = b;
	colors[colorOffset + 23] = alpha;
	
	//----------------------------------------------
	
	numSprites[layer]++;
	
	return true;
}

bool ofxSpriteSheetRenderer::addCenterRotatedTile(int tile_name, int frame, float x, float y, int layer, float w, float h, flipDirection f, float scale, int rot, CollisionBox_t* collisionBox, int r, int g, int b, int alpha)
{
	if(layer==-1)
		layer=defaultLayer;
	
	if(texture == NULL)
	{
		cerr << "RENDER ERROR: No texture loaded!"  << endl;
		return false;
	}
	
	if(numSprites[layer] >= tilesPerLayer)
	{
		cerr << "RENDER ERROR: Layer " << layer << " over allocated! Max " << tilesPerLayer << " sprites per layer!"  << endl;
		return false;
	}
	
	if(layer > numLayers)
	{
		cerr << "RENDER ERROR: Bogus layer '" << layer << "'! Only " << numLayers << " layers compiled!"  << endl;
		return false;
	}
	
	float frameX;
	float frameY;
	int layerOffset = layer*tilesPerLayer;
	int vertexOffset = (layerOffset + numSprites[layer])*18;
	int colorOffset = (layerOffset + numSprites[layer])*24;
	
	getFrameXandY(tile_name, frameX, frameY);
	
	frameX += frame*w*tileSize_f;
	//add a check here to make animations wrap around
	
	addTexCoords(f, frameX, frameY, layer, w, h);
	
	
	//these two should never be called
	//
	/*
	while(rot<0)
		rot+=360;
	
	while(rot>359)
		rot-=360;
	 */
	//
	
	
	//rot*=2;
	
	//w*=scale;
	//h*=scale;
	
	/*RotationInformation_t * rotA = &rotationArrays[0];
	int s;
	for(int i=0;i<rotationArrays.size();i++)
	{
		if(rotationArrays[i].w == w && rotationArrays[i].h ==h) // is the array the proper size?
		{
			rotA = &rotationArrays[i];
			i=rotationArrays.size();
		}
		else //check to see if this array scaled will work
		{
			s = w/rotationArrays[i].w;
			if(rotationArrays[i].h*s == h)
			{
				scale*=s;
				rotA = &rotationArrays[i];
				i=rotationArrays.size();
			}
		}
	}*/

	w*=scale*tileSize;
	w/=2;
	
	h*=scale*tileSize;
	h/=2;
	
	//verticies ------------------------------------
	verts[vertexOffset     ] = x + getX(-w, -h,  rot);//x+scale*rotA->ul[rot  ]; //ul ur ll
	verts[vertexOffset + 1 ] = y + getY(-w, -h,  rot);//y+scale*rotA->ul[rot+1];
	verts[vertexOffset + 2 ] = 0;
	
	verts[vertexOffset + 3 ] = x + getX(w, -h,  rot);//x+scale*rotA->ur[rot  ];
	verts[vertexOffset + 4 ] = y + getY(w, -h,  rot);//y+scale*rotA->ur[rot+1];
	verts[vertexOffset + 5 ] = 0;
	
	verts[vertexOffset + 6 ] = x + getX(-w, h,  rot);//x+scale*rotA->ll[rot  ];
	verts[vertexOffset + 7 ] = y + getY(-w, h,  rot);//y+scale*rotA->ll[rot+1];
	verts[vertexOffset + 8 ] = 0;
	
	verts[vertexOffset + 9 ] = x + getX(w, -h,  rot);//x+scale*rotA->ur[rot  ]; //ur ll lr
	verts[vertexOffset + 10] = y + getY(w, -h,  rot);//y+scale*rotA->ur[rot+1];
	verts[vertexOffset + 11] = 0;
	
	verts[vertexOffset + 12] = x + getX(-w, h,  rot);//x+scale*rotA->ll[rot  ];
	verts[vertexOffset + 13] = y + getY(-w, h,  rot);//y+scale*rotA->ll[rot+1];
	verts[vertexOffset + 14] = 0;
	
	verts[vertexOffset + 15] = x + getX(w, h,  rot);//x+scale*rotA->lr[rot  ];
	verts[vertexOffset + 16] = y + getY(w, h,  rot);//y+scale*rotA->lr[rot+1];
	verts[vertexOffset + 17] = 0;
	//colors ---------------------------------------
	
	colors[colorOffset	 ] = r;
	colors[colorOffset + 1 ] = g;
	colors[colorOffset + 2 ] = b;
	colors[colorOffset + 3 ] = alpha;
	
	colors[colorOffset + 4 ] = r;
	colors[colorOffset + 5 ] = g;
	colors[colorOffset + 6 ] = b;
	colors[colorOffset + 7 ] = alpha;
	
	colors[colorOffset + 8 ] = r;
	colors[colorOffset + 9 ] = g;
	colors[colorOffset + 10] = b;
	colors[colorOffset + 11] = alpha;
	
	
	
	colors[colorOffset + 12] = r;
	colors[colorOffset + 13] = g;
	colors[colorOffset + 14] = b;
	colors[colorOffset + 15] = alpha;
	
	colors[colorOffset + 16] = r;
	colors[colorOffset + 17] = g;
	colors[colorOffset + 18] = b;
	colors[colorOffset + 19] = alpha;
	
	colors[colorOffset + 20] = r;
	colors[colorOffset + 21] = g;
	colors[colorOffset + 22] = b;
	colors[colorOffset + 23] = alpha;
	
	//pass back collision box-----------------------
	
	if(collisionBox != NULL)
	{
		collisionBox->UL.set(verts[vertexOffset     ], verts[vertexOffset + 1 ]);
		collisionBox->UR.set(verts[vertexOffset + 3 ], verts[vertexOffset + 4 ]);
		collisionBox->LL.set(verts[vertexOffset + 6 ], verts[vertexOffset + 7 ]);
		collisionBox->LR.set(verts[vertexOffset + 15], verts[vertexOffset + 16]);
	}
	
	//----------------------------------------------
	numSprites[layer]++;
	
	return true;
}

void ofxSpriteSheetRenderer::update(unsigned long time)
{
	gameTime = time;
}

void ofxSpriteSheetRenderer::draw(int startLayer, int endLayer)
{	
	if(endLayer == -1)
		endLayer = numLayers-1;
	if(safeMode)
	{
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	}
	
	glVertexPointer(3, GL_FLOAT, 0, &verts[0]);
	glColorPointer(4, GL_UNSIGNED_BYTE, 0, &colors[0]);
	glTexCoordPointer(2, GL_FLOAT, 0, &coords[0]);
	
	texture->bind();
	for(int i = startLayer; i <= endLayer; i++)
		if(numSprites[i] > 0)
			glDrawArrays(GL_TRIANGLES, i*tilesPerLayer*6, numSprites[i]*6);
	texture->unbind();
	
	if(safeMode)
	{
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}
}

void ofxSpriteSheetRenderer::addTexCoords(flipDirection f, float &frameX, float &frameY, int layer, float w, float h)
{
	int layerOffset = layer*tilesPerLayer;
	int coordOffset = (layerOffset + numSprites[layer])*12;
	
	w*=tileSize_f;
	h*=tileSize_f;
	
	switch (f) {
		case F_NONE:
			coords[coordOffset     ] = frameX;
			coords[coordOffset + 1 ] = frameY;
			
			coords[coordOffset + 2 ] = frameX+w;
			coords[coordOffset + 3 ] = frameY;
			
			coords[coordOffset + 4 ] = frameX;
			coords[coordOffset + 5 ] = frameY+h;
			
			
			
			coords[coordOffset + 6 ] = frameX+w;
			coords[coordOffset + 7 ] = frameY;
			
			coords[coordOffset + 8 ] = frameX;
			coords[coordOffset + 9 ] = frameY+h;
			
			coords[coordOffset + 10] = frameX+w;
			coords[coordOffset + 11] = frameY+h;
			
			break;
		case F_HORIZ:
			coords[coordOffset     ] = frameX+w;
			coords[coordOffset + 1 ] = frameY;
			
			coords[coordOffset + 2 ] = frameX;
			coords[coordOffset + 3 ] = frameY;
			
			coords[coordOffset + 4 ] = frameX+w;
			coords[coordOffset + 5 ] = frameY+h;
			
			
			
			coords[coordOffset + 6 ] = frameX;
			coords[coordOffset + 7 ] = frameY;
			
			coords[coordOffset + 8 ] = frameX+w;
			coords[coordOffset + 9 ] = frameY+h;
			
			coords[coordOffset + 10] = frameX;
			coords[coordOffset + 11] = frameY+h;
			
			break;
		case F_VERT:
			coords[coordOffset     ] = frameX;
			coords[coordOffset + 1 ] = frameY+h;
			
			coords[coordOffset + 2 ] = frameX+w;
			coords[coordOffset + 3 ] = frameY+h;
			
			coords[coordOffset + 4 ] = frameX;
			coords[coordOffset + 5 ] = frameY;
			
			
			
			coords[coordOffset + 6 ] = frameX+w;
			coords[coordOffset + 7 ] = frameY+h;
			
			coords[coordOffset + 8 ] = frameX;
			coords[coordOffset + 9 ] = frameY;
			
			coords[coordOffset + 10] = frameX+w;
			coords[coordOffset + 11] = frameY;
			
			break;
		case F_HORIZ_VERT:
			coords[coordOffset     ] = frameX+w;
			coords[coordOffset + 1 ] = frameY+h;
			
			coords[coordOffset + 2 ] = frameX;
			coords[coordOffset + 3 ] = frameY+h;
			
			coords[coordOffset + 4 ] = frameX+w;
			coords[coordOffset + 5 ] = frameY;
			
			
			
			coords[coordOffset + 6 ] = frameX;
			coords[coordOffset + 7 ] = frameY+h;
			
			coords[coordOffset + 8 ] = frameX+w;
			coords[coordOffset + 9 ] = frameY;
			
			coords[coordOffset + 10] = frameX;
			coords[coordOffset + 11] = frameY;
			
			break;
		default:
			break;
	}
}

bool ofxSpriteSheetRenderer::addLine(ofVec2f a, ofVec2f b, int width, int layer, bool capped)
{
	if(layer==-1)
		layer=defaultLayer;
	
	if(texture == NULL)
	{
		cerr << "RENDER ERROR: No texture loaded!"  << endl;
		return false;
	}
	
	if(numSprites[layer] >= tilesPerLayer)
	{
		cerr << "RENDER ERROR: Layer " << layer << " over allocated! Max " << tilesPerLayer << " sprites per layer!"  << endl;
		return false;
	}
	
	if(layer > numLayers)
	{
		cerr << "RENDER ERROR: Bogus layer '" << layer << "'! Only " << numLayers << " layers compiled!"  << endl;
		return false;
	}
	
	if(brushIndex < 0)
	{
		cerr << "RENDER ERROR: brush not initialized"  << endl;
		return false;
	}
	
	
	//based off http://answers.oreilly.com/topic/1669-how-to-render-anti-aliased-lines-with-textures-in-ios-4/
 	ofVec2f e = (b - a).normalized() * width;
	
 	ofVec2f N = ofVec2f(-e.y, e.x);
 	ofVec2f S = -N;
 	ofVec2f NE = N + e;
 	ofVec2f NW = N - e;
 	ofVec2f SW = -NE;
 	ofVec2f SE = -NW;
	
	int layerOffset = layer*tilesPerLayer;
	int vertexOffset = (layerOffset + numSprites[layer])*18;
	int colorOffset = (layerOffset + numSprites[layer])*24;
	int coordOffset = (layerOffset + numSprites[layer])*12;
	
	/*
	 destVertex++->Position = a + SW;
	 
	 destVertex++->Position = a + NW;
	 
	 destVertex++->Position = a + S;
	 
	 destVertex++->Position = a + N;
	 
	 destVertex++->Position = b + S;
	 
	 destVertex++->Position = b + N;
	 
	 destVertex++->Position = b + SE;
	 
	 destVertex++->Position = b + NE;
	 */
	
	//verticies ------------------------------------
	verts[vertexOffset     ]	= a.x+N.x; // uncapped line aN, aN, aS
	verts[vertexOffset + 1 ]	= a.y+N.y;
	verts[vertexOffset + 2 ]	= 0;
	
	verts[vertexOffset + 3 ]	= b.x+N.x;
	verts[vertexOffset + 4 ]	= b.y+N.y;
	verts[vertexOffset + 5 ]	= 0;
	
	verts[vertexOffset + 6 ]	= a.x+S.x;
	verts[vertexOffset + 7 ]	= a.y+S.y;
	verts[vertexOffset + 8 ]	= 0;
	
	
	verts[vertexOffset + 9 ]	= a.x + S.x; // uncapped line aS, aN, bS
	verts[vertexOffset + 10]	= a.y + S.y;
	verts[vertexOffset + 11]	= 0;
	
	verts[vertexOffset + 12]	= b.x + N.x;
	verts[vertexOffset + 13]	= b.y + N.y;
	verts[vertexOffset + 14]	= 0;
	
	verts[vertexOffset + 15]	= b.x + S.x;
	verts[vertexOffset + 16]	= b.y + S.y;
	verts[vertexOffset + 17]	= 0;
	
	if(capped){
		verts[vertexOffset + 18]	= a.x + NW.x; // a cap 1 aNW, bN, aSW
		verts[vertexOffset + 19]	= a.y + NW.y;
		verts[vertexOffset + 20]	= 0;
		
		verts[vertexOffset + 21]	= a.x + N.x;
		verts[vertexOffset + 22]	= a.y + N.y;
		verts[vertexOffset + 23]	= 0;
		
		verts[vertexOffset + 24]	= a.x + SW.x;
		verts[vertexOffset + 25]	= a.y + SW.y;
		verts[vertexOffset + 26]	= 0;
		
		verts[vertexOffset + 27]	= a.x + SW.x; // a cap 2 line aSW, bN, aS
		verts[vertexOffset + 28]	= a.y + SW.y;
		verts[vertexOffset + 29]	= 0;
		
		verts[vertexOffset + 30]	= a.x + N.x;
		verts[vertexOffset + 31]	= a.y + N.y;
		verts[vertexOffset + 32]	= 0;
		
		verts[vertexOffset + 33]	= a.x + S.x;
		verts[vertexOffset + 34]	= a.y + S.y;
		verts[vertexOffset + 35]	= 0;
		
		
		
		verts[vertexOffset + 36]	= b.x + N.x; // b cap 1 bN, bNE, bS
		verts[vertexOffset + 37]	= b.y + N.y;
		verts[vertexOffset + 38]	= 0;
		
		verts[vertexOffset + 39]	= b.x + NE.x;
		verts[vertexOffset + 40]	= b.y + NE.y;
		verts[vertexOffset + 41]	= 0;
		
		verts[vertexOffset + 42]	= b.x + S.x;
		verts[vertexOffset + 43]	= b.y + S.y;
		verts[vertexOffset + 44]	= 0;
		
		verts[vertexOffset + 45]	= b.x + S.x; // b cap 2 line bS, bNE, bSE
		verts[vertexOffset + 46]	= b.y + S.y;
		verts[vertexOffset + 47]	= 0;
		
		verts[vertexOffset + 48]	= b.x + NE.x;
		verts[vertexOffset + 49]	= b.y + NE.y;
		verts[vertexOffset + 50]	= 0;
		
		verts[vertexOffset + 51]	= b.x + SE.x;
		verts[vertexOffset + 52]	= b.y + SE.y;
		verts[vertexOffset + 53]	= 0;
	}
	
	//colors ---------------------------------------
	colors[colorOffset	   ]	= shapeR;			//ul
	colors[colorOffset + 1 ]	= shapeG;
	colors[colorOffset + 2 ]	= shapeB;
	colors[colorOffset + 3 ]	= shapeA;
	
	colors[colorOffset + 4 ]	= shapeR;			//ur
	colors[colorOffset + 5 ]	= shapeG;
	colors[colorOffset + 6 ]	= shapeB;
	colors[colorOffset + 7 ]	= shapeA;
	
	colors[colorOffset + 8 ]	= shapeR;			//ll
	colors[colorOffset + 9 ]	= shapeG;
	colors[colorOffset + 10]	= shapeB;
	colors[colorOffset + 11]	= shapeA;
	
	
	colors[colorOffset + 12]	= shapeR;			//ur
	colors[colorOffset + 13]	= shapeG;
	colors[colorOffset + 14]	= shapeB;
	colors[colorOffset + 15]	= shapeA;
	
	colors[colorOffset + 16]	= shapeR;			//ll
	colors[colorOffset + 17]	= shapeG;
	colors[colorOffset + 18]	= shapeB;
	colors[colorOffset + 19]	= shapeA;
	
	colors[colorOffset + 20]	= shapeR;			//lr
	colors[colorOffset + 21]	= shapeG;
	colors[colorOffset + 22]	= shapeB;
	colors[colorOffset + 23]	= shapeA;
	
	if(capped){
	
		colors[colorOffset + 24]	= shapeR;			//left cap t1
		colors[colorOffset + 25]	= shapeG;
		colors[colorOffset + 26]	= shapeB;
		colors[colorOffset + 27]	= shapeA;
		
		colors[colorOffset + 28]	= shapeR;			
		colors[colorOffset + 29]	= shapeG;
		colors[colorOffset + 30]	= shapeB;
		colors[colorOffset + 31]	= shapeA;
		
		colors[colorOffset + 32]	= shapeR;			
		colors[colorOffset + 33]	= shapeG;
		colors[colorOffset + 34]	= shapeB;
		colors[colorOffset + 35]	= shapeA;
		
		
		colors[colorOffset + 36]	= shapeR;			//left cap t2
		colors[colorOffset + 37]	= shapeG;
		colors[colorOffset + 38]	= shapeB;
		colors[colorOffset + 39]	= shapeA;
		
		colors[colorOffset + 40]	= shapeR;			
		colors[colorOffset + 41]	= shapeG;
		colors[colorOffset + 42]	= shapeB;
		colors[colorOffset + 43]	= shapeA;
		
		colors[colorOffset + 44]	= shapeR;			
		colors[colorOffset + 45]	= shapeG;
		colors[colorOffset + 46]	= shapeB;
		colors[colorOffset + 47]	= shapeA;
		
		
		
		colors[colorOffset + 48]	= shapeR;			//right cap t1
		colors[colorOffset + 49]	= shapeG;
		colors[colorOffset + 50]	= shapeB;
		colors[colorOffset + 51]	= shapeA;
		
		colors[colorOffset + 52]	= shapeR;			
		colors[colorOffset + 53]	= shapeG;
		colors[colorOffset + 54]	= shapeB;
		colors[colorOffset + 55]	= shapeA;
		
		colors[colorOffset + 56]	= shapeR;			
		colors[colorOffset + 57]	= shapeG;
		colors[colorOffset + 58]	= shapeB;
		colors[colorOffset + 59]	= shapeA;
		
		
		colors[colorOffset + 60]	= shapeR;			//right cap t2
		colors[colorOffset + 61]	= shapeG;
		colors[colorOffset + 62]	= shapeB;
		colors[colorOffset + 63]	= shapeA;
		
		colors[colorOffset + 64]	= shapeR;			
		colors[colorOffset + 65]	= shapeG;
		colors[colorOffset + 66]	= shapeB;
		colors[colorOffset + 67]	= shapeA;
		
		colors[colorOffset + 68]	= shapeR;			
		colors[colorOffset + 69]	= shapeG;
		colors[colorOffset + 70]	= shapeB;
		colors[colorOffset + 71]	= shapeA;
	}
	
	//texture --------------------------------------
	
	coords[coordOffset     ] = brushX+halfBrushSize; //N
	coords[coordOffset + 1 ] = brushY;
	
	coords[coordOffset + 2 ] = brushX+halfBrushSize; //N
	coords[coordOffset + 3 ] = brushY;
	
	coords[coordOffset + 4 ] = brushX+halfBrushSize; //S
	coords[coordOffset + 5 ] = brushY+brushSize;
	
	
	
	coords[coordOffset + 6 ] = brushX+halfBrushSize; //S
	coords[coordOffset + 7 ] = brushY+brushSize;
	
	coords[coordOffset + 8 ] = brushX+halfBrushSize; //N
	coords[coordOffset + 9 ] = brushY;
	
	coords[coordOffset + 10] = brushX+halfBrushSize; //S
	coords[coordOffset + 11] = brushY+brushSize;
	
	if(capped){
		
		// a cap 1 aNW, aN, aSW
		coords[coordOffset + 12] = brushX; //NW
		coords[coordOffset + 13] = brushY;
		
		coords[coordOffset + 14] = brushX+halfBrushSize; //N
		coords[coordOffset + 15] = brushY;
		
		coords[coordOffset + 16] = brushX; //SW
		coords[coordOffset + 17] = brushY+brushSize;
		
		// a cap 2 line aSW, aN, aS
		coords[coordOffset + 18] = brushX; //SW
		coords[coordOffset + 19] = brushY+brushSize;
		
		coords[coordOffset + 20] = brushX+halfBrushSize; //N
		coords[coordOffset + 21] = brushY;
		
		coords[coordOffset + 22] = brushX+halfBrushSize; //S
		coords[coordOffset + 23] = brushY+brushSize;
		
		// b cap 1 bN, bNE, bS
		coords[coordOffset + 24] = brushX+halfBrushSize; //N
		coords[coordOffset + 25] = brushY;
		
		coords[coordOffset + 26] = brushX+brushSize; //NE
		coords[coordOffset + 27] = brushY;
		
		coords[coordOffset + 28] = brushX+halfBrushSize; //S
		coords[coordOffset + 29] = brushY+brushSize;
		
		// b cap 2 line bS, bNE, bSE
		coords[coordOffset + 30] = brushX+halfBrushSize; //S
		coords[coordOffset + 31] = brushY+brushSize;
		
		coords[coordOffset + 32] = brushX+brushSize; //NE
		coords[coordOffset + 33] = brushY;
		
		coords[coordOffset + 34] = brushX+brushSize; //SE
		coords[coordOffset + 35] = brushY+brushSize;
	}
	
	numSprites[layer]++; // sprite == 2 triangles
	
	if(capped)
		numSprites[layer]+=2; // one more 'sprite' for each cap
	
	return true;
}

bool ofxSpriteSheetRenderer::addCircle(float x, float y, float z, float radius, int width, bool filled, int layer)
{
	if(layer==-1)
		layer=defaultLayer;
	
	if(texture == NULL)
	{
		cerr << "RENDER ERROR: No texture loaded!"  << endl;
		return false;
	}
	
	if(numSprites[layer] >= tilesPerLayer)
	{
		cerr << "RENDER ERROR: Layer " << layer << " over allocated! Max " << tilesPerLayer << " sprites per layer!"  << endl;
		return false;
	}
	
	if(layer > numLayers)
	{
		cerr << "RENDER ERROR: Bogus layer '" << layer << "'! Only " << numLayers << " layers compiled!"  << endl;
		return false;
	}
	
	if(brushIndex < 0)
	{
		cerr << "RENDER ERROR: brush not initialized"  << endl;
		return false;
	}
	
	if(filled)
	{
		int k = 0;
		
		for(int i=0;i<numCirclePts;i++)
		{
			int layerOffset = layer*tilesPerLayer;
			int vertexOffset = (layerOffset + numSprites[layer])*18;
			int colorOffset = (layerOffset + numSprites[layer])*24;
			int coordOffset = (layerOffset + numSprites[layer])*12;
			
			//triangle 1 -----------------------------------------------------------------------
			if(i>0)
			{
				//verticies ------------------------------------
				verts[vertexOffset     ]	= x + circlePts[k] * radius  * 0.5;
				verts[vertexOffset + 1 ]	= y + circlePts[k+1] * radius  * 0.5;
				verts[vertexOffset + 2 ]	= z;
				
				verts[vertexOffset + 3 ]	= x + circlePts[k-2] * radius  * 0.5;
				verts[vertexOffset + 4 ]	= y + circlePts[k-1] * radius  * 0.5;
				verts[vertexOffset + 5 ]	= z;
				
				verts[vertexOffset + 6 ]	= x;
				verts[vertexOffset + 7 ]	= y;
				verts[vertexOffset + 8 ]	= z;
			}
			else if(i==0)
			{
				//verticies ------------------------------------
				verts[vertexOffset     ]	= x + circlePts[k] * radius  * 0.5;
				verts[vertexOffset + 1 ]	= y + circlePts[k+1] * radius  * 0.5;
				verts[vertexOffset + 2 ]	= z;
				
				verts[vertexOffset + 3 ]	= x + circlePts[numCirclePts*2-2] * radius  * 0.5;
				verts[vertexOffset + 4 ]	= y + circlePts[numCirclePts*2-1] * radius  * 0.5;
				verts[vertexOffset + 5 ]	= z;
				
				verts[vertexOffset + 6 ]	= x;
				verts[vertexOffset + 7 ]	= y;
				verts[vertexOffset + 8 ]	= z;
			}
			
			//triangle 2 -----------------------------------------------------------------------
			//verticies ------------------------------------
			verts[vertexOffset + 9 ]	= x + circlePts[k] * radius  * 0.5;
			verts[vertexOffset + 10]	= y + circlePts[k+1] * radius  * 0.5;
			verts[vertexOffset + 11]	= z;
				
			verts[vertexOffset + 12]	= x + circlePts[k-2] * radius  * 0.5;
			verts[vertexOffset + 13]	= y + circlePts[k-1] * radius  * 0.5;
			verts[vertexOffset + 14]	= z;
			
			verts[vertexOffset + 15]	= x;
			verts[vertexOffset + 16]	= y;
			verts[vertexOffset + 17]	= z;
		
			
			//colors ---------------------------------------
			colors[colorOffset	   ]	= shapeR;			//ul
			colors[colorOffset + 1 ]	= shapeG;
			colors[colorOffset + 2 ]	= shapeB;
			colors[colorOffset + 3 ]	= shapeA;
			
			colors[colorOffset + 4 ]	= shapeR;			//ur
			colors[colorOffset + 5 ]	= shapeG;
			colors[colorOffset + 6 ]	= shapeB;
			colors[colorOffset + 7 ]	= shapeA;
			
			colors[colorOffset + 8 ]	= shapeR;			//ll
			colors[colorOffset + 9 ]	= shapeG;
			colors[colorOffset + 10]	= shapeB;
			colors[colorOffset + 11]	= shapeA;
			
			
			colors[colorOffset + 12]	= shapeR;			//ur
			colors[colorOffset + 13]	= shapeG;
			colors[colorOffset + 14]	= shapeB;
			colors[colorOffset + 15]	= shapeA;
			
			colors[colorOffset + 16]	= shapeR;			//ll
			colors[colorOffset + 17]	= shapeG;
			colors[colorOffset + 18]	= shapeB;
			colors[colorOffset + 19]	= shapeA;
			
			colors[colorOffset + 20]	= shapeR;			//lr
			colors[colorOffset + 21]	= shapeG;
			colors[colorOffset + 22]	= shapeB;
			colors[colorOffset + 23]	= shapeA;
			
			
			//texture --------------------------------------------
			coords[coordOffset     ] = brushX+halfBrushSize;
			coords[coordOffset + 1 ] = brushY+halfBrushSize;
			
			coords[coordOffset + 2 ] = brushX+halfBrushSize;//+w;
			coords[coordOffset + 3 ] = brushY+halfBrushSize;
			
			coords[coordOffset + 4 ] = brushX+halfBrushSize;
			coords[coordOffset + 5 ] = brushY+halfBrushSize;//+h;
			
			coords[coordOffset + 6 ] = brushX+halfBrushSize;//+w;
			coords[coordOffset + 7 ] = brushY+halfBrushSize;
			
			coords[coordOffset + 8 ] = brushX+halfBrushSize;
			coords[coordOffset + 9 ] = brushY+halfBrushSize;//+h;
			
			coords[coordOffset + 10] = brushX+halfBrushSize;//+w;
			coords[coordOffset + 11] = brushY+halfBrushSize;//+h;
			
			numSprites[layer]++;
			k+=2;
		}
	} else
	{
		int k = 0;
		int internalRadius = radius-width;
		radius+=width;
		
		for(int i=0;i<numCirclePts;i++)
		{
			int layerOffset = layer*tilesPerLayer;
			int vertexOffset = (layerOffset + numSprites[layer])*18;
			int colorOffset = (layerOffset + numSprites[layer])*24;
			int coordOffset = (layerOffset + numSprites[layer])*12;
			
			if(i>0)
			{
				//verticies ------------------------------------
				verts[vertexOffset     ]	= x + circlePts[k] * radius  * 0.5;
				verts[vertexOffset + 1 ]	= y + circlePts[k+1] * radius  * 0.5;
				verts[vertexOffset + 2 ]	= z;
				
				verts[vertexOffset + 3 ]	= x + circlePts[k-2] * radius  * 0.5;
				verts[vertexOffset + 4 ]	= y + circlePts[k-1] * radius  * 0.5;
				verts[vertexOffset + 5 ]	= z;
				
				verts[vertexOffset + 6 ]	= x + circlePts[k] * internalRadius  * 0.5;
				verts[vertexOffset + 7 ]	= y + circlePts[k+1] * internalRadius  * 0.5;
				verts[vertexOffset + 8 ]	= z;
                
                //second triangle                
				verts[vertexOffset + 9 ]	= x + circlePts[k-2] * radius  * 0.5;
				verts[vertexOffset + 10]	= y + circlePts[k-1] * radius  * 0.5;
				verts[vertexOffset + 11]	= z;
				
				verts[vertexOffset + 12]	= x + circlePts[k-2] * internalRadius  * 0.5;
				verts[vertexOffset + 13]	= y + circlePts[k-1] * internalRadius  * 0.5;
				verts[vertexOffset + 14]	= z;
				
				verts[vertexOffset + 15]	= x + circlePts[k] * internalRadius  * 0.5;
				verts[vertexOffset + 16]	= y + circlePts[k+1] * internalRadius  * 0.5;
				verts[vertexOffset + 17]	= z;
			}
			else if(i==0)
			{
				//verticies ------------------------------------
				verts[vertexOffset     ]	= x + circlePts[k] * radius  * 0.5;
				verts[vertexOffset + 1 ]	= y + circlePts[k+1] * radius  * 0.5;
				verts[vertexOffset + 2 ]	= z;
				
				verts[vertexOffset + 3 ]	= x + circlePts[numCirclePts*2-2] * radius  * 0.5;
				verts[vertexOffset + 4 ]	= y + circlePts[numCirclePts*2-1] * radius  * 0.5;
				verts[vertexOffset + 5 ]	= z;
				
				verts[vertexOffset + 6 ]	= x + circlePts[k] * internalRadius  * 0.5;;
				verts[vertexOffset + 7 ]	= y + circlePts[k+1] * internalRadius  * 0.5;;
				verts[vertexOffset + 8 ]	= z;
                
                //second triangle ------------------------------------
				verts[vertexOffset + 9 ]	= x + circlePts[numCirclePts*2-2] * radius  * 0.5;
				verts[vertexOffset + 10]	= y + circlePts[numCirclePts*2-1] * radius  * 0.5;
				verts[vertexOffset + 11]	= z;
				
				verts[vertexOffset + 12]	= x + circlePts[numCirclePts*2-2] * internalRadius  * 0.5;
				verts[vertexOffset + 13]	= y + circlePts[numCirclePts*2-1] * internalRadius  * 0.5;
				verts[vertexOffset + 14]	= z;
				
				verts[vertexOffset + 15]	= x + circlePts[k] * internalRadius  * 0.5;;
				verts[vertexOffset + 16]	= y + circlePts[k+1] * internalRadius  * 0.5;;
				verts[vertexOffset + 17]	= z;
			}
			
			//colors ---------------------------------------
			colors[colorOffset	   ]	= shapeR;			//ul
			colors[colorOffset + 1 ]	= shapeG;
			colors[colorOffset + 2 ]	= shapeB;
			colors[colorOffset + 3 ]	= shapeA;
			
			colors[colorOffset + 4 ]	= shapeR;			//ur
			colors[colorOffset + 5 ]	= shapeG;
			colors[colorOffset + 6 ]	= shapeB;
			colors[colorOffset + 7 ]	= shapeA;
			
			colors[colorOffset + 8 ]	= shapeR;			//ll
			colors[colorOffset + 9 ]	= shapeG;
			colors[colorOffset + 10]	= shapeB;
			colors[colorOffset + 11]	= shapeA;
			
			
			colors[colorOffset + 12]	= shapeR;			//ur
			colors[colorOffset + 13]	= shapeG;
			colors[colorOffset + 14]	= shapeB;
			colors[colorOffset + 15]	= shapeA;
			
			colors[colorOffset + 16]	= shapeR;			//ll
			colors[colorOffset + 17]	= shapeG;
			colors[colorOffset + 18]	= shapeB;
			colors[colorOffset + 19]	= shapeA;
			
			colors[colorOffset + 20]	= shapeR;			//lr
			colors[colorOffset + 21]	= shapeG;
			colors[colorOffset + 22]	= shapeB;
			colors[colorOffset + 23]	= shapeA;
			
			//texture --------------------------------------------			
			coords[coordOffset     ] = brushX+halfBrushSize; //N
			coords[coordOffset + 1 ] = brushY;
			
			coords[coordOffset + 2 ] = brushX+halfBrushSize; //N
			coords[coordOffset + 3 ] = brushY;
			
			coords[coordOffset + 4 ] = brushX+halfBrushSize; //S
			coords[coordOffset + 5 ] = brushY+brushSize;
			
			
			
			coords[coordOffset + 6 ] = brushX+halfBrushSize; //S
			coords[coordOffset + 7 ] = brushY;
			
			coords[coordOffset + 8 ] = brushX+halfBrushSize; //N
			coords[coordOffset + 9 ] = brushY+brushSize;
			
			coords[coordOffset + 10] = brushX+halfBrushSize; //S
			coords[coordOffset + 11] = brushY+brushSize;
			
			numSprites[layer]+=2;
			k+=2;
		}
	}
}

bool ofxSpriteSheetRenderer::addRect(float x, float y, float z, float w, float h, int layer)
{
	if(layer==-1)
		layer=defaultLayer;
	
	if(texture == NULL)
	{
		cerr << "RENDER ERROR: No texture loaded!"  << endl;
		return false;
	}
	
	if(numSprites[layer] >= tilesPerLayer)
	{
		cerr << "RENDER ERROR: Layer " << layer << " over allocated! Max " << tilesPerLayer << " sprites per layer!"  << endl;
		return false;
	}
	
	if(layer > numLayers)
	{
		cerr << "RENDER ERROR: Bogus layer '" << layer << "'! Only " << numLayers << " layers compiled!"  << endl;
		return false;
	}
	
	if(brushIndex < 0)
	{
		cerr << "RENDER ERROR: brush not initialized"  << endl;
		return false;
	}
	
	int layerOffset = layer*tilesPerLayer;
	int vertexOffset = (layerOffset + numSprites[layer])*18;
	int colorOffset = (layerOffset + numSprites[layer])*24;
	int coordOffset = (layerOffset + numSprites[layer])*12;
	
	//verticies ------------------------------------
	verts[vertexOffset     ]	= x;
	verts[vertexOffset + 1 ]	= y;
	verts[vertexOffset + 2 ]	= z;
	
	verts[vertexOffset + 3 ]	= x + w;
	verts[vertexOffset + 4 ]	= y;
	verts[vertexOffset + 5 ]	= z;
	
	verts[vertexOffset + 6 ]	= x;
	verts[vertexOffset + 7 ]	= y + h;
	verts[vertexOffset + 8 ]	= z;
	
	
	verts[vertexOffset + 9 ]	= x + w;
	verts[vertexOffset + 10]	= y;
	verts[vertexOffset + 11]	= z;
	
	verts[vertexOffset + 12]	= x;
	verts[vertexOffset + 13]	= y + h;
	verts[vertexOffset + 14]	= z;
	
	verts[vertexOffset + 15]	= x + w;
	verts[vertexOffset + 16]	= y + h;
	verts[vertexOffset + 17]	= z;
	
	//colors ---------------------------------------
	colors[colorOffset	   ]	= shapeR;			//ul
	colors[colorOffset + 1 ]	= shapeG;
	colors[colorOffset + 2 ]	= shapeB;
	colors[colorOffset + 3 ]	= shapeA;
	
	colors[colorOffset + 4 ]	= shapeR;			//ur
	colors[colorOffset + 5 ]	= shapeG;
	colors[colorOffset + 6 ]	= shapeB;
	colors[colorOffset + 7 ]	= shapeA;
	
	colors[colorOffset + 8 ]	= shapeR;			//ll
	colors[colorOffset + 9 ]	= shapeG;
	colors[colorOffset + 10]	= shapeB;
	colors[colorOffset + 11]	= shapeA;
	
	
	colors[colorOffset + 12]	= shapeR;			//ur
	colors[colorOffset + 13]	= shapeG;
	colors[colorOffset + 14]	= shapeB;
	colors[colorOffset + 15]	= shapeA;
	
	colors[colorOffset + 16]	= shapeR;			//ll
	colors[colorOffset + 17]	= shapeG;
	colors[colorOffset + 18]	= shapeB;
	colors[colorOffset + 19]	= shapeA;
	
	colors[colorOffset + 20]	= shapeR;			//lr
	colors[colorOffset + 21]	= shapeG;
	colors[colorOffset + 22]	= shapeB;
	colors[colorOffset + 23]	= shapeA;
	
	//texture --------------------------------------
		
	coords[coordOffset     ] = brushX+halfBrushSize;
	coords[coordOffset + 1 ] = brushY+halfBrushSize;
	
	coords[coordOffset + 2 ] = brushX+halfBrushSize;//+w;
	coords[coordOffset + 3 ] = brushY+halfBrushSize;
	
	coords[coordOffset + 4 ] = brushX+halfBrushSize;
	coords[coordOffset + 5 ] = brushY+halfBrushSize;//+h;
	
	
	
	coords[coordOffset + 6 ] = brushX+halfBrushSize;//+w;
	coords[coordOffset + 7 ] = brushY+halfBrushSize;
	
	coords[coordOffset + 8 ] = brushX+halfBrushSize;
	coords[coordOffset + 9 ] = brushY+halfBrushSize;//+h;
	
	coords[coordOffset + 10] = brushX+halfBrushSize;//+w;
	coords[coordOffset + 11] = brushY+halfBrushSize;//+h;
	
	numSprites[layer]++; // sprite == 2 triangles
	
	return true;
}

bool ofxSpriteSheetRenderer::addCenteredRect(float x, float y, float z, float w, float h, int layer)
{
	return addRect(x-w/2,y-h/2,z,w,h,layer);
}

void ofxSpriteSheetRenderer::getFrameXandY(int tile_position, float &x, float &y)
{

	y = (tile_position / spriteSheetWidth);
	x = (tile_position - y * spriteSheetWidth);
	
	x*=tileSize_f;
	y*=tileSize_f;
}

float ofxSpriteSheetRenderer::getX(int x, int y, int angle){
	angle = CLAMP(angle,0,359);
	return (float)x*_cos[angle] - (float)y*_sin[angle];
}
float ofxSpriteSheetRenderer::getY(int x, int y, int angle){
	angle = CLAMP(angle,0,359);
	return (float)x*_sin[angle] + (float)y*_cos[angle];
}

void ofxSpriteSheetRenderer::setBrushIndex(int index, int wh)
{
	brushIndex = index;
	getFrameXandY(brushIndex, brushX, brushY);
	
	brushSize = tileSize_f * wh;
	halfBrushSize = brushSize/2;
}

void ofxSpriteSheetRenderer::setShapeColor(int r, int g, int b, int a)
{
	shapeR = r;
	shapeG = g;
	shapeB = b;
	shapeA = a;
}

void ofxSpriteSheetRenderer::setCircleResolution(int res){
	
	if(res%2!=0)
		res--; // only even resolutions
	
	res = MIN( MAX(1, res), OF_MAX_CIRCLE_PTS);
	
	if (res > 1 && res != numCirclePts){
		numCirclePts = res;
		
		float angle = 0.0f;
		float angleAdder = M_TWO_PI / (float)res;
		int k = 0;
		for (int i = 0; i < numCirclePts; i++){
			circlePts[k] = cos(angle);
			circlePts[k+1] = sin(angle);
			angle += angleAdder;
			k+=2;
		}
	}
}