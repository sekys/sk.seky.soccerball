#include "util.h"

int rotatedRectangleIntersection( const RotatedRect& rect1, const RotatedRect& rect2, OutputArray intersectingRegion )
{
	const float samePointEps = 0.00001f; // used to test if two points are the same

	Point2f vec1[4], vec2[4];
	Point2f pts1[4], pts2[4];

	std::vector <Point2f> intersection;

	rect1.points(pts1);
	rect2.points(pts2);

	int ret = INTERSECT_FULL;

	// Specical case of rect1 == rect2
	{
		bool same = true;

		for( int i = 0; i < 4; i++ )
		{
			if( fabs(pts1[i].x - pts2[i].x) > samePointEps || (fabs(pts1[i].y - pts2[i].y) > samePointEps) )
			{
				same = false;
				break;
			}
		}

		if(same)
		{
			intersection.resize(4);

			for( int i = 0; i < 4; i++ )
			{
				intersection[i] = pts1[i];
			}

			Mat(intersection).copyTo(intersectingRegion);

			return INTERSECT_FULL;
		}
	}

	// Line vector
	// A line from p1 to p2 is: p1 + (p2-p1)*t, t=[0,1]
	for( int i = 0; i < 4; i++ )
	{
		vec1[i].x = pts1[(i+1)%4].x - pts1[i].x;
		vec1[i].y = pts1[(i+1)%4].y - pts1[i].y;

		vec2[i].x = pts2[(i+1)%4].x - pts2[i].x;
		vec2[i].y = pts2[(i+1)%4].y - pts2[i].y;
	}

	// Line test - test all line combos for intersection
	for( int i = 0; i < 4; i++ )
	{
		for( int j = 0; j < 4; j++ )
		{
			// Solve for 2x2 Ax=b
			float x21 = pts2[j].x - pts1[i].x;
			float y21 = pts2[j].y - pts1[i].y;

			float vx1 = vec1[i].x;
			float vy1 = vec1[i].y;

			float vx2 = vec2[j].x;
			float vy2 = vec2[j].y;

			float det = vx2*vy1 - vx1*vy2;

			float t1 = (vx2*y21 - vy2*x21) / det;
			float t2 = (vx1*y21 - vy1*x21) / det;

			// This takes care of parallel lines
			if( cvIsInf(t1) || cvIsInf(t2) || cvIsNaN(t1) || cvIsNaN(t2) )
			{
				continue;
			}

			if( t1 >= 0.0f && t1 <= 1.0f && t2 >= 0.0f && t2 <= 1.0f )
			{
				float xi = pts1[i].x + vec1[i].x*t1;
				float yi = pts1[i].y + vec1[i].y*t1;

				intersection.push_back(Point2f(xi,yi));
			}
		}
	}

	if( !intersection.empty() )
	{
		ret = INTERSECT_PARTIAL;
	}

	// Check for vertices from rect1 inside recct2
	for( int i = 0; i < 4; i++ )
	{
		// We do a sign test to see which side the point lies.
		// If the point all lie on the same sign for all 4 sides of the rect,
		// then there's an intersection
		int posSign = 0;
		int negSign = 0;

		float x = pts1[i].x;
		float y = pts1[i].y;

		for( int j = 0; j < 4; j++ )
		{
			// line equation: Ax + By + C = 0
			// see which side of the line this point is at
			float A = -vec2[j].y;
			float B = vec2[j].x;
			float C = -(A*pts2[j].x + B*pts2[j].y);

			float s = A*x+ B*y+ C;

			if( s >= 0 )
			{
				posSign++;
			}
			else
			{
				negSign++;
			}
		}

		if( posSign == 4 || negSign == 4 )
		{
			intersection.push_back(pts1[i]);
		}
	}

	// Reverse the check - check for vertices from rect2 inside recct1
	for( int i = 0; i < 4; i++ )
	{
		// We do a sign test to see which side the point lies.
		// If the point all lie on the same sign for all 4 sides of the rect,
		// then there's an intersection
		int posSign = 0;
		int negSign = 0;

		float x = pts2[i].x;
		float y = pts2[i].y;

		for( int j = 0; j < 4; j++ )
		{
			// line equation: Ax + By + C = 0
			// see which side of the line this point is at
			float A = -vec1[j].y;
			float B = vec1[j].x;
			float C = -(A*pts1[j].x + B*pts1[j].y);

			float s = A*x + B*y + C;

			if( s >= 0 )
			{
				posSign++;
			}
			else
			{
				negSign++;
			}
		}

		if( posSign == 4 || negSign == 4 )
		{
			intersection.push_back(pts2[i]);
		}
	}

	// Get rid of dupes
	for( int i = 0; i < (int)intersection.size()-1; i++ )
	{
		for( size_t j = i+1; j < intersection.size(); j++ )
		{
			float dx = intersection[i].x - intersection[j].x;
			float dy = intersection[i].y - intersection[j].y;
			double d2 = dx*dx + dy*dy; // can be a really small number, need double here

			if( d2 < samePointEps*samePointEps )
			{
				// Found a dupe, remove it
				std::swap(intersection[j], intersection.back());
				intersection.pop_back();
				j--; // restart check
			}
		}
	}

	if( intersection.empty() )
	{
		return INTERSECT_NONE ;
	}

	// If this check fails then it means we're getting dupes, increase samePointEps
	CV_Assert( intersection.size() <= 8 );

	Mat(intersection).copyTo(intersectingRegion);

	return ret;
}

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

bool isRelativeIntersection(vector<Point>& contour, Rect& rec, const float& volume) {
	int podobnost = intersectionPoints(contour, rec);
	return (podobnost > ((int) contour.size() * volume));
}

void createWindows(const char* windows[]) {
	const size_t len = sizeof(windows) / sizeof(windows[0]);
	for (size_t i = 0; i < len; ++i) {
		const char* meno = windows[i];
		namedWindow(meno);
		resizeWindow(meno, 640, 480);
		moveWindow(meno, 0, 0);
	}
}

void drawPoints(Mat&image, vector<Point>& points, Scalar color) {
	vector<vector<Point> > contours;
	contours.push_back(points);
	drawContours(image, contours, 0, color);
}

void drawPoints(Mat&image, vector<Point2f>& points, Scalar color) {
	vector<vector<Point> > contours;
	contours.push_back(points);
	drawContours(image, contours, 0, color);
}