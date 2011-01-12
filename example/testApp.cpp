#include "testApp.h"

// comparison routine for sort...
bool sortVertically(  basicSprite * a, basicSprite * b ) { 
	return a->pos.y > b->pos.y; 
}   

//--------------------------------------------------------------
void testApp::setup(){
	ofSetFrameRate(30); //lets run at 30 fps!

	spriteRenderer = new ofxSpriteSheetRenderer(1, 10000, 0, 32); //declare a new renderer with 1 layer, 10000 tiles per layer, default layer of 0, tile size of 32
	spriteRenderer->loadTexture("spriteSheetExample.png", 256, GL_NEAREST); // load the spriteSheetExample.png texture of size 256x256 into the sprite sheet. set it's scale mode to nearest since it's pixel art
	
	ofEnableAlphaBlending(); // turn on alpha blending. important!
}

//--------------------------------------------------------------
void testApp::update(){
	spriteRenderer->clear(); // clear the sheet
	spriteRenderer->update(ofGetElapsedTimeMillis()); //update the time in the renderer, this is necessary for animations to advance
	
	sort( sprites.begin(), sprites.end(), sortVertically ); // sorts the sprites vertically so the ones that are lower are drawn later and there for in front of the ones that are higher up
	
	if(sprites.size()>0) // if we have sprites
	{
		for(int i=sprites.size()-1;i>=0;i--) //go through them
		{
			sprites[i]->pos.y+=sprites[i]->speed; //add their speed to their y
			
			if(sprites[i]->pos.y > ofGetHeight()+16) //if they are past the bottom of the screen
			{
				delete sprites[i]; //delete them
				sprites.erase(sprites.begin()+i); // remove them from the vector
			}
			else //otherwise
				spriteRenderer->addCenteredTile(&sprites[i]->animation, sprites[i]->pos.x, sprites[i]->pos.y); // add them to the sprite renderer (add their animation at their position, there are a lot more options for what could happen here, scale, tint, rotation, etc, but the animation, x and y are the only variables that are required)
		}
	}
	
	for(int i=0;i<10;i++) //lets add ten sprites every frame and fill the screen with an army
	{
		basicSprite * newSprite = new basicSprite(); // create a new sprite
		newSprite->pos.set(ofRandom(0,ofGetWidth()),-16); //set its position
		newSprite->speed=ofRandom(1,5); //set its speed
		newSprite->animation = walkAnimation; //set its animation to the walk animation we declared
		newSprite->animation.frame_duration /= newSprite->speed; //adjust its frame duration based on how fast it is walking (faster = smaller)
		newSprite->animation.index = (int)ofRandom(0,4)*8; //change the start index of our sprite. we have 4 rows of animations and our spritesheet is 8 tiles wide, so our possible start indicies are 0, 8, 16, and 24
		sprites.push_back(newSprite); //add our sprite to the vector
	}
}

//--------------------------------------------------------------
void testApp::draw(){
	
	spriteRenderer->draw(); //draw the sprites!
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
	
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
	
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

