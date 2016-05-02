#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "opencv2/opencv.hpp"
#include "ofxCvPlayer.h"

class ofApp : public ofBaseApp{

public:
	void setup();
	void update();
	void draw();

	void keyPressed(int key);
	void dragEvent(ofDragInfo dragInfo);
	
protected:
	ofRectangle getCenteredRect(int srcWidth, int srcHeight, int otherWidth, int otherHeight, bool isFill = true)
	{
		auto other = ofRectangle(0, 0, otherWidth, otherHeight);
		ofRectangle result;
		result.setFromCenter(other.getCenter(), srcWidth, srcHeight);
		float scaleBy;
		auto aspectAspect = result.getAspectRatio() / other.getAspectRatio();

		if ((isFill && aspectAspect <= 1.0f) || (!isFill && aspectAspect >= 1.0f))
			scaleBy = other.getWidth() / result.getWidth();
		else
			scaleBy = other.getHeight() / result.getHeight();

		result.scaleFromCenter(scaleBy);
		return result;
	}

private:
	enum {
		WIDTH = 1280,
		HEIGHT = 720,
		FBO_SIZE = 1920,
		MAX_PLAYERS = 4
	};
	
	bool bDebugVisible = true;
	std::vector<std::shared_ptr<ofxGuiGroup> > mGui;

	ofParameterGroup mSettings;
	ofParameter<float> gThreshold;
	ofParameter<int> gNum;
		
	std::vector<ofxCvPlayer> players;
	std::vector<ofTexture> textures;

};
