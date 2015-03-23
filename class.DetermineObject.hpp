#pragma once
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include "entities.hpp"
#include "class.ThresholdColor.hpp"

using namespace cv;
using namespace std;

enum DetectedObjectType { 
	BALL = 1<<0;
	HUMAN = 1<<1;
	BANNER = 1<<2;
	ARTEFACT = 1<<3;  
};
enum HumanType { GOAL_KEEPER, PLAYER, REFEREE, UNKNOWN };
enum TeamsType { A, B, REFEREES };

class FrameObject {
public:
	RotatedRect m_boundary;
	vector<Point> m_countour;

	DetectedObjectType type;
	HumanType human;
	TeamsType team;

	FrameObject(vector<Point> kontura) {
		m_countour = kontura;
		m_boundary = fitEllipse( Mat(kontura) );
	}

	float getSpace() {
		return m_boundary.size.area();;
	}

	int pixels() {
		return m_countour.size();
	}

	friend ostream& operator<< (ostream& out, FrameObject& object) {
		out << "FrameObject(";
		out << "frame: " << object.frame << ",";
		out << ")";
		return out;
	}
};

class DetermineObject {
private:
	// ROzdel podla farby
	ThresholdColor* m_teamB_goalKeeper;
	ThresholdColor* m_teamA_goalKeeper;
	ThresholdColor* m_refereee;
	ThresholdColor* m_teamA;
	ThresholdColor* m_teamB;

	// Filtruj kontury
	int MIN_PIXELS_IN_CONTOUR; 
	int MIN_AREA;
	int MAX_AREA;
	Rect BANNER_AREA;
	float VOLUME_BANNER;

public:
	DetermineObject() {
		MIN_PIXELS_IN_CONTOUR = 15;// 
		MIN_AREA = 20; // 868 velksot hraca
		MAX_AREA = 5000;
		BANNER_AREA = Rect(0, 0, 640, 18);
		VOLUME_BANNER = 0.8f;

		m_teamB_goalKeeper = new ThresholdColor(Scalar(21, 90, 0), Scalar(52, 190, 255));
		m_teamA_goalKeeper = new ThresholdColor(Scalar(21, 90, 0), Scalar(52, 190, 255));
		m_refereee = new ThresholdColor(Scalar(0, 127, 81), Scalar(12, 219, 255));
		m_teamA = new ThresholdColor(Scalar(110, 67, 48), Scalar(141, 150, 158));
		m_teamB = new ThresholdColor(Scalar(0, 0, 245), Scalar(180, 255, 255));
	}

	void findObjects(Mat& image, Mat& mask, vector<RotatedRect>& objects) {
		// Vyber kontury
		vector<vector<Point>> contours;
		findContours(mask, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

		for( unsigned int i = 0; i < contours.size(); i++ ) { 
			FrameObject* obj = new FrameObject(contours[i]); 
			if( obj->pixels() < MIN_PIXELS_IN_CONTOUR) { 
				obj->type = ARTEFACT;
				continue;
			}
			float takeSpace = obj->getSpace();
			if(takeSpace < MIN_AREA) {
				obj->type = ARTEFACT;
				continue;
			}
			if(takeSpace > MAX_AREA) {
				obj->type = BANNER;
				continue;
			}

			if(isRelativeIntersection(m_countour, BANNER_AREA, VOLUME_BANNER)) {
				if(m_debugDraw) drawContours( image, contours, i, Scalar(0,255,0), 1);
				continue;
			}

			objects.push_back(rec);
		}
	}

};