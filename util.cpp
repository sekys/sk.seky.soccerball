#include "util.h"

Mat computeClusters(Mat src) {
	vector<cv::Mat> imgRGB;
	split(src, imgRGB);
	int k = 3; // dres ma 2 casti, biela ciara, trava
	int n = src.rows *src.cols;
	Mat img3xN(n, 3, CV_8U);
	for(int i=0; i!=3; ++i) {  
		imgRGB[i].reshape(1,n).copyTo(img3xN.col(i));
	}

	img3xN.convertTo(img3xN, CV_32F);
	Mat bestLables;
	kmeans(img3xN, k, bestLables, cv::TermCriteria(), 10, KMEANS_RANDOM_CENTERS );
	bestLables = bestLables.reshape(0, src.rows);
	convertScaleAbs(bestLables, bestLables, int(255/k));
	return bestLables;
}

Mat computeHistogram(Mat src) {
	/// Separate the image in 3 places ( B, G and R )
	vector<Mat> bgr_planes;
	split( src, bgr_planes );

	/// Establish the number of bins
	int histSize = 8;

	/// Set the ranges ( for B,G,R) )
	float range[] = { 0, 256 } ;
	const float* histRange = { range };

	bool uniform = true; 
	bool accumulate = false;

	Mat b_hist, g_hist, r_hist;

	/// Compute the histograms:
	calcHist( &bgr_planes[0], 1, 0, Mat(), b_hist, 1, &histSize, &histRange, uniform, accumulate );
	calcHist( &bgr_planes[1], 1, 0, Mat(), g_hist, 1, &histSize, &histRange, uniform, accumulate );
	calcHist( &bgr_planes[2], 1, 0, Mat(), r_hist, 1, &histSize, &histRange, uniform, accumulate );

	// Draw the histograms for B, G and R
	int hist_w = 512; 
	int hist_h = 400;
	int bin_w = cvRound( (double) hist_w/histSize );

	Mat histImage( hist_h, hist_w, CV_8UC3, Scalar( 0,0,0) );

	/// Normalize the result to [ 0, histImage.rows ]
	normalize(b_hist, b_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat() );
	normalize(g_hist, g_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat() );
	normalize(r_hist, r_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat() );

	/// Draw for each channel
	for( int i = 1; i < histSize; i++ )
	{
		line( histImage, Point( bin_w*(i-1), hist_h - cvRound(b_hist.at<float>(i-1)) ) ,
			Point( bin_w*(i), hist_h - cvRound(b_hist.at<float>(i)) ),
			Scalar( 255, 0, 0), 2, 8, 0  );
		line( histImage, Point( bin_w*(i-1), hist_h - cvRound(g_hist.at<float>(i-1)) ) ,
			Point( bin_w*(i), hist_h - cvRound(g_hist.at<float>(i)) ),
			Scalar( 0, 255, 0), 2, 8, 0  );
		line( histImage, Point( bin_w*(i-1), hist_h - cvRound(r_hist.at<float>(i-1)) ) ,
			Point( bin_w*(i), hist_h - cvRound(r_hist.at<float>(i)) ),
			Scalar( 0, 0, 255), 2, 8, 0  );
	}
	return histImage;
}

// Vypocitaj v kolkych bodoch dochadza k prieniku
int intersectionPoints(vector<Point>& contour, Rect& rec) {
	int pocet = 0;
	for( unsigned int i = 0; i < contour.size(); i++ ) { 
		if (rec.contains(contour[i])) {
			pocet++;
		}
	}
	return pocet;
}

bool comparePoint2f(Point2f& a, Point2f& b) {	
	return ( ((int)a.x) == ((int)b.x) && ((int)a.y) == ((int)b.y));
}

bool containsPixel(vector<Point2f>& a, Point2f& b) {	
	for(int i=0; i < a.size(); i++) {
		if(comparePoint2f(a.at(i), b)) {
			return true;
		}
	}
	return false;
}

int intersection(vector<Point2f>& a, vector<Point2f>&b) {
	int pocet = 0;
	for(int i=0; i < a.size(); i++) {
		if( containsPixel(b, a.at(i)) ) {
			pocet++;
		}
	}
	return pocet;
}

bool isRelativeIntersection(vector<Point>& contour, Rect& rec, const float& volume) {
	int podobnost = intersectionPoints(contour, rec);
	return (podobnost > ((int) contour.size() * volume));
}

void createWindows(const char* windows[], Size size) {
	const size_t len = sizeof(windows) / sizeof(windows[0]);
	for (size_t i = 0; i < len; ++i) {
		const char* meno = windows[i];
		namedWindow(meno);
		resizeWindow(meno, size.width, size.height);
		moveWindow(meno, 0, 0);
	}
}

void drawPoints(Mat&image, vector<Point>& points, Scalar color) {
	vector<vector<Point> > contours;
	contours.push_back(points);
	drawContours(image, contours, 0, color);
}

void drawPoints(Mat&image, vector<Point2f>& points, Scalar color) {
	vector<Point> dst;
	Mat(points).copyTo(dst);
	drawPoints(image, dst, color);
}
double euclideanDist(Point p, Point q) {
    Point diff = p - q;
    return cv::sqrt(diff.x*diff.x + diff.y*diff.y);
}