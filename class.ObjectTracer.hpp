#pragma once
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include "entities.hpp"
#include "class.ThresholdColor.hpp"
#include "util.h"

using namespace cv;
using namespace std;


class ObjectTracer {
private:	
	static bool m_sendingPoint;
	static Point2f m_point;
    int TRACE_PIXELS;
	FrameObject* m_tracing;
	vector<Point2f> m_points[2];
	Mat m_gray, m_prevGray;

	void chooseObject(vector<FrameObject*>& objs);
	void init();
	void startTrace(FrameObject* obj);


public:
	ObjectTracer() {
		m_tracing = NULL;
		TRACE_PIXELS = 100; 
		init();		
	}

	static void onMouse( int event, int x, int y, int, void* );
	void process(Mat& image, vector<FrameObject*>& objs);
};