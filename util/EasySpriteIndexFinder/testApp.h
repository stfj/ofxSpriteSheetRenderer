#ifndef _TEST_APP
#define _TEST_APP


#include "ofMain.h"
#include "ofxDirList.h"

class testApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed  (int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);

		int 		nImages;
		ofImage	* 	images;
		ofxDirList   DIR;
		
		int         currentImage;
	
	int selTile;
	
	int vert;
	
	int TILESIZE;
	
	void drawGrid();
	
	int makingSprite;
	
	ofPoint spriteStart;
	int startIndex;
	int ww;
	int hh;
	int numFrames;
};

#endif
