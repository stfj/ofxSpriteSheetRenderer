#include "testApp.h"

char*  getTextFromPasteboard() {  
	
    OSStatus             err = noErr;  
    ItemCount            itemCount;  
    PasteboardSyncFlags  syncFlags;  
    static PasteboardRef inPasteboard = NULL;  
    PasteboardCreate( kPasteboardClipboard, &inPasteboard );  
    char* data;  
    data = "";  
	
    syncFlags = PasteboardSynchronize( inPasteboard );  
    err = badPasteboardSyncErr;  
	
    err = PasteboardGetItemCount( inPasteboard, &itemCount );  
    require_noerr( err, CantGetPasteboardItemCount );  
	
    for( int itemIndex = 1; itemIndex <= itemCount; itemIndex++ ) {  
        PasteboardItemID  itemID;  
        CFDataRef  flavorData;  
		
        err = PasteboardGetItemIdentifier( inPasteboard, itemIndex, &itemID );  
        require_noerr( err, CantGetPasteboardItemIdentifier );  
		
        err = PasteboardCopyItemFlavorData( inPasteboard, itemID, CFSTR("public.utf8-plain-text"), &flavorData );         
        data = (char*)CFDataGetBytePtr(flavorData);  
		
	CantGetPasteboardItemIdentifier:  
        ;  
    }  
	
CantGetPasteboardItemCount:  
	
    return data;  
}  


static OSStatus setTextToPasteboard(char* byteArrayIndex) {  
	
    OSStatus                err = noErr;  
    static PasteboardRef    pasteboard = NULL;  
    PasteboardCreate( kPasteboardClipboard, &pasteboard );  
	
    err = PasteboardClear( pasteboard );  
    require_noerr( err, PasteboardClear_FAILED );  
	
    CFDataRef  data;  
	
    data = CFDataCreate(kCFAllocatorDefault, (UInt8*) byteArrayIndex, strlen(byteArrayIndex));  
    err = PasteboardPutItemFlavor( pasteboard, (PasteboardItemID)1, kUTTypeUTF8PlainText, data, 0);   
    require_noerr( err, PasteboardPutItemFlavor_FAILED );  
	
PasteboardPutItemFlavor_FAILED:   
PasteboardClear_FAILED:  
    return err;  
}  

//--------------------------------------------------------------
void testApp::setup(){

	//DIR.setVerbose(false);
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
	
	TILESIZE = 32;
	
	makingSprite = 0;
}

void testApp::drawGrid()
{
	ofSetColor(0,0,0,100);
	for(int x=0;x<ofGetWidth()/TILESIZE;x++)
	{
		ofLine(x*TILESIZE,0,x*TILESIZE,ofGetHeight()-TILESIZE/2);
	}
	for(int y=0;y<ofGetHeight()/TILESIZE;y++)
	{
		ofLine(0,y*TILESIZE,ofGetWidth(),y*TILESIZE);
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
	images[currentImage].draw(0,0-vert*TILESIZE);
	drawGrid();
	glPopMatrix();
	
	ofSetColor(255,255,255);
	ofRect(0,0,200,12);
	ofSetColor(0,0,0);
	ofDrawBitmapString("selected tile # "+ofToString(selTile), 5, 10);
	
	
	if(makingSprite>1)
	{
		ofSetColor(255, 0, 0, 100);
		ofRect(spriteStart.x*TILESIZE, spriteStart.y*TILESIZE, ww*TILESIZE, hh*TILESIZE);
	}
	if(makingSprite>2)
	{
		for(int i=1;i<numFrames;i++)
		{
			ofFill();
			ofSetColor(255, 0, 255, 100);
			ofRect(spriteStart.x*TILESIZE + (spriteStart.x+i*ww)*TILESIZE, spriteStart.y*TILESIZE, ww*TILESIZE, hh*TILESIZE);
			
			ofNoFill();
			ofSetColor(0, 0, 0);
			ofRect(spriteStart.x*TILESIZE + (spriteStart.x+i*ww)*TILESIZE, spriteStart.y*TILESIZE, ww*TILESIZE, hh*TILESIZE);
			ofFill();
		}
	}
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
	else if(key=='q')
		TILESIZE/=2;
	else if(key=='w')
		TILESIZE*=2;
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
	
	if(makingSprite == 2 && selTile != (y/TILESIZE+vert)*(ofGetWidth()/TILESIZE)+x/TILESIZE)
	{
		numFrames = (x/TILESIZE - spriteStart.x)/ww + 1;
				
		string s = "animation_t SpriteName = {	"+ofToString(startIndex)+", /*index*/";
		s+="\n\t0,\t/*frame*/";
		s+="\n\t" + ofToString(numFrames)+",\t/*num frames*/";		
		s+="\n\t" + ofToString(ww)+",\t/*width*/";	
		s+="\n\t" + ofToString(hh)+",\t/*height*/";
		s+="\n\t75,\t/*frame duration*/";
		s+="\n\t0,\t/*next tick*/";
		s+="\n\t0,\t/*loops*/";
		s+="\n\t-1,\t/*final index*/";
		s+="\n\t1\t/*frame skip*/\n};";
		
		char *text = (char*)s.c_str();
		
		setTextToPasteboard(text);
		makingSprite = 3;
	}

	if(makingSprite == 1 && selTile != (y/TILESIZE+vert)*(ofGetWidth()/TILESIZE)+x/TILESIZE)
	{
		ww = x/TILESIZE - spriteStart.x + 1;
		hh = y/TILESIZE+vert - spriteStart.y + 1;
		makingSprite++;
	}
	
	if(selTile == (y/TILESIZE+vert)*(ofGetWidth()/TILESIZE)+x/TILESIZE)
	{
		makingSprite = 1;
		startIndex = (y/TILESIZE+vert)*(ofGetWidth()/TILESIZE)+x/TILESIZE;
		spriteStart.set(x/TILESIZE, y/TILESIZE+vert);
	}
	
	selTile = (y/TILESIZE+vert)*(ofGetWidth()/TILESIZE)+x/TILESIZE;
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

