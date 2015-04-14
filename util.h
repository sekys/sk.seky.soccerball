#pragma once
#include <opencv2/core/core.hpp>
#include "opencv2/opencv.hpp"

using namespace cv;

#define INTERSECT_NONE 0 //- No intersection
#define INTERSECT_PARTIAL 1 //- There is a partial intersection
#define INTERSECT_FULL 2 //- One of the rectangle is fully enclosed in the other

// Source: http://sourceforge.net/p/emgucv/opencv/ci/1547fe0ed8d01c1d8a7162a478f5bbaeff975bc5/tree/modules/imgproc/src/intersection.cpp
// DOcumentation: http://docs.opencv.org/trunk/modules/imgproc/doc/structural_analysis_and_shape_descriptors.html#rotatedrectangleintersection
int rotatedRectangleIntersection( const RotatedRect& rect1, const RotatedRect& rect2, OutputArray intersectingRegion );

Mat computeClusters(Mat src);
Mat computeHistogram(Mat src);
int intersectionPoints(vector<Point>& contour, Rect& rec);
bool isRelativeIntersection(vector<Point>& contour, Rect& rec, const float& volume);
void createWindows(const char* windows[]);
void drawPoints(Mat& image, vector<Point>& points, Scalar color);
void drawPoints(Mat& image, vector<Point2f>& points, Scalar color);