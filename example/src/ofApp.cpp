#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetWindowShape(WIDTH, HEIGHT);
	ofSetWindowPosition((ofGetScreenWidth() - ofGetWidth()) / 2, (ofGetScreenHeight() - ofGetHeight()) / 2);
	ofSetFrameRate(60);
	ofDisableArbTex();
	//ofSetVerticalSync(true);
        
    {
		mSettings.setName("Settings");
		mSettings.add(gThreshold.set("threshold", 128.0f, 0.0f, 255.0f));
		mSettings.add(gNum.set("num", 1, 1, MAX_PLAYERS));
		        
		auto gui = shared_ptr<ofxGuiGroup>(new ofxGuiGroup);
		gui->setup("GUI");
		gui->add(mSettings);
		mGui.push_back(gui);
    }
	

	{
		const string filename = "videos/gw2.mp4";
		players.resize(MAX_PLAYERS);
		textures.resize(MAX_PLAYERS);
		for (auto& p : players)
		{
			p.load(filename);
		}
	}

}

//--------------------------------------------------------------
void ofApp::update(){
	ofSetWindowTitle("oF Application: " + ofToString(ofGetFrameRate(), 1));
	

	float ts;
	ts = ofGetElapsedTimef();
	for (size_t i = 0; i < gNum; i++)
	{
		auto& p = players[i];
		if (!p.isFrameNew()) continue;

		auto& pix = p.getPixels();
		textures[i].loadData(pix);
	}
	float dt = ofGetElapsedTimef() - ts;
	if (dt > 1.0 / 60.0)
	{
		printf("update %f\n", dt);
	}
	
}

//--------------------------------------------------------------
void ofApp::draw(){
	ofClear(0);
	auto viewport = ofGetCurrentViewport();
	

	int num = gNum;
	if (num > 0)
	{
		int sq = sqrt(float(num));
		bool isPower2 = num == (sq * sq);
		if (!isPower2 || (num != 1 && sq == 1)) sq++;
		int cols = sq;
		int rows = num / sq;
		if (!isPower2 && num % sq != 0) rows++;

		//printf("%i, %i, %i\n", sq, cols, rows);

		float width = cols * textures[0].getWidth();
		float height = rows * textures[0].getHeight();

		auto rect = getCenteredRect(width, height, viewport.width, viewport.height, false);

		float scale = rect.width / width;

		int x = 0;
		int y = 0;
		ofPushMatrix();
		ofTranslate(rect.position);
		ofScale(scale, scale);
		for (int i = 0; i < gNum; i++)
		{
			auto& tex = textures[i];
			if (!tex.isAllocated()) continue;
			tex.draw(x * tex.getWidth(), y * tex.getHeight());
			x++;

			if (x < cols) continue;
			x = 0;
			y++;
		}
		ofPopMatrix();
	}

	

	if (bDebugVisible)
	{
		ofPushMatrix();
		for (auto& gui : mGui)
		{
			gui->draw();
		}
		ofPopMatrix();
	}
		
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	auto toggleFullscreen = [&]()
	{
		ofToggleFullscreen();
		if (!(ofGetWindowMode() == OF_FULLSCREEN))
		{
			ofSetWindowShape(WIDTH, HEIGHT);
			auto pos = ofVec2f(ofGetScreenWidth() - WIDTH, ofGetScreenHeight() - HEIGHT) * 0.5f;
			ofSetWindowPosition(pos.x, pos.y);
		}
	};
	
	switch (key)
	{
	case OF_KEY_F1:
		bDebugVisible = !bDebugVisible;
		break;
	case OF_KEY_F11:
		toggleFullscreen();
		break;
	}
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}