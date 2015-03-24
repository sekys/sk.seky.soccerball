#pragma once
#include <opencv2/core/core.hpp>
#include "opencv2/opencv.hpp"
#include <queue> 

#define SAFE_DELETE(a) if( (a) != NULL ) delete (a); (a) = NULL;
#define SAFE_RELEASE(p) if ( (p) != NULL  ) (p)->release(); (p) = NULL;

using namespace cv;
using namespace std;

/*
* Opencv examples
*/
inline ostream& operator<< (ostream& out, Point2f& object) {
	out << "Point2f(x:" << object.x << ", y:" << object.y << ")";
	return out;
}

inline ostream& operator<< (ostream& out, Point3f& object) {
	out << "Point3f(x:" << object.x << ", y:" << object.y << ", z:" << object.z << ")";
	return out;
}

inline ostream& operator<< (ostream& out, RotatedRect& object) {
	out << "RotatedRect(center:" << object.center << ", size:" << object.size << ")";
	return out;
}

/**
* Trieda pre obrazok ako univerzalna entita v ramci Carlos.
*/
class Frame {
public:
	double pos_msec; // urcuje poziciu snimky vo video v ms
	Mat data;

	friend ostream& operator<< (ostream& out, Frame& object) {
		out << "Image(pos_msec:" << object.pos_msec << ")";
		return out;
	}
	Frame* clone() {
		Frame* temp = new Frame();
		temp->pos_msec = this->pos_msec;
		temp->data = this->data.clone();
		return temp;
	}
	void release() {
		data.release();
	}
};

enum DetectedObjectType { 
	UNKNOWN,
	ARTEFACT,
	BANNER, 
	PERSON,  // out playground 
	GOAL_KEEPER_A, 
	GOAL_KEEPER_B, 
	PLAYER_A, 
	PLAYER_B, 
	REFEREE,
	BALL
};

inline ostream& operator<< (ostream& out, DetectedObjectType& type) {
	switch (type)
	{
	case UNKNOWN:   out << "UNKNOWN"; break;
	case ARTEFACT:   out << "ARTEFACT"; break;
	case BANNER: out << "BANNER"; break;
	case PERSON: out << "PERSON"; break;
	case GOAL_KEEPER_A: out << "GOAL_KEEPER_A"; break;
	case GOAL_KEEPER_B: out << "GOAL_KEEPER_B"; break;
	case PLAYER_A: out << "PLAYER_A"; break;
	case PLAYER_B: out << "PLAYER_B"; break;
	case REFEREE: out << "REFEREE"; break;
	case BALL: out << "BALL"; break;
	default:      out << "NULL"; break;
	}
	return out;
}

class FrameObject {
public:
	RotatedRect m_boundary;
	vector<Point> m_countour;
	DetectedObjectType type;

	FrameObject(vector<Point> kontura, RotatedRect rec) {
		m_countour = kontura;
		m_boundary = rec;
		type = DetectedObjectType::UNKNOWN;
	}

	Mat getROI(Mat& image) {
		Rect rec =  m_boundary.boundingRect();
		rec = rec & Rect(0, 0, image.cols, image.rows);
		return Mat(image, rec);
	}

	float getSpace() {
		return m_boundary.size.area();;
	}

	int pixels() {
		return m_countour.size();
	}

	friend ostream& operator<< (ostream& out, FrameObject& object) {
		out << "FrameObject(";
		out << "type: " << object.type << ",";
		out << ")";
		return out;
	}
};