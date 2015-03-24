#pragma once
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include "entities.hpp"

using namespace cv;
using namespace std;

class ThresholdColor {
private:
	Scalar_<int> m_min;
	Scalar_<int> m_max;

public:
	ThresholdColor(Scalar min, Scalar max) {
		m_min = min;
		m_max = max;
	}

	void createTrackBars(const string& win) {
		createTrackbar( "min h", win, &(m_min.val[0]), 180);
		createTrackbar( "min s", win, &(m_min.val[1]), 255);
		createTrackbar( "min v", win, &(m_min.val[2]), 255);
		createTrackbar( "max h", win, &(m_max.val[0]), 180);
		createTrackbar( "max s", win, &(m_max.val[1]), 255);
		createTrackbar( "max v", win, &(m_max.val[2]), 255);
	}

	Mat getMask(Mat& input) {
		Mat mask, labColor;
		cvtColor(input, labColor, CV_BGR2HSV);
		Scalar min = m_min;
		Scalar max = m_max;
		inRange(labColor, min, max, mask);
		return mask;
	}

	int pixels(Mat& input) {
		return countNonZero( getMask(input) );
	}

	Scalar getMin() {
		return m_min;
	}
};