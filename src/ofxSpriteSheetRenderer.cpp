
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
int ofxSpriteSheetRenderer::polyCount = 0;
int ofxSpriteSheetRenderer::polyCountFrame = 0;

ofxSpriteSheetRenderer::ofxSpriteSheetRenderer(int _numLayers, int _tilesPerLayer, int _defaultLayer, int _tileSize)
{
	texture = NULL;
	numSprites = NULL;
	points = NULL;
    
	textureIsExternal = false;
	
	safeMode = true;
	
	gameTime = ofGetElapsedTimeMillis();
		
	reAllocateArrays(_numLayers, _tilesPerLayer, _defaultLayer, _tileSize);
	
	generateRotationArrays();
    screen.set(0, 0, ofGetWidth(), ofGetHeight());
}

ofxSpriteSheetRenderer::~ofxSpriteSheetRenderer()
{
	if(texture != NULL && textureIsExternal)
		texture->clear();
	
	if(numSprites != NULL)
		delete[] numSprites;
    if(points != NULL)
        delete[] points;
}

void ofxSpriteSheetRenderer::reAllocateArrays(int _numLayers, int _tilesPerLayer, int _defaultLayer, int _tileSize)
{
	numLayers = _numLayers;
	tileSize = _tileSize;
	tilesPerLayer = _tilesPerLayer;
	defaultLayer = _defaultLayer;
	
	if(points != NULL)
		delete[] points;
	if(numSprites != NULL)
		delete[] numSprites;
	
	numSprites = new int[numLayers];	
    points = new vertexStruct[numLayers * tilesPerLayer * 6];
    
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
void ofxSpriteSheetRenderer::allocate(int width, int height, int internalGLScaleMode)
{
	if(texture == NULL)
	{
		tileSize_f = tileSize;
		tileSize_f /= width;
		sheetSize = width;
        
		spriteSheetWidth = width/tileSize;
        spriteSheetHeight = height/tileSize;
        
		LinearTexture * newTexture = new LinearTexture();
		
		newTexture->allocate(width, height, GL_RGBA, internalGLScaleMode);
		
		texture = (ofTexture*) newTexture;
	}
	else
		cerr<<"cannot double allocate ofxSpriteSheetRenderer Texture! Please clearTexture() first"<<endl;
}
void ofxSpriteSheetRenderer::allocate(int widthHeight, int internalGLScaleMode)
{
	if(texture == NULL)
	{
		tileSize_f = tileSize;
		tileSize_f /= widthHeight;
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
void ofxSpriteSheetRenderer::loadTexture(string fileName, int width, int height, int internalGLScaleMode)
{
    clearTexture();
	clear();
	allocate(width, height, internalGLScaleMode);
	addMisc(fileName, 0, 0);
	finishTexture();
	textureIsExternal = false;
}

void ofxSpriteSheetRenderer::loadTexture(string fileName, int widthHeight, int internalGLScaleMode)
{
    loadTexture(fileName, widthHeight, widthHeight, internalGLScaleMode);
}

void ofxSpriteSheetRenderer::loadTexture(ofTexture * _texture)
{
	clear();
	clearTexture();
	textureIsExternal = true;
	texture = _texture;
    sheetSize = texture->getWidth();
    spriteSheetWidth = texture->getWidth();
    spriteSheetHeight = texture->getHeight();
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
	if(layer==-1)
		layer=defaultLayer;

	// we are no longer handling the animation system in the tile renderer, so we are going to pass x y coords rather than indexes to the next object
	return addCenterRotatedTile(sprite->tex_x, sprite->tex_y, x, y, layer, sprite->tex_w, sprite->tex_h, f, scale, rot, r, g, b, alpha);

}
bool ofxSpriteSheetRenderer::addCornerTile(animation_t* sprite, ofPoint p1, ofPoint p2, ofPoint p3, ofPoint p4, int layer, flipDirection f, int r, int g, int b, int alpha)
{
	if(layer==-1)
		layer=defaultLayer;
    /*
    float alphaMult = alpha/255;
    r *= alphaMult;
    g *= alphaMult;
    b *= alphaMult;
     */
	return addCornerTile(sprite->tex_x, sprite->tex_y, p1, p2, p3, p4, layer, f, sprite->tex_w, sprite->tex_h, r, g, b, alpha);
}
bool ofxSpriteSheetRenderer::addCornerColorTile(animation_t* sprite, ofPoint p1, ofPoint p2, ofPoint p3, ofPoint p4,			int layer, flipDirection f, ofColor c1, ofColor c2, ofColor c3, ofColor c4)
{
	if(layer==-1)
		layer=defaultLayer;
	return addCornerColorTile(sprite->tex_x, sprite->tex_y, p1, p2, p3, p4, layer, f, sprite->tex_w, sprite->tex_h, c1, c2, c3, c4);
}

bool ofxSpriteSheetRenderer::addTile(float tex_x, float tex_y, float x, float y, int layer, float w, float h, flipDirection f, int r, int g, int b, int alpha)
{
    static ofColor color;
    static ofVec2f p1, p2, p3, p4;
    p1.set(x, y);
    p2.set(x+w, y);
    p3.set(x, y+h);
    p4.set(x+w, y+h);
    color.set(r, g, b, alpha);
    return addCornerColorTile(tex_x, tex_y, p1, p2, p3, p4, layer, f, w, h, color, color, color, color);
}

bool ofxSpriteSheetRenderer::addCenteredTile(int tile_name, int frame, float x, float y, int layer, float w, float h, flipDirection f, float scale, int r, int g, int b, int alpha)
{
    // this one requires a bit of additional lookup
    float tex_x;
	float tex_y;

	getFrameXandY(tile_name, tex_x, tex_x);
	tex_x += frame*w*tileSize_f;
	
    static float tex_h = h;
    static float tex_w = w;
		
	w*=tileSize*scale;
	w/=2;
	h*=tileSize*scale;
	h/=2;
    static ofColor color;
    static ofVec2f p1, p2, p3, p4;
    p1.set(x-w, y-h);
    p2.set(x+w, y-h);
    p3.set(x-w, y+h);
    p4.set(x+w, y+h);
    color.set(r, g, b, alpha);
    return addCornerColorTile(tex_x, tex_y, p1, p2, p3, p4, layer, f, tex_w, tex_h, color, color, color, color);
}

bool ofxSpriteSheetRenderer::addCenterRotatedTile(float tex_x, float tex_y, float x, float y, int layer, float w, float h, flipDirection f, float scale, int rot, int r, int g, int b, int alpha)
{
	
//    static float tex_h = h;
//    static float tex_w = w;
    
//	tex_x /= spriteSheetWidth;
//	tex_y /= spriteSheetHeight;
//	tex_w /= spriteSheetWidth;
//    tex_h /= spriteSheetHeight;
    float sw = w;
    float sh = h;
    
	int degOff = 0;
	if (w != h) {
		degOff = atan2(w/2, h/2)*RAD_TO_DEG-45;
	}
	sw *= scale;
	sh *= scale;
	float halfSize = sqrt((sw/2)*(sw/2)+(sh/2)*(sh/2));
	
	
	sw = halfSize;
	sh = halfSize;

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

    sw = floor(sw);
    sh = floor(sh);
    x = floor(x);
    y = floor(y);

    static ofColor color;
    static ofVec2f p1, p2, p3, p4;
    p1.set(x+sw*ul[ulRot  ], y+sh*ul[ulRot+1]);
    p2.set(x+sw*ur[urRot  ], y+sh*ur[urRot+1]);
    p3.set(x+sw*ll[llRot  ], y+sh*ll[llRot+1]);
    p4.set(x+sw*lr[lrRot  ], y+sh*lr[lrRot+1]);
    color.set(r, g, b, alpha);
    
    return addCornerColorTile(tex_x, tex_y, p1, p2, p3, p4, layer, f, w, h, color, color, color, color);

}
bool ofxSpriteSheetRenderer::addCornerTile(float tex_x, float tex_y,  ofPoint p1, ofPoint p2, ofPoint p3, ofPoint p4, int layer, flipDirection f, float w, float h, int r, int g, int b, int alpha)
{
    static ofColor color;
    color.set(r, g, b, alpha);
    return addCornerColorTile(tex_x, tex_y, p1, p2, p3, p4, layer, f, w, h, color, color, color, color);
}
bool ofxSpriteSheetRenderer::addCornerColorTile(float tex_x, float tex_y,  ofPoint p1, ofPoint p2, ofPoint p3, ofPoint p4, int layer, flipDirection f, float w, float h, ofColor c1, ofColor c2, ofColor c3, ofColor c4)
{
	//flipDirection f = F_NONE;
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
	int vertexOffset = (layerOffset + numSprites[layer])*6;
	
	frameX /= spriteSheetWidth;
	frameY /= spriteSheetHeight;
	
	addTexCoords(f, frameX, frameY, layer, vertexOffset, w, h);

    // tl
    // add a degenerate triangle at the beginning, i.e. put in the same point twice
    points[vertexOffset].position[0] = p1.x;
    points[vertexOffset].position[1] = p1.y;
    points[vertexOffset].position[2] = 0;
    // add a degenerate triangle at the beginning, i.e. put in the same point twice    
    points[vertexOffset+1].position[0] = p1.x;
    points[vertexOffset+1].position[1] = p1.y;
    points[vertexOffset+1].position[2] = 0;
    
    // tr
    points[vertexOffset+2].position[0] = p2.x;
    points[vertexOffset+2].position[1] = p2.y;
    points[vertexOffset+2].position[2] = 0;
    // bl
    points[vertexOffset+3].position[0] = p3.x;
    points[vertexOffset+3].position[1] = p3.y;
    points[vertexOffset+3].position[2] = 0;
    // br
    points[vertexOffset+4].position[0] = p4.x;
    points[vertexOffset+4].position[1] = p4.y;
    points[vertexOffset+4].position[2] = 0;
    // add another degenerate triangle at the end
    points[vertexOffset+5].position[0] = p4.x;
    points[vertexOffset+5].position[1] = p4.y;
    points[vertexOffset+5].position[2] = 0;
    
    // do the same thing for the colors
    // tl
    points[vertexOffset].color[0] = c1.r;
    points[vertexOffset].color[1] = c1.g;
    points[vertexOffset].color[2] = c1.b;
    points[vertexOffset].color[3] = c1.a;
    // add a degenerate triangle at the beginning, i.e. put in the same point twice    
    points[vertexOffset+1].color[0] = c1.r;
    points[vertexOffset+1].color[1] = c1.g;
    points[vertexOffset+1].color[2] = c1.b;
    points[vertexOffset+1].color[3] = c1.a;
    
    // tr
    points[vertexOffset+2].color[0] = c2.r;
    points[vertexOffset+2].color[1] = c2.g;
    points[vertexOffset+2].color[2] = c2.b;
    points[vertexOffset+2].color[3] = c2.a;
    // bl
    points[vertexOffset+3].color[0] = c3.r;
    points[vertexOffset+3].color[1] = c3.g;
    points[vertexOffset+3].color[2] = c3.b;
    points[vertexOffset+3].color[3] = c3.a;
    // br
    points[vertexOffset+4].color[0] = c4.r;
    points[vertexOffset+4].color[1] = c4.g;
    points[vertexOffset+4].color[2] = c4.b;
    points[vertexOffset+4].color[3] = c4.a;
    
    // add another degenerate triangle at the end
    points[vertexOffset+5].color[0] = c4.r;
    points[vertexOffset+5].color[1] = c4.g;
    points[vertexOffset+5].color[2] = c4.b;
    points[vertexOffset+5].color[3] = c4.a;
		
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
        
	glVertexPointer(3, GL_SHORT, sizeof(vertexStruct), &points[0].position);
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(vertexStruct), &points[0].color);
	glTexCoordPointer(2, GL_FLOAT, sizeof(vertexStruct), &points[0].texCoord);
	texture->bind();
	for(int i = 0; i < numLayers; i++){
		if(numSprites[i] > 0){
			glDrawArrays(GL_TRIANGLE_STRIP, i*tilesPerLayer*6, numSprites[i]*6);
        }
    }
	texture->unbind();
	if(safeMode)
	{
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}
}

void ofxSpriteSheetRenderer::addTexCoords(flipDirection f, float &frameX, float &frameY, int layer, int coordOffset, float w, float h)
{
	w /= spriteSheetWidth;
	h /= spriteSheetHeight;
	
	switch (f) {
		case F_NONE:
            //tl
            points[coordOffset].texCoord[0] = frameX;
            points[coordOffset].texCoord[1] = frameY;
            points[coordOffset+1].texCoord[0] = frameX;
            points[coordOffset+1].texCoord[1] = frameY;

            //tr
            points[coordOffset+2].texCoord[0] = frameX+w;
            points[coordOffset+2].texCoord[1] = frameY;

			//bl
            points[coordOffset+3].texCoord[0] = frameX;
            points[coordOffset+3].texCoord[1] = frameY+h;
			
			//br
            points[coordOffset+4].texCoord[0] = frameX+w;
            points[coordOffset+4].texCoord[1] = frameY+h;
            points[coordOffset+5].texCoord[0] = frameX+w;
            points[coordOffset+5].texCoord[1] = frameY+h;
			
			break;
		case F_HORIZ:
            
            //tl
            points[coordOffset].texCoord[0] = frameX+w;
            points[coordOffset].texCoord[1] = frameY;
            points[coordOffset+1].texCoord[0] = frameX+w;
            points[coordOffset+1].texCoord[1] = frameY;
            
            //tr
            points[coordOffset+2].texCoord[0] = frameX;
            points[coordOffset+2].texCoord[1] = frameY;
            
			//bl
            points[coordOffset+3].texCoord[0] = frameX+w;
            points[coordOffset+3].texCoord[1] = frameY+h;
			
			//br
            points[coordOffset+4].texCoord[0] = frameX;
            points[coordOffset+4].texCoord[1] = frameY+h;
            points[coordOffset+5].texCoord[0] = frameX;
            points[coordOffset+5].texCoord[1] = frameY+h;
			
			break;
		case F_VERT:
            //tl
            points[coordOffset].texCoord[0] = frameX;
            points[coordOffset].texCoord[1] = frameY+h;
            points[coordOffset+1].texCoord[0] = frameX;
            points[coordOffset+1].texCoord[1] = frameY+h;
            
            //tr
            points[coordOffset+2].texCoord[0] = frameX+w;
            points[coordOffset+2].texCoord[1] = frameY+h;
            
			//bl
            points[coordOffset+3].texCoord[0] = frameX;
            points[coordOffset+3].texCoord[1] = frameY;
			
			//br
            points[coordOffset+4].texCoord[0] = frameX+w;
            points[coordOffset+4].texCoord[1] = frameY;
            points[coordOffset+5].texCoord[0] = frameX+w;
            points[coordOffset+5].texCoord[1] = frameY;
			
			break;
		case F_HORIZ_VERT:
            //tl
            points[coordOffset].texCoord[0] = frameX+w;
            points[coordOffset].texCoord[1] = frameY+h;
            points[coordOffset+1].texCoord[0] = frameX+w;
            points[coordOffset+1].texCoord[1] = frameY+h;
            
            //tr
            points[coordOffset+2].texCoord[0] = frameX;
            points[coordOffset+2].texCoord[1] = frameY+h;
            
			//bl
            points[coordOffset+3].texCoord[0] = frameX+w;
            points[coordOffset+3].texCoord[1] = frameY;
			
			//br
            points[coordOffset+4].texCoord[0] = frameX;
            points[coordOffset+4].texCoord[1] = frameY;
            points[coordOffset+5].texCoord[0] = frameX;
            points[coordOffset+5].texCoord[1] = frameY;
			
			break;
		default:
			break;
	}
}

void ofxSpriteSheetRenderer::getFrameXandY(int tile_position, float &x, float &y)
{
	y = (tile_position / spriteSheetHeight);
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