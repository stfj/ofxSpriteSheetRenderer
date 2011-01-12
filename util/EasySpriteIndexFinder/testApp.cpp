#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){

	DIR.setVerbose(false);
    nImages = DIR.listDir("images/");
 	images = new ofImage[nImages];
	
	for(int i = 0; i < nImages; i++){
		images[i].loadImage(DIR.getPath(i));
    }
	
	ofBackground(255, 255, 255);
	
	selTile=0;
    currentImage = 0;
	ofSetFrameRate(10);
	vert =0;
}

void testApp::drawGrid()
{
	ofSetColor(0,0,0,100);
	for(int x=0;x<32;x++)
	{
		ofLine(x*32,0,x*32,ofGetHeight()-16);
	}
	for(int y=0;y<32;y++)
	{
		ofLine(0,y*32,ofGetWidth(),y*32);
	}
}

//--------------------------------------------------------------
void testApp::update(){

}

//--------------------------------------------------------------
void testApp::draw()
{
	ofEnableAlphaBlending();
	glPushMatrix();
	ofSetColor(255,255,255);
	//glTranslated((1024-curWidth)/2, (768-curHeight)/2, 0);
	images[currentImage].draw(0,0-vert*32);
	drawGrid();
	glPopMatrix();
	
	ofSetColor(255,255,255);
	ofRect(0,0,200,12);
	ofSetColor(0,0,0);
	ofDrawBitmapString("selected tile # "+ofToString(selTile), 5, 10);
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
	
	
	if(key==358) // rt
	{
		currentImage++;
		if(currentImage==nImages)
			currentImage=0;
	}
	else if(key==356) // lft
	{
		currentImage--;
		if(currentImage<0)
			currentImage=nImages-1;
	}
	else if(key==357) // up
	{
		vert++;
	}
	else if(key==359) // dn
	{
		vert--;
	}
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
	selTile = (y/32+vert)*32+x/32;
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

