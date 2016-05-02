#pragma once
#include "ofMain.h"
#include "opencv2/opencv.hpp"

class CvPlayer;

class ofxCvPlayer : public ofBaseVideoPlayer
{
public:
	ofxCvPlayer();
	ofxCvPlayer(const ofxCvPlayer&) = delete;
	ofxCvPlayer & operator=(const ofxCvPlayer&) = delete;
	ofxCvPlayer(ofxCvPlayer &&);
	ofxCvPlayer & operator=(ofxCvPlayer&&);

	bool                load(string path);
	void                update();

	void                close();

	void                play();
	void                stop();

	bool                isFrameNew() const;

	const ofPixels &    getPixels() const;
	ofPixels &          getPixels();

	float               getWidth() const;
	float               getHeight() const;

	bool                isPaused() const;
	bool                isLoaded() const;
	bool                isPlaying() const;

	bool                setPixelFormat(ofPixelFormat pixelFormat);
	ofPixelFormat       getPixelFormat() const;

	float               getPosition() const;
	float               getSpeed() const;
	float               getDuration() const;
	bool                getIsMovieDone() const;

	void                setPaused(bool bPause);
	void                setPosition(float pct);
	void                setVolume(float volume); // 0..1
	void                setLoopState(ofLoopType state);
	void                setSpeed(float speed);
	void                setFrame(int frame);  // frame 0 = first frame...

	int                 getCurrentFrame() const;
	int                 getTotalNumFrames() const;
	ofLoopType          getLoopState() const;

	void                firstFrame();
	void                nextFrame();
	void                previousFrame();

	void                loadTexture(ofTexture* tex);

private:
	std::shared_ptr<CvPlayer> player;
	ofPixelFormat pixelFormat = OF_PIXELS_RGB;
};