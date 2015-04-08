#include "class.Drawer.hpp"

Drawer::Drawer() {
	m_roi_index = 0;
	m_roiDraw = false;
	m_debugDraw = false;
	m_teamColoring = false;
	m_drawType = 0;
	m_roi = new ThresholdColor(Scalar(35, 72, 50), Scalar(51, 142, 144));
	m_roi->createTrackBars("roiMask");

	log = CREATE_LOG4CPP();
	if(log != NULL) {
		log->debug("Starting drawer");
	}
}
Drawer::~Drawer() {
	SAFE_DELETE(m_roi);
}

void Drawer::switchTeamColoring() {
	m_teamColoring = !m_teamColoring; 
	log->debugStream() << "m_teamColoring " << m_teamColoring;
}
void Drawer::switchDrawType() {
	m_drawType = (m_drawType + 1) % 3; 
	log->debugStream() << "m_drawType " << m_drawType;
}
void Drawer::switchDebugDraw() {
	m_debugDraw = !m_debugDraw; 
	log->debugStream() << "m_debugDraw " << m_debugDraw;
}
void Drawer::switchROIDraw() {
	m_roiDraw = !m_roiDraw;
	log->debugStream() << "m_roiDraw " << m_roiDraw;
}
void Drawer::nextROI() {
	m_roi_index++;
	log->debugStream() << "next roi ";
}
void Drawer::previousROI() {
	m_roi_index--;
	log->debugStream() << "previous roi ";
}
void Drawer::draw(Mat& image, Mat& mask, vector<FrameObject*>& objs) {
	if(m_roiDraw) {
		drawROI(image, mask, objs);
	}

	// Vykresli detekovane oblasti
	for( UINT i = 0; i < objs.size(); i++ ) { 
		FrameObject* obj = objs.at(i);
		if(!isVisible(obj)) {
			continue;
		}
		Scalar color = determineColor(obj); 
		if(m_drawType == 0) {
			ellipse(image, objs.at(i)->m_boundary, color, 1);
		} else if(m_drawType == 2) {
			ostringstream os;
			os << obj->type;
			putText(image, os.str(), obj->m_boundary.center, 0, 0.2, color, 1, 8);
		} else if(m_drawType == 1) {
			vector<vector<Point> > contours;
			contours.push_back(objs.at(i)->m_countour);
			drawContours(image, contours, 0, color);
		}
	}
	imshow("Vystup", image);
}

bool Drawer::isVisible(FrameObject* obj) {
	if((obj->type == ARTEFACT || obj->type == BANNER )) {
		return m_debugDraw;
	}
	return true;
}

Scalar Drawer::determineColor(FrameObject* obj) {
	// bgr
	if(obj->type == ARTEFACT) {
		return Scalar(0, 0, 255);
	}
	if(obj->type == BANNER) {
		return Scalar(0, 0, 255);
	}
	if(obj->type == BALL) {
		return Scalar(255, 0, 255);
	}
	if(obj->type == PERSON) {
		return Scalar(60, 60, 60);
	}
	if(obj->type == REFEREE) {
		return Scalar(0, 0, 255);
	}
	if(m_teamColoring && obj->type == GOAL_KEEPER_A) {
		return Scalar(255, 255, 255);
	}
	if(obj->type == GOAL_KEEPER_A) {
		return Scalar(0, 220, 220);
	}
	if(obj->type == PLAYER_A) {
		return Scalar(255, 0, 0);
	}
	if(obj->type == PLAYER_B) {
		return Scalar(255, 255, 255);
	}
	return Scalar(0, 0, 0);
}

void Drawer::drawROI(Mat& image, Mat& mask, vector<FrameObject*>& objs) {
	// Find ROI or select main image
	Mat ROI;
	UINT size = objs.size();
	Size winSize = Size(640, 480);
	if(size > 0) {
		Mat combinedImageMask;
		image.copyTo(combinedImageMask, mask); 
		ROI = objs.at( m_roi_index % size )->getROI(combinedImageMask);		

		//Mat roiMask = m_roi->getMask(ROI);
		//resize(roiMask, roiMask, winSize);
		//imshow("roiMask", roiMask);
		//Mat histogram = computeHistogram(labColor);
		//resize(histogram, histogram, m_winSize);
		//imshow("Histogram HSV",  computeHistogram(labColor) );
		//Mat labels = computeClusters(ROI);
		//resize(labels, labels, m_winSize);
		//imshow("result", labels);	
	} else {
		ROI = Mat(image);
	}
	resize(ROI, ROI, winSize);
	imshow("Vybrana sekcia", ROI);
}
