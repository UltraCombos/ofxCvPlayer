#include "ofxCvPlayer.h"

class CvPlayer
{
public:
	CvPlayer()
	{
		isThreadRunning = true;
		mThread = std::thread(&CvPlayer::threadedFunction, this);
	}

	~CvPlayer()
	{
		isThreadRunning = false;
		if (mThread.joinable())
			mThread.join();
	}

	bool loadMovie(string path, ofPixelFormat format)
	{
		std::lock_guard<std::mutex> guard(captureMutex);
		bVideoOpened = cap.open(path);
		if (bVideoOpened)
		{
			width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
			height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
			averageTimePerFrame = 1.0 / cap.get(cv::CAP_PROP_FPS);
			totalFrames = cap.get(cv::CAP_PROP_FRAME_COUNT);
		}
		this->pixelFormat = format;
		return bVideoOpened;
	}
	
	bool isLoaded() {
		return bVideoOpened;
	}

	double getDurationInSeconds() {
		if (isLoaded()) {
			// need check;
			return totalFrames * averageTimePerFrame;
		}
		return 0.0;
	}

	void setVolume(float volPct) {
		if (isLoaded()) {
			volPct = CLAMP(volPct, 1.0, 0.0);
			// need implement
		}
	}

	float getVolume() {
		float volPct = 0.0;
		if (isLoaded()) {
			// need implement
		}
		return volPct;
	}

	double getCurrentTimeInSeconds() {
		if (isLoaded()) {
			return currentPosMsec / 1000.0;
		}
		return 0.0;
	}

	void setPosition(float pct) {
		if (bVideoOpened) {
			pct = CLAMP(pct, 0.0, 1.0);
			std::lock_guard<std::mutex> guard(captureMutex);
			cap.set(cv::CAP_PROP_POS_AVI_RATIO, pct);
		}
	}

	float getPosition() {
		if (bVideoOpened) {
			return currentPosition;
		}
		return 0.0;
	}

	void setSpeed(float speed) {
		if (bVideoOpened) {
			// need implement
		}
	}

	double getSpeed() {
		return movieRate;
	}

	void play() {
		if (!bVideoOpened) return;
		bEndReached = false;
		bPlaying = true;
	}

	void stop() {
		if (bVideoOpened) {
			if (isPlaying()) {
				setPosition(0.0);
				bPlaying = false;
				bPaused = false;
			}
		}
	}

	void setPaused(bool bPaused) {
		if (bVideoOpened) {
			if (bPaused) {
				bPlaying = false;
				this->bPaused = true;
			}
			else
			{
				bPlaying = true;
				this->bPaused = false;
			}
		}

	}

	bool isPlaying() {
		return bPlaying;
	}

	bool isPaused() {
		return bPaused;
	}

	bool isLooping() {
		return bLoop;
	}

	void setLoop(bool loop) {
		bLoop = loop;
	}

	bool isMovieDone() {
		return bEndReached;
	}

	float getWidth() {
		return width;
	}

	float getHeight() {
		return height;
	}

	bool isFrameNew() {
		return bFrameNew;
	}

	void nextFrame() {
		//we have to do it like this as the frame based approach is not very accurate
		if (bVideoOpened && (isPlaying() || isPaused())) {
			int curFrame = getCurrentFrameNo();
			float curFrameF = curFrame;
			for (int i = 1; i < 20; i++) {
				setAproximateFrameF(curFrameF + 0.3 * (float)i);
				if (getCurrentFrameNo() >= curFrame + 1) {
					break;
				}
			}
		}
	}

	void preFrame() {
		//we have to do it like this as the frame based approach is not very accurate
		if (bVideoOpened && (isPlaying() || isPaused())) {
			int curFrame = getCurrentFrameNo();
			float curFrameF = curFrame;
			for (int i = 1; i < 20; i++) {
				setAproximateFrameF(curFrameF - 0.3 * (float)i);
				if (getCurrentFrameNo() <= curFrame + 1) {
					break;
				}
			}
		}
	}

	void setAproximateFrameF(float frameF) {
		if (bVideoOpened) {
			float pct = frameF / (float)getAproximateNoFrames();
			pct = CLAMP(pct, 1.0, 0.0);
			setPosition(pct);
		}
	}

	void setAproximateFrame(int frame) {
		if (bVideoOpened) {
			float pct = (float)frame / (float)getAproximateNoFrames();
			pct = CLAMP(pct, 1.0, 0.0);
			setPosition(pct);
		}
	}

	int getCurrentFrameNo() {
		if (bVideoOpened) {
			return getPosition() * (float)getAproximateNoFrames();
		}
		return 0;
	}

	int getAproximateNoFrames() {
		if (bVideoOpened && averageTimePerFrame > 0.0) {
			return getDurationInSeconds() / averageTimePerFrame;
		}
		return 0;
	}

	ofPixels & getPixels() {
		if (bVideoOpened && bNewPixels) {
			std::lock_guard<std::mutex> guard(frameMutex);
			pixels.setFromExternalPixels(frames[frontIndex].data, frames[frontIndex].cols, frames[frontIndex].rows, OF_PIXELS_RGB);
			//pixels.setFromPixels(frames[frontIndex].data, frames[frontIndex].cols, frames[frontIndex].rows, OF_PIXELS_RGB);
			bNewPixels = false;
		}
		return pixels;
	}
protected:
	void threadedFunction()
	{
		float time_stamp = 0.0f;
		cv::Mat mat;
		while (isThreadRunning)
		{
			if (!bVideoOpened) continue;

			float current_time = ofGetElapsedTimef();
			if (current_time - time_stamp > averageTimePerFrame)
			{
				time_stamp = current_time;
				{
					std::lock_guard<std::mutex> guard(captureMutex);
					cap >> mat;
					currentPosMsec = cap.get(cv::CAP_PROP_POS_MSEC);
					frameCount = cap.get(cv::CAP_PROP_POS_FRAMES);
				}
				
				cv::cvtColor(mat, frames[backIndex], cv::COLOR_BGR2RGB);

				if (curMovieFrame != frameCount) {
					bFrameNew = true;
				}
				else {
					bFrameNew = false;
				}
				curMovieFrame = frameCount;

				currentPosition = curMovieFrame / totalFrames;

				if (totalFrames == curMovieFrame)
				{
					if (bLoop) {
						//printf("Restarting!\n");
						setPosition(0.0);
					}
					else {
						bEndReached = true;
						//printf("movie end reached!\n");
						stop();
					}
				}

				std::lock_guard<std::mutex> guard(frameMutex);
				bNewPixels = true;
				std::swap(frontIndex, backIndex);
			}
		}
	}

	cv::VideoCapture cap;
	cv::Mat frames[2];
	int frontIndex = 0;
	int backIndex = 1;

	double width = 0.0;
	double height = 0.0;
	double averageTimePerFrame = 0.0;
	double totalFrames = 0.0;
	double currentPosMsec = 0.0;
	double currentPosition = 0.0;
	bool bFrameNew = false;
	bool bNewPixels = false;
	bool bVideoOpened = false;
	bool bPlaying = false;
	bool bPaused = false;
	bool bLoop = true;
	bool bEndReached = false;
	double movieRate = 1.0;
	int curMovieFrame = 0;
	int frameCount;
	std::thread mThread;
	std::mutex frameMutex;
	std::mutex captureMutex;
	bool isThreadRunning = false;

	ofPixels pixels;
	ofPixelFormat pixelFormat;
};

ofxCvPlayer::ofxCvPlayer() {

}

ofxCvPlayer::ofxCvPlayer(ofxCvPlayer && other)
	:player(std::move(other.player))
	,pixelFormat(std::move(other.pixelFormat)) {

}

ofxCvPlayer & ofxCvPlayer::operator=(ofxCvPlayer&& other) {
	if (&other == this) {
		return *this;
	}

	player = std::move(other.player);
	pixelFormat = std::move(other.pixelFormat);
	return *this;
}

bool ofxCvPlayer::load(string path) {
	path = ofToDataPath(path);

	close();
	player.reset(new CvPlayer);
	bool loadOk = player->loadMovie(path, pixelFormat);
	if (!loadOk) {
		ofLogError("ofxCvPlayer") << " Cannot load video of this file type.  Make sure you have codecs installed on your system.  OF recommends the free K-Lite Codec pack. " << endl;
	}
	return loadOk;
}

void ofxCvPlayer::close() {
	player.reset();
}

void ofxCvPlayer::update() {
	if (player && player->isLoaded()) {
		// no need to update
	}
}

void ofxCvPlayer::play() {
	if (player && player->isLoaded()) {
		player->play();
	}
}

void ofxCvPlayer::stop() {
	if (player && player->isLoaded()) {
		player->stop();
	}
}

bool ofxCvPlayer::isFrameNew() const {
	return (player && player->isFrameNew());
}

const ofPixels & ofxCvPlayer::getPixels() const {
	return player->getPixels();
}

ofPixels & ofxCvPlayer::getPixels() {
	return player->getPixels();
}

float ofxCvPlayer::getWidth() const {
	if (player && player->isLoaded()) {
		return player->getWidth();
	}
	return 0.0;
}

float ofxCvPlayer::getHeight() const {
	if (player && player->isLoaded()) {
		return player->getHeight();
	}
	return 0.0;
}

bool ofxCvPlayer::isPaused() const {
	return (player && player->isPaused());
}

bool ofxCvPlayer::isLoaded() const {
	return (player && player->isLoaded());
}

bool ofxCvPlayer::isPlaying() const {
	return (player && player->isPlaying());
}

bool ofxCvPlayer::setPixelFormat(ofPixelFormat pixelFormat) {
	switch (pixelFormat) {
	case OF_PIXELS_RGB:
	case OF_PIXELS_BGR:
	case OF_PIXELS_BGRA:
	case OF_PIXELS_RGBA:
		this->pixelFormat = pixelFormat;
		return true;
	default:
		return false;
	}
}

ofPixelFormat ofxCvPlayer::getPixelFormat() const {
	return this->pixelFormat;
}

//should implement!
float ofxCvPlayer::getPosition() const {
	if (player && player->isLoaded()) {
		return player->getPosition();
	}
	return 0.0;
}

float ofxCvPlayer::getSpeed() const {
	if (player && player->isLoaded()) {
		return player->getSpeed();
	}
	return 0.0;
}

float ofxCvPlayer::getDuration() const {
	if (player && player->isLoaded()) {
		return player->getDurationInSeconds();
	}
	return 0.0;
}


bool ofxCvPlayer::getIsMovieDone() const {
	return (player && player->isMovieDone());
}

void ofxCvPlayer::setPaused(bool bPause) {
	if (player && player->isLoaded()) {
		player->setPaused(bPause);
	}
}

void ofxCvPlayer::setPosition(float pct) {
	if (player && player->isLoaded()) {
		player->setPosition(pct);
	}
}

void ofxCvPlayer::setVolume(float volume) {
	if (player && player->isLoaded()) {
		player->setVolume(volume);
	}
}

void ofxCvPlayer::setLoopState(ofLoopType state) {
	if (player) {
		if (state == OF_LOOP_NONE) {
			player->setLoop(false);
		}
		else if (state == OF_LOOP_NORMAL) {
			player->setLoop(true);
		}
		else {
			ofLogError("ofDirectShowPlayer") << " cannot set loop of type palindrome " << endl;
		}
	}
}

void ofxCvPlayer::setSpeed(float speed) {
	if (player && player->isLoaded()) {
		player->setSpeed(speed);
	}
}

int ofxCvPlayer::getCurrentFrame() const {
	if (player && player->isLoaded()) {
		return player->getCurrentFrameNo();
	}
	return 0;
}

int ofxCvPlayer::getTotalNumFrames() const {
	if (player && player->isLoaded()) {
		return player->getAproximateNoFrames();
	}
	return 0;
}

ofLoopType ofxCvPlayer::getLoopState() const {
	if (player) {
		if (player->isLooping()) {
			return OF_LOOP_NORMAL;
		}

	}
	return OF_LOOP_NONE;
}

void ofxCvPlayer::setFrame(int frame) {
	if (player && player->isLoaded()) {
		frame = ofClamp(frame, 0, getTotalNumFrames());
		return player->setAproximateFrame(frame);
	}
}  // frame 0 = first frame...

void ofxCvPlayer::firstFrame() {
	setPosition(0.0);
}

void ofxCvPlayer::nextFrame() {
	if (player && player->isLoaded()) {
		player->nextFrame();
	}
}

void ofxCvPlayer::previousFrame() {
	if (player && player->isLoaded()) {
		player->preFrame();
	}
}