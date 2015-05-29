#include "class.ObjectTracer.hpp"
#include "util.h"

bool ObjectTracer::m_sendingPoint;
Point ObjectTracer::m_point;

void ObjectTracer::onMouse( int event, int x, int y, int, void*)
{
    if( event == EVENT_LBUTTONDOWN ) {
        m_point = Point(x, y);
        m_sendingPoint = true;
    }
}

void ObjectTracer::init() {
	m_sendingPoint = false;
	namedWindow("Vystup");
	setMouseCallback( "Vystup", onMouse, 0 );
}

void ObjectTracer::process(Mat& image, vector<FrameObject*>& objs) {
	cvtColor(image, m_gray, COLOR_BGR2GRAY);
	if(m_prevGray.empty()) {
		m_gray.copyTo(m_prevGray);
	}

	traceObjects(image, objs);
	checkNewObjects(objs);
	swap(m_prevGray, m_gray);
}

void ObjectTracer::checkNewObjects(vector<FrameObject*>& objs) {
	// Vyber novy objekt ak je to potrebne
	for ( auto it = objs.begin(); it != objs.end(); ++it ) {
		FrameObject* obj = (*it);
		if(obj->hasHistory() || !canTrace(obj)) {
			continue; // trackujeme len urcite objekty
		}

		if(m_sendingPoint) {
			m_sendingPoint = false;
			double result = pointPolygonTest( (*it)->m_countour, m_point, false);
			if(result > 0.0) {
				// Pouzivatel si vybral prave tuto konturu
				startTrace((*it));
				break;
			}
		}
		//startTrace(obj);
	}

}

vector<Point> ObjectTracer::mapPixels(vector<Point2f>& in, vector<Point2f>& out) {
	vector<uchar> status;
	vector<float> error;
	calcOpticalFlowPyrLK(m_prevGray, m_gray, in, out, status, error);
	size_t i, k;
	vector<Point> lostPoints;
	for( i = k = 0; i < out.size(); i++ ) {
		if(!status[i]) {
			lostPoints.push_back(out[i]);
			continue; // Ked narazim na chybu, bod uz nepridam do zoznamu
		}
		out[k++] = out[i];            
	}
	out.resize(k);
	return lostPoints;
}

void ObjectTracer::traceObjects(Mat& image, vector<FrameObject*>& objs) {
	for (int i=0; i < m_footprints.size(); i++) { 
		TraceTrack* track = m_footprints.at(i);

		// Trakuj  zvoleny objekt
		if( !track->points[0].empty() ) {
			int pixelovNaVstupe = track->points[0].size();
			vector<Point> lostPoints = mapPixels(track->points[0], track->points[1]);

			// Ked stratim vacsinu pixelov, nerobim nic, napisem error
			if(((int) (pixelovNaVstupe * LOST_OBJECT_PERCENT)) < lostPoints.size()) {
				m_footprints.erase(m_footprints.begin() + i);
				lostTracing(track); 
				i--;
				continue;
			}

			// Vykreslujeme body roznou farbou pre debug pohlad
			Mat debugImage;
			image.copyTo(debugImage);
			drawPoints(debugImage, track->points[0], Scalar(255, 0, 0)); //bgr
			drawPoints(debugImage, track->points[1], Scalar(0, 255, 0));
			drawPoints(debugImage, lostPoints, Scalar(0, 0, 255));
			imshow("tracker", debugImage);
			//cv::waitKey();
			compareObjects(track, objs);
		}
	}
}

void ObjectTracer::compareObjects(TraceTrack* track, vector<FrameObject*>& objs) {
	vector<Point2f> points;
	for (int i=0; i < objs.size(); i++) { 
		FrameObject* obj = objs.at(i);
		if(!canTrace(obj)) {
			continue;
		}
		Mat(obj->getLocations()).copyTo(points); 
		int pocet = intersection(track->points[1], points);
		int namapovanychBodov = track->points[1].size();
		if((namapovanychBodov * EQUALS_OBJECT_PERCENT) < pocet) {
			obj->m_previous = track->tracing;
			track->tracing = obj;
			track->points[1] = points;
			break; // EQUALS_OBJECT_PERCENT sanca nam staci na potvrdenie objektu
		}
	}

	swap(track->points[1], track->points[0]); // ked sa rozhodnem nove body nepriradit, musim stale vymenit vystup za novy vstup
}

void ObjectTracer::lostTracing(TraceTrack* obj) {
	if(log != NULL) {
		log->debug("Stratil som objekt.");
	}
	SAFE_DELETE(obj);
}

bool ObjectTracer::canTrace(FrameObject* obj) {
	return (
			obj->type == GOAL_KEEPER_A 
			|| obj->type == GOAL_KEEPER_B
			|| obj->type == PLAYER_A
			|| obj->type == PLAYER_B
			|| obj->type == REFEREE
		); // trackujeme len urcite objekty
}

void ObjectTracer::startTrace(FrameObject* obj) {
	vector<Point2f> points;
	Mat(obj->getLocations()).copyTo(points);  

	TraceTrack* track = new TraceTrack();
	track->tracing = obj;
	track->points[0] = points;
	m_footprints.push_back(track);
	if(log != NULL) {
		log->debug("Zacinam trakovat.");
	}
	/*
	Mat debugImage(image.cols, image.rows, CV_8UC1, Scalar(0, 0, 0));
	drawPoints(debugImage, m_points[1], Scalar(255));
	imshow("Tracer", debugImage);
	cv::waitKey();
	*/
}