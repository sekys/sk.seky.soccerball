#pragma once
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include "entities.hpp"
#include "class.ThresholdColor.hpp"
#include "util.h"
#include "log4cpp.h"

using namespace cv;
using namespace std;


class ObjectTracer {
private:	
	log4cpp::Category* log;

	// Konstanty
	static bool m_sendingPoint;
	static Point m_point;
	float LOST_OBJECT_PERCENT;
	float EQUALS_OBJECT_PERCENT;

	// Informnacie pre trakovanie
	class TraceTrack {
	public:
		vector<Point2f> points[2];
		FrameObject* tracing;
	};
	vector<TraceTrack*> m_footprints;
	Mat m_gray, m_prevGray;

	// Rozdelena logika do metod
	void checkNewObjects(vector<FrameObject*>& objs);
	void init();
	void startTrace(FrameObject* obj);
	void traceObjects(Mat& image, vector<FrameObject*>& objs);
	vector<Point> mapPixels(vector<Point2f>& in, vector<Point2f>& out); // return lost points
	void lostTracing(TraceTrack* obj);
	void compareObjects(TraceTrack* obj, vector<FrameObject*>& objs);
	bool canTrace(FrameObject* obj);

public:
	ObjectTracer() {
		log = CREATE_LOG4CPP();
		LOST_OBJECT_PERCENT = 0.3; 
		EQUALS_OBJECT_PERCENT = 0.7;
		init();		
	}

	static void onMouse( int event, int x, int y, int, void* );
	void process(Mat& image, vector<FrameObject*>& objs);
};
