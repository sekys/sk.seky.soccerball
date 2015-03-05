#pragma once
#include <opencv2/core/core.hpp>
#include "opencv2/opencv.hpp"

#define SAFE_DELETE(a) if( (a) != NULL ) delete (a); (a) = NULL;

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
};

/**
* Trieda reprezentuje objekt detekovany na obrazovke videa.
* Dany objekt odkazuje na svetovy objekt
*/
class DetekovanyObjekt {
public:
	RotatedRect boundary; /**< ojekt ma velkost rectange, to je aj jeho pozicia na obrazku */

	friend ostream& operator<< (ostream& out, DetekovanyObjekt& object) {
		out << "DetekovanyObjekt(";
		out << "objekt: " << object.boundary << ",";
		out << ")";
		return out;
	}
};