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
	
	generateRotationArrays();
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
		sheetSize = widthHeight;
		
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
	if(f==F_HORIZ)
		return addTile(sprite->tex_x, sprite->tex_y, x+sprite->w-sprite->sprite_x-sprite->tex_w, y+sprite->sprite_y, layer, sprite->tex_w, sprite->tex_h, f, r, g, b, alpha);		
	// we are no longer handling the animation system in the tile renderer, so we are going to pass x y coords rather than indexes to the next object
	return addTile(sprite->tex_x, sprite->tex_y, x+sprite->sprite_x, y+sprite->sprite_y, layer, sprite->tex_w, sprite->tex_h, f, r, g, b, alpha);
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

bool ofxSpriteSheetRenderer::addCenterRotatedTile(animation_t* sprite, float x, float y, int layer, float wh, flipDirection f, float scale, int rot, int r, int g, int b, int alpha){
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
	// we are no longer handling the animation system in the tile renderer, so we are going to pass x y coords rather than indexes to the next object
	return addCenterRotatedTile(sprite->tex_x, sprite->tex_y, x, y, layer, sprite->tex_w, sprite->tex_h, f, scale, rot, r, g, b, alpha);

}

bool ofxSpriteSheetRenderer::addTile(float tex_x, float tex_y, float x, float y, int layer, float w, float h, flipDirection f, int r, int g, int b, int alpha)
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
	
	float frameX = tex_x;
	float frameY = tex_y;
	int layerOffset = layer*tilesPerLayer;
	int vertexOffset = (layerOffset + numSprites[layer])*18;
	int colorOffset = (layerOffset + numSprites[layer])*24;

	//cout << "texture x offset: " << frameX << endl;

//	getFrameXandY(tile_name, frameX, frameY);
	
//	frameX += frame*w*tileSize_f;
	frameX /= sheetSize;
	frameY /= sheetSize;
//	w /= sheetSize;
//	h /= sheetSize;
	
	addTexCoords(f, frameX, frameY, layer, w, h);
	
//	w*=tileSize;
//	h*=tileSize;
	
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

bool ofxSpriteSheetRenderer::addCenterRotatedTile(float tex_x, float tex_y, float x, float y, int layer, float w, float h, flipDirection f, float scale, int rot, int r, int g, int b, int alpha)
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
	
	float frameX = tex_x;
	float frameY = tex_y;

	int layerOffset = layer*tilesPerLayer;
	int vertexOffset = (layerOffset + numSprites[layer])*18;
	int colorOffset = (layerOffset + numSprites[layer])*24;
	
	frameX /= sheetSize;
	frameY /= sheetSize;
	//	w /= sheetSize;
	//	h /= sheetSize;
	
	int degOff = 0;
	cout << w << " : " << h << endl;
	if (w != h) {
		degOff = atan2(w/2, h/2)*RAD_TO_DEG-45;
		cout << degOff << endl;
	}
	addTexCoords(f, frameX, frameY, layer, w, h);
	w *= scale;
	h *= scale;
	float halfSize = sqrt((w/2)*(w/2)+(h/2)*(h/2));
	
	
	w = halfSize;
	h = halfSize;

	rot = rot%360;
	if (rot<0)
		rot+=360;
	
	int ulRot = rot-degOff;
	int urRot = rot+degOff;
	int llRot = rot+degOff;
	int lrRot = rot-degOff;
	
	// scale to 360
	ulRot = ulRot%360;
	if (ulRot<0)
		ulRot+=360;
	urRot = urRot%360;
	if (urRot<0)
		urRot+=360;
	llRot = llRot%360;
	if (llRot<0)
		llRot+=360;
	lrRot = lrRot%360;
	if (lrRot<0)
		lrRot+=360;
	
	rot*=2;
	ulRot*=2;
	urRot*=2;
	llRot*=2;
	lrRot*=2;
	
	//verticies ------------------------------------
	verts[vertexOffset     ] = x+w*ul[ulRot  ]; //ul ur ll
	verts[vertexOffset + 1 ] = y+h*ul[ulRot+1];
	verts[vertexOffset + 2 ] = 0;
	
	verts[vertexOffset + 3 ] = x+w*ur[urRot  ];
	verts[vertexOffset + 4 ] = y+h*ur[urRot+1];
	verts[vertexOffset + 5 ] = 0;
	
	verts[vertexOffset + 6 ] = x+w*ll[llRot  ];
	verts[vertexOffset + 7 ] = y+h*ll[llRot+1];
	verts[vertexOffset + 8 ] = 0;
	
	verts[vertexOffset + 9 ] = x+w*ur[urRot  ]; //ur ll lr
	verts[vertexOffset + 10] = y+h*ur[urRot+1];
	verts[vertexOffset + 11] = 0;
	
	verts[vertexOffset + 12] = x+w*ll[llRot  ];
	verts[vertexOffset + 13] = y+h*ll[llRot+1];
	verts[vertexOffset + 14] = 0;
	
	verts[vertexOffset + 15] = x+w*lr[lrRot  ];
	verts[vertexOffset + 16] = y+h*lr[lrRot+1];
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

void ofxSpriteSheetRenderer::update(unsigned long time)
{
	gameTime = time;
}

void ofxSpriteSheetRenderer::draw()
{	
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
	for(int i = 0; i < numLayers; i++)
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
	w /= sheetSize;
	h /= sheetSize;
	
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

void ofxSpriteSheetRenderer::getFrameXandY(int tile_position, float &x, float &y)
{
	y = (tile_position / spriteSheetWidth);
	x = (tile_position - y * spriteSheetWidth);
	
	x*=tileSize_f;
	y*=tileSize_f;
}

void ofxSpriteSheetRenderer::generateRotationArrays()
{	
	// ul
	int start = 225;
	for(int i=0;i<360;i++)
	{
		ul[i*2    ] = cos(TWO_PI*((float)start/360));
		ul[i*2 + 1] = sin(TWO_PI*((float)start/360));
		start++;
		if(start>=360)
			start=0;
	}
	
	//ur
	start = 315;
	for(int i=0;i<360;i++)
	{
		ur[i*2    ] = cos(TWO_PI*((float)start/360));
		ur[i*2 + 1] = sin(TWO_PI*((float)start/360));
		start++;
		if(start>=360)
			start=0;
	}
	
	//lr
	start = 45;
	for(int i=0;i<360;i++)
	{
		lr[i*2    ] = cos(TWO_PI*((float)start/360));
		lr[i*2 + 1] = sin(TWO_PI*((float)start/360));
		start++;
		if(start>=360)
			start=0;
	}
	
	//ll
	start = 135;
	for(int i=0;i<360;i++)
	{
		ll[i*2    ] = cos(TWO_PI*((float)start/360));
		ll[i*2 + 1] = sin(TWO_PI*((float)start/360));
		start++;
		if(start>=360)
			start=0;
	}
}