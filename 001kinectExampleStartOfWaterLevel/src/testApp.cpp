#include "testApp.h"


//--------------------------------------------------------------
void testApp::setup() {
    
    //general
    ofBackground(ofColor::black);
	ofEnableSmoothing();
	ofSetFrameRate(30);
    ofSetVerticalSync(true);
	ofSetLogLevel(OF_LOG_NOTICE);
    ofEnableAlphaBlending();
    
    //kinect
    
	//ofSetLogLevel(OF_LOG_VERBOSE);
	
	// enable depth->video image calibration
	kinect.setRegistration(true);
    kinect.setLed(ofxKinect::LED_OFF); //turn LED off..
    
	kinect.init();
	kinect.open();

	// print the intrinsic IR sensor values
	if(kinect.isConnected()) {
		ofLogNotice() << "sensor-emitter dist: " << kinect.getSensorEmitterDistance() << "cm";
		ofLogNotice() << "sensor-camera dist:  " << kinect.getSensorCameraDistance() << "cm";
		ofLogNotice() << "zero plane pixel size: " << kinect.getZeroPlanePixelSize() << "mm";
		ofLogNotice() << "zero plane dist: " << kinect.getZeroPlaneDistance() << "mm";
	}
	
	colorImg.allocate(kinect.width, kinect.height);
	grayImage.allocate(kinect.width, kinect.height);
	grayThreshNear.allocate(kinect.width, kinect.height);
	grayThreshFar.allocate(kinect.width, kinect.height);
	
	nearThreshold = 230;
	farThreshold = 70;
    
    //box 2D
	box2d.init();
	box2d.setGravity(0, 0);
	box2d.setFPS(30.0);
    
	for (int i=0; i<128; i++) {
		
		
		float r = ofRandom(10, 20);
		ofxBox2dCircle circle;
		circle.setPhysics(3.0, 0.53, 0.9);
        ofPoint circlePosition = ofPoint(ofGetWidth()/2, ofGetHeight()/2);
		circle.setup(box2d.getWorld(), circlePosition.x, circlePosition.y, r);
		circles.push_back(circle);
        
        vector<ofPoint> justOnePosition;
        justOnePosition.push_back(circlePosition);
        
        circleHistoryOfPositions.push_back(justOnePosition);
		
		float w = ofRandom(4, 20);
		float h = ofRandom(4, 20);
		ofxBox2dRect rect;
		rect.setPhysics(3.0, 0.53, 0.9);
        ofPoint rectPosition = ofPoint(ofGetWidth()/2, ofGetHeight()/2);
		rect.setup(box2d.getWorld(), rectPosition.x, rectPosition.y, w, h);
		boxes.push_back(rect);
        
        vector<ofPoint> justOneRectPosition;
        justOneRectPosition.push_back(rectPosition);
        
        boxHistoryOfPositions.push_back(justOneRectPosition);
	}
    
    circleColour = ofColor(129,202,189);
    boxColour = ofColor(85, 126, 191);
    
    //gui
    bShowGUI = true;
    
    gui.setup("Swynt");
	gui.add(tideRadius.setup( "tide radius", 140, 10, 300 ));
	gui.add(tideCircleResolution.setup("tide res", 5, 3, 90));
    gui.add(showBlobs.setup("show Blobs", true));
    gui.add(showSpirals.setup("show Spirals", true));
    gui.add(showTideMask.setup("show Tide mask", true));
    gui.add(showNoise.setup("show Noise", true));
    gui.add(showExtraGUI.setup("show Extra GUI", false));
    gui.loadFromFile("settings.xml");
    
    //masking
    maskFbo.allocate(kinect.width, kinect.height, GL_RGBA);
    // Let�s clear the FBO�s
    // otherwise it will bring some junk with it from the memory
    maskFbo.begin();
    ofClear(0,0,0,255);
    maskFbo.end();
    
    //noise image
    noiseImage.allocate(kinect.width, kinect.height,OF_IMAGE_GRAYSCALE);
    
    //image of the swnt - tobe replaced by live image
    swntLiveImage.loadImage("swntImage.jpg");
    
}

void testApp::updateTideMask(){
    ofFill();
    
    maskFbo.begin();
    
    ofClear(0,0,0,255);
    ofSetCircleResolution(tideCircleResolution);
    ofSetColor(ofColor(255,255,255,0));
    ofVec2f centre = ofVec2f(maskFbo.getWidth()/2.f, maskFbo.getHeight()/2.f);
    ofCircle(centre, tideRadius);
    
    maskFbo.end();
}

void testApp::updateNoise(){
    
    int maskWidth = noiseImage.getWidth();
    int maskHeight = noiseImage.getHeight();
    
    for (int y=0; y< maskHeight; y++) {
        for (int x=0; x< maskWidth; x++) {
            
            float a = x * .01;
            float b = y * .01;
            float c = ofGetFrameNum() / 50.0;
            
            float noise = ofNoise(a,b,c) * 255;
            float color = noise>200 ? ofMap(noise,200,255,0,255) : 0;
            
            noiseImage.getPixels()[y*maskWidth+x] = color;
        }
    }
    
    noiseImage.reloadTexture();
}

//--------------------------------------------------------------
void testApp::update() {
	
	kinect.update();
	
	// there is a new frame and we are connected
	if(kinect.isFrameNew()) {
		
		// load grayscale depth image from the kinect source
		grayImage.setFromPixels(kinect.getDepthPixels(), kinect.width, kinect.height);
		
        grayThreshNear = grayImage;
        grayThreshFar = grayImage;
        grayThreshNear.threshold(nearThreshold, true);
        grayThreshFar.threshold(farThreshold);
        cvAnd(grayThreshNear.getCvImage(), grayThreshFar.getCvImage(), grayImage.getCvImage(), NULL);
		
		// update the cv images
		grayImage.flagImageChanged();
		
		// find contours which are between the size of 20 pixels and 1/3 the w*h pixels.
		// also, find holes is set to true so we will get interior contours as well....
		contourFinder.findContours(grayImage, 10, (kinect.width*kinect.height)/2, 20, false);
	}
    
    //box2d
    box2d.update();
	ofVec2f mouse(ofGetMouseX(), ofGetMouseY());
	float minDis = ofGetMousePressed() ? 300 : 200;
    
	for(int i=0; i<circles.size(); i++) {
		float dis = mouse.distance(circles[i].getPosition());
		if(dis < minDis) circles[i].addRepulsionForce(mouse, 9);
		else circles[i].addAttractionPoint(mouse, 4.0);
		
		
	}
	for(int i=0; i<boxes.size(); i++) {
		float dis = mouse.distance(boxes[i].getPosition());
		if(dis < minDis) boxes[i].addRepulsionForce(mouse, 9);
		else boxes[i].addAttractionPoint(mouse, 4.0);
	}
    
    updateTideMask();
    updateNoise();
}

void testApp::drawBlobs(){
    //contourFinder.draw(0, 0, ofGetWidth(), ofGetHeight());
    //stolen from the above
    float blobWidth = contourFinder.getWidth();
    float blobHeight = contourFinder.getHeight();
    float x = 0.f;
    float y = 0.f;
    
    float scalex = 0.0f;
    float scaley = 0.0f;
    if( blobWidth != 0 ) { scalex = ofGetWidth()/blobWidth; } else { scalex = 1.0f; }
    if( blobHeight != 0 ) { scaley = ofGetHeight()/blobHeight; } else { scaley = 1.0f; }
    
    ofPushStyle();
    glPushMatrix();
    glTranslatef( x, y, 0.0 );
    glScalef( scalex, scaley, 0.0 );
    
	// ---------------------------- draw the blobs
    ofSetColor(boxColour);
    
	for( int i=0; i<(int)contourFinder.blobs.size(); i++ ) {
		ofFill();
		ofBeginShape();
		for( int j=0; j<contourFinder.blobs[i].nPts; j++ ) {
			ofVertex( contourFinder.blobs[i].pts[j].x, contourFinder.blobs[i].pts[j].y );
		}
		ofEndShape();
        
	}
	glPopMatrix();
	ofPopStyle();
}

void testApp::drawSpirals(){
    ofFill();
    
    ofSetColor(circleColour);
    
    for(int i=0; i<circles.size(); i++) {

        ofPoint positionOfCircle = circles[i].getPosition();
        //ofCircle(positionOfCircle.x, positionOfCircle.y, circles[i].getRadius());
		//circles[i].draw();
        
        circleHistoryOfPositions[i].push_back(positionOfCircle);
        
        if(circleHistoryOfPositions[i].size() > 50){
            circleHistoryOfPositions[i].erase(circleHistoryOfPositions[i].begin());
        }
        
        ofPolyline newLine;
        newLine.addVertices(circleHistoryOfPositions[i]);
        //ofSetColor(ofColor::red);
        newLine.draw();
	}
	
    ofSetColor(boxColour);
    
	for(int i=0; i<boxes.size(); i++) {

        //        ofRect(boxes[i].getPosition().x, boxes[i].getPosition().y, boxes[i].getWidth(), boxes[i].getHeight());
        //boxes[i].draw();
        
        ofPoint positionOfRect = ofPoint(boxes[i].getPosition().x, boxes[i].getPosition().y);
        boxHistoryOfPositions[i].push_back(positionOfRect);
        
        if(boxHistoryOfPositions[i].size() > 50){
            boxHistoryOfPositions[i].erase(boxHistoryOfPositions[i].begin());
        }
        
        ofPolyline newLine;
        newLine.addVertices(boxHistoryOfPositions[i]);
        //ofSetColor(ofColor::red);
        newLine.draw();
	}
	
	// draw the ground
	//box2d.drawGround();
}

void testApp::drawTideMask(){
    ofSetColor(ofColor::white);
    maskFbo.draw(0,0, ofGetWidth(),ofGetHeight());
}

void testApp::drawNoise(){
    ofSetColor(circleColour);
    noiseImage.draw(0,0, ofGetWidth(),ofGetHeight());
}

void testApp::drawBackground(){
    ofSetBackgroundColor(ofColor::black);
    ofSetColor(ofColor::white);
}

void testApp::drawGUI(){
    ofSetColor(ofColor::white);
    
    if(showExtraGUI){
        // draw from the live kinect
        kinect.drawDepth(10, 10, 400, 300);
        kinect.draw(420, 10, 400, 300);
        
        contourFinder.draw(10, 320, 400, 300);
        swntLiveImage.draw(420, 320, 400, 300);
        
        // draw instructions
        stringstream reportStream;
        
        reportStream << "set near threshold " << nearThreshold << " (press: + -)" << endl
        << "set far threshold " << farThreshold << " (press: < >) num blobs found " << contourFinder.nBlobs
        << ", fps: " << ofGetFrameRate() << endl;
        
        ofDrawBitmapString(reportStream.str(), 20, 652);
        
        string info = "";
        info += "Swynt. Hellicar&Lewis for the National Trust\n";
        info += "Press [c] for circles\n";
        info += "Press [b] for blocks\n";
        info += "Total Bodies: "+ofToString(box2d.getBodyCount())+"\n";
        info += "Total Joints: "+ofToString(box2d.getJointCount())+"\n\n";
        ofDrawBitmapString(info, 30, 30);
    }
    
    gui.draw();
}

//--------------------------------------------------------------
void testApp::draw() {

    drawBackground();
    
    if(showNoise){
        drawNoise();
    }
    
    if(showTideMask){
        drawTideMask();
    }
    
    if(showSpirals){
        drawSpirals();
    }
    
    if(showBlobs){
        drawBlobs();
    }
    
    if( bShowGUI ){
		drawGUI();
	}
}

//--------------------------------------------------------------
void testApp::exit() {
	kinect.close();
}

//--------------------------------------------------------------
void testApp::keyPressed (int key) {
	switch (key) {
		case '>':
		case '.':
        {
			farThreshold ++;
			if (farThreshold > 255) farThreshold = 255;
        }
			break;
		case '<':
		case ',':
        {
			farThreshold --;
			if (farThreshold < 0) farThreshold = 0;
        }
			break;
		case '+':
		case '=':
        {
			nearThreshold ++;
			if (nearThreshold > 255) nearThreshold = 255;
        }
			break;
		case '-':
        {
			nearThreshold --;
			if (nearThreshold < 0) nearThreshold = 0;
        }
			break;
        case 'g':
        {
            bShowGUI = !bShowGUI;
        }
            break;
        case 'c':
        {
            float r = ofRandom(14, 20);		// a random radius 4px - 20px
            ofxBox2dCircle circle;
            circle.setPhysics(3.0, 0.53, 0.9);
            circle.setup(box2d.getWorld(), mouseX, mouseY, r);
            circles.push_back(circle);
            
            vector<ofPoint> justOnePosition;
            justOnePosition.push_back(ofPoint(mouseX, mouseY));
            
            circleHistoryOfPositions.push_back(justOnePosition);
        }
            break;
        case 'b':
        {
            float w = ofRandom(14, 20);
            float h = ofRandom(14, 20);
            ofxBox2dRect rect;
            rect.setPhysics(3.0, 0.53, 0.9);
            rect.setup(box2d.getWorld(), mouseX, mouseY, w, h);
            boxes.push_back(rect);
            
            vector<ofPoint> justOnePosition;
            justOnePosition.push_back(ofPoint(mouseX, mouseY));
            
            boxHistoryOfPositions.push_back(justOnePosition);
        }
            break;
        case 'f':
        {
            ofToggleFullscreen();
        }
            break;
        case ' ':
        {
            string filename = ofGetTimestampString() + ".png";
            ofSaveScreen(filename);
            
        }
            break;
        case 's':
        {
            gui.saveToFile("settings.xml");
        }
            break;
        case 'l':
        {
            gui.loadFromFile("settings.xml");
        }
            break;
	}


}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button)
{}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button)
{}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button)
{}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h)
{}
