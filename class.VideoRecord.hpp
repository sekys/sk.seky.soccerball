#pragma once
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include "entities.hpp"
#include <queue>

using namespace cv;
using namespace std;

class VideoRecord {

private:
	// http://docs.opencv.org/modules/highgui/doc/reading_and_writing_images_and_video.html#videocapture
	VideoCapture* cap;
	string filename;
	queue<Image*> buffer;

	void load() {
		if(filename.empty()) {
			// TODO: VideoCapture nacitava vlastny DLL subor a moze sposobit loader dead lock
			cap = new VideoCapture(0);
		} else {
			cap = new VideoCapture(filename);
		}
		if(!cap->isOpened()) {
			throw runtime_error("[ModulKamera] I cannot open device / file.");
		}
	}

	void unload() {
		if(cap != NULL) {
			if(cap->isOpened()) {
				cap->release();
			}
			delete cap;
			cap = NULL;
		}
	}

public:

	class EndOfStream : public std::exception {
	public:
		EndOfStream() : std::exception("EndOfStream") {}
	};

	VideoRecord(const string filename) {
		this->filename = filename;
		load();
	}

	~VideoRecord() {
		unload();
	}

	// Ziskaj aktualnu snimku z kamery
	Image* readNext() {
		Image* frame = new Image();
		if(!cap->read(frame->data)) {
			throw EndOfStream();
		}
		frame->pos_msec = cap->get(CV_CAP_PROP_POS_MSEC);
		return frame;
	}

	Image* bufferNext() {
		Image* frame = new Image();
		if(buffer.empty()) {
			throw EndOfStream();
		}
		frame = buffer.front();
		buffer.pop();
		frame->pos_msec = cap->get(CV_CAP_PROP_POS_MSEC);
		return frame;
	}

	void readAll() {
		try {
			for(int i=0; i < 300; i++) {
				Image* frame = this->readNext();
				buffer.push(frame);
			}
		} catch(VideoRecord::EndOfStream stream) {

		}
	}

	void doReset() {
		unload();
		load();
		// cap->set(CV_CAP_PROP_POS_MSEC, 0); toto sposobuje delay a je to tak neefektivne
	}
};