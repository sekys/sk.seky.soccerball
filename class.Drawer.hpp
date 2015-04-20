#pragma once
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include "entities.hpp"
#include "log4cpp.h"
#include "class.ThresholdColor.hpp"

using namespace cv;
using namespace std;

class Drawer {
private:
	static Size WIN_SIZE;
	log4cpp::Category* log;
	ThresholdColor* m_roi;
	int m_roi_index;
	int m_drawType;
	bool m_roiDraw;
	bool m_debugDraw;
	bool m_teamColoring;

	void drawROI(Mat& image, Mat& mask, vector<FrameObject*>& objs);
	bool isVisible(FrameObject* obj);
	Scalar determineColor(FrameObject* obj);

public:
	Drawer();
	~Drawer();

	void previousROI();
	void nextROI();
	void switchROIDraw();
	void switchDebugDraw();
	void switchTeamColoring();
	void switchDrawType();
	void draw(Mat& image, Mat& mask, vector<FrameObject*>& objs);
};