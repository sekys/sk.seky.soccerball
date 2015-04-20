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

	// Vyber novy objekt ak je to potrebne
	if(m_sendingPoint) {
		chooseObject(objs);
	}	
	traceObjects(image, objs);
	swap(m_prevGray, m_gray);
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
		TraceTrack* trace = m_footprints.at(i);

		// Trakuj  zvoleny objekt
		if( !trace->points[0].empty() ) {
			int pixelovNaVstupe = trace->points[0].size();
			vector<Point> lostPoints = mapPixels(trace->points[0], trace->points[1]);

			// Ked stratim vacsinu pixelov, nerobim nic, napisem error
			if(((int) (pixelovNaVstupe * LOST_OBJECT_PERCENT)) < lostPoints.size()) {
				m_footprints.erase(m_footprints.begin() + i);
				lostTracing(trace); 
				i--;
				continue;
			}

			// Vykreslujeme body roznou farbou pre debug pohlad
			Mat debugImage;
			image.copyTo(debugImage);
			drawPoints(debugImage, trace->points[0], Scalar(255, 0, 0)); //bgr
			drawPoints(debugImage, trace->points[1], Scalar(0, 255, 0));
			drawPoints(debugImage, lostPoints, Scalar(0, 0, 255));
			imshow("Tracer", debugImage);
			//cv::waitKey();

			compareObjects(trace, objs);
		}
	}
}

void ObjectTracer::compareObjects(TraceTrack* trace, vector<FrameObject*>& objs) {

	// TODO: Teraz tu zistit, ze stare body sedia z ktorym objektom novym .. ma najviac spolocnych bodov
	// Mozem vyberat ludi alebo aj zvysne objekty .... tie zvysne sa casom mozu stat typu ludia, podla historie
	// casom = Ked dane body sedia viac ako 10 snimkov za sebou, som si isty, ze ten objekt je clovek, je v pohybe
	// Potom tieto body nahradit novymi z daneho objektu
	// Ukladat historiu pohybu objektu

	swap(trace->points[1], trace->points[0]); // ked sa rozhodnem nove body nepriradit, musim aspon vymenit vyustp za novy vstup
}

void ObjectTracer::lostTracing(TraceTrack* obj) {
	if(log != NULL) {
		log->debug("Stratil som objekt.");
	}
	SAFE_DELETE(obj);
}

void ObjectTracer::chooseObject(vector<FrameObject*>& objs) {
	m_sendingPoint = false;

	for ( auto it = objs.begin(); it != objs.end(); ++it ) {
		FrameObject* obj = (*it);
		if( !(
			obj->type == GOAL_KEEPER_A 
			|| obj->type == GOAL_KEEPER_B
			|| obj->type == PLAYER_A
			|| obj->type == PLAYER_B
			|| obj->type == REFEREE
		)) {
			continue; // trackujeme len urcite objekty
		}

		double result = pointPolygonTest( (*it)->m_countour, m_point, false);
		if(result > 0.0) {
			// Pouzivatel si vybral prave tuto konturu
			startTrace((*it));
			break;
		}
	}
}

void ObjectTracer::startTrace(FrameObject* obj) {
	vector<Point2f> points;
	Mat(obj->getLocations()).copyTo(points);  

	TraceTrack* trace = new TraceTrack();
	trace->tracing = obj;
	trace->points[0] = points;
	m_footprints.push_back(trace);
	/*
	Mat debugImage(image.cols, image.rows, CV_8UC1, Scalar(0, 0, 0));
	drawPoints(debugImage, m_points[1], Scalar(255));
	imshow("Tracer", debugImage);
	cv::waitKey();
	*/
}