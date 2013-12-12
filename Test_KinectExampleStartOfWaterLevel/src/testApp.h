#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxKinect.h"
#include "ofxBox2d.h"
#include "ofxGui.h"

#define USE_PHYSICS 0

class testApp : public ofBaseApp {
public:
	
	void setup();
	void update();
	void draw();
	void exit();
	
	void keyPressed(int key);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void windowResized(int w, int h);
    
    //additional functions
    void updateTideMask();
    void updateNoise();
    void drawBlobs();
    void drawSpirals();
    void drawTideMask();
    void drawNoise();
    void drawBackground();
    void drawGUI();
	
    //kinect
	ofxKinect kinect;
	
	ofxCvColorImage colorImg;
	
	ofxCvGrayscaleImage grayImage; // grayscale depth image
	ofxCvGrayscaleImage grayThreshNear; // the near thresholded image
	ofxCvGrayscaleImage grayThreshFar; // the far thresholded image
	
	ofxCvContourFinder contourFinder;
	
	int nearThreshold;
	int farThreshold;
	
	int angle;
    
    //box2d
	ofxBox2d						box2d;			  //	the box2d world
	vector		<ofxBox2dCircle>	circles;		  //	default box2d circles
	vector		<ofxBox2dRect>		boxes;			  //	defalut box2d rects
    
    ofColor circleColour;
    ofColor boxColour;
    
    vector< vector<ofPoint> > circleHistoryOfPositions;
    vector< vector<ofPoint> > boxHistoryOfPositions;
    
    //gui now...
	bool bShowGUI;
    
	ofxFloatSlider tideRadius;
	//ofxColorSlider color;
	//ofxVec2Slider centre;
	ofxIntSlider tideCircleResolution;
    ofxToggle showBlobs;
    ofxToggle showSpirals;
    ofxToggle showTideMask;
    ofxToggle showNoise;
    ofxToggle showExtraGUI;
    
	ofxPanel gui;
    
    //mask fbo for the well...
    ofFbo    maskFbo; //see /examples/gl/alphaMaskingShaderExample
    
    //noise
    ofImage noiseImage;
    
    //image to sample colours from - TODO: replace by live camera image from ext CCTV
    ofImage swntLiveImage;
};
