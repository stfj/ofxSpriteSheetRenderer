#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
	ofSetFrameRate(30);
	
	//1 layer, 1000 tiles per layer, default layer of 0, tile size of 32
	spriteRenderer = new ofxSpriteSheetRenderer(1, 1000, 0, 32); 
	spriteRenderer->loadTexture("spriteSheetExample.png", 256, GL_NEAREST);
	
	ofEnableAlphaBlending();
}

//--------------------------------------------------------------
void testApp::update(){
	spriteRenderer->clear();
	spriteRenderer->update(ofGetElapsedTimeMillis());
	
	if(sprites.size()>0)
	{
		for(int i=sprites.size()-1;i>=0;i--)
		{
			sprites[i].pos.y+=sprites[i].speed;
			
			if(sprites[i].pos.y > ofGetHeight()+32)
				sprites.erase(sprites.begin()+i);
			else
				spriteRenderer->addCenteredTile(&sprites[i].animation, sprites[i].pos.x, sprites[i].pos.y);
		}
	}
}

//--------------------------------------------------------------
void testApp::draw(){
	
	spriteRenderer->draw();
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
	
	for(int i=0;i<10;i++)
	{
		basicSprite newSprite;
		
		newSprite.pos.set(x+ofRandomf()*100,y+ofRandomf()*100);
		newSprite.speed=ofRandom(1,5);
		newSprite.animation = walkAnimation;
		newSprite.animation.frame_duration /= newSprite.speed;
		sprites.push_back(newSprite);
	}
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

