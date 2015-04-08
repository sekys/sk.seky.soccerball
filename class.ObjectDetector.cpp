#include "class.ObjectDetector.hpp"
#include <array>

void ObjectDetector::computeHistogram(Mat& ROI) {
	for ( auto it = histogram.begin(); it != histogram.end(); ++it ) {
		it->count = it->color->pixels(ROI);
	}
	sort(histogram.begin(), histogram.end());
}

void ObjectDetector::determinePerson(Mat& image, Mat& mask, FrameObject* obj) {
	// Person out of playground
	if(isRelativeIntersection(obj->m_countour, BANNER_AREA, VOLUME_BANNER)) {
		obj->type = DetectedObjectType::PERSON;
		return;
	}

	// Prepare data for detecting players
	Mat combinedImageMask;
	image.copyTo(combinedImageMask, mask); 
	Mat ROI = obj->getROI(combinedImageMask);		

	// Count pixels for specific colors
	int pixelsInROI = countNonZero( obj->getROI(mask) );
	computeHistogram(ROI);
	BinInfo info = histogram.back();
	if(info.count < (pixelsInROI * MIN_COLOR_VOLUME)) {
		obj->type = DetectedObjectType::PERSON; //reprezentacia pixelov je velmi mala
		return;
	}
	obj->type = info.type; // prirad typ z farby
}

void ObjectDetector::determineObject(Mat& image, Mat& mask, FrameObject* obj) {
	// Artefakt
	if( obj->pixels() < MIN_PIXELS_IN_CONTOUR) { 
		obj->type = ARTEFACT;
		return;
	}
	float takeSpace = obj->getSpace();
	if(takeSpace < MIN_AREA) {
		obj->type = ARTEFACT;
		return;
	}

	// Banner
	if(takeSpace > MAX_AREA) {
		obj->type = BANNER;
		return;
	}

	// Is it person?
	determinePerson(image, mask, obj);
}

void ObjectDetector::findObjects(Mat& image, Mat& mask, vector<FrameObject*>& objects) {		
	vector<vector<Point>> contours;
	findContours(mask, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE); // Vyber kontury

	for( unsigned int i = 0; i < contours.size(); i++ ) { 
		vector<Point> kontura = contours[i];
		if(kontura.size() < 5) {
			continue;
		}
		FrameObject* obj = new FrameObject(kontura, fitEllipse( Mat(kontura) )); 
		objects.push_back(obj);
		determineObject(image, mask, obj);
	}
}