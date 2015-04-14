#include "class.ObjectTracer.hpp"
#include "util.h"

void ObjectTracer::onMouse( int event, int x, int y, int, void*)
{
    if( event == EVENT_LBUTTONDOWN ) {
        m_point = Point2f((float)x, (float)y);
        m_sendingPoint = true;
    }
}

void ObjectTracer::init() {
	m_sendingPoint = false;
	setMouseCallback( "Vystup", onMouse, 0 );
}

void ObjectTracer::process(Mat& image, vector<FrameObject*>& objs) {
	cvtColor(image, m_gray, COLOR_BGR2GRAY);

	// Vyber novy objekt ak je to potrebne
	if(m_sendingPoint) {
		chooseObject(objs);
	}	

	// Trakuj posledne zvoleny objekt
	if( !m_points[0].empty() ) {
        vector<uchar> status;
        vector<float> error;
        if(m_prevGray.empty()) {
            m_gray.copyTo(m_prevGray);
		}

		// - goodfeatures vygenerujem priznaky a ich trakujem, kym nestratim
		// http://docs.opencv.org/modules/imgproc/doc/feature_detection.html
        calcOpticalFlowPyrLK(m_prevGray, m_gray, m_points[0], m_points[1], status, error);
        size_t i, k;
		vector<Point> lostPoints;
        for( i = k = 0; i < m_points[1].size(); i++ ) {
            if(!status[i]) {
				lostPoints.push_back(m_points[1][i]);
				// Ked narazim na chybu, bod uz nepridam do zoznamu
				continue; 
			}
            m_points[1][k++] =m_points[1][i];            
        }
        m_points[1].resize(k);

		// TODO: Vyhladaj transformaciu 2 bodov estimateRigidTransform
		// TODO: bod spadne z hraca na loptu, pixeli nam spadnu ked su na hranici s ciarou
		// TODO: Ako zistim, ci som ich stratil objekt = mam status, ked 90% pixelov sedi, je to v poriadku
		// TODO: Ked stratim vacsinu pixelov, nerobim nic, napisem error a pauznem

		// Vykreslujeme pointy roznou farbou pre debug pohlad
		Mat debugImage;
		image.copyTo(debugImage);
		drawPoints(debugImage, m_points[0], Scalar(255, 0, 0));
		drawPoints(debugImage, m_points[1], Scalar(0, 255, 0));
		drawPoints(debugImage, lostPoints, Scalar(0, 0, 255));
		imshow("Tracer", debugImage);
    }

	swap(m_points[1], m_points[0]);
    swap(m_prevGray, m_gray);
}

void ObjectTracer::chooseObject(vector<FrameObject*>& objs) {
	m_tracing = NULL;
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
	m_tracing = obj;
	goodFeaturesToTrack(m_gray, m_points[1], TRACE_PIXELS, 0.01, 10, Mat(), 3, 0, 0.04);
}