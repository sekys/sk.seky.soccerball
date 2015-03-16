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
class Image {
public:
	double pos_msec; // urcuje poziciu snimky vo video v ms
	Mat data;

	friend ostream& operator<< (ostream& out, Image& object) {
		out << "Image(pos_msec:" << object.pos_msec << ")";
		return out;
	}
	Image* clone() {
		Image* temp = new Image();
		temp->pos_msec = this->pos_msec;
		temp->data = this->data.clone();
		return temp;
	}
	void release() {
		data.release();
	}
};

/**
* Trieda reprezentuje objekt detekovany na obrazovke videa.
* Dany objekt odkazuje na svetovy objekt
*/
class FrameObject {
public:
	RotatedRect boundary;
	int frame;

	friend ostream& operator<< (ostream& out, FrameObject& object) {
		out << "FrameObject(";
		out << "boundary: " << object.boundary << ",";
		out << "frame: " << object.frame << ",";
		out << ")";
		return out;
	}
};

class SoccerObject {
private:
	bool staticObj;
	bool visible;

public:
	UINT id; // generovane ID
	queue<FrameObject> positions; // TODO: moze byt obmedzeny na 20snimkov a info budeme ukladat inde
	// int speed; // jeho posledne zachytena rychlost aby smem sledoval ikde asi bude
	UINT team;

	SoccerObject() {
		staticObj = false;
		visible = true;
	}

	int distance() {
		// Vypocitaj vzdialenost, ktoru presiel z historie
	}

	friend ostream& operator<< (ostream& out, SoccerObject& object) {
		out << "SoccerObject(";
		out << "id: " << object.id << ",";
		//out << "speed: " << object.speed << ",";
		out << ")";
		return out;
	}

	/*
	bool identify(RotatedRect& rect) {
		const double MAX_DISTANCE = 100;

			for( UINT a = 0; a < m_realObjects.size(); a++ ) { 
			FrameObject& frame = m_realObjects[a].positions.back();
			double distance = norm(frame.boundary.center - rect.center);
			if(distance > MAX_DISTANCE) {
				continue; // objekty su priliz vzdialene
			}
			int intersection = rotatedRectangleIntersection(rect, frame.boundary, Mat()); 
			if(intersection == 0) {
				continue; // objekty sa nedotykaju, nevieme zistit ci ide o ten isty objekt
			}
		}
	}*/
};