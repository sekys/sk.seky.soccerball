#pragma once
#include <opencv2/core/core.hpp>
#include "opencv2/opencv.hpp"

using namespace cv;

Mat computeClusters(Mat src);
Mat computeHistogram(Mat src);
int intersectionPoints(vector<Point>& contour, Rect& rec);
bool isRelativeIntersection(vector<Point>& contour, Rect& rec, const float& volume);
void createWindows(const char* windows[]);
void drawPoints(Mat& image, vector<Point>& points, Scalar color);
void drawPoints(Mat& image, vector<Point2f>& points, Scalar color);
bool containsPixel(vector<Point2f>& a, Point2f& b);
int intersection(vector<Point2f>& a, vector<Point2f>&b);
bool comparePoint2f(Point2f& a, Point2f& b);
double euclideanDist(Point p, Point q);