#include "class.SoccerBall.hpp"
#include <opencv2/imgproc/imgproc.hpp>
#include "util.h"

SoccerBall::SoccerBall() : m_lockFPS(60), m_winSize(640, 480) 
{
	// Nacitaj vsetky casti SoccerBalla
	log = CREATE_LOG4CPP();
	if(log != NULL) {
		log->debug("Starting SoccerBall");
	}
	m_pause = false;
	m_actual = NULL;
	m_roi_index = 0;
}

SoccerBall::~SoccerBall() {
	SAFE_DELETE(m_record);
	SAFE_DELETE(m_pMOG2);
	SAFE_RELEASE(m_actual);
	m_realObjects.clear();
	if(log != NULL) log->debug("Ending SoccerBall");
}

int SoccerBall::getLockFPS() {
	return m_lockFPS;
}

bool SoccerBall::Run() {
	int key = cv::waitKey(30); // caka najmenej 30ms, zalezi od win procesov
	string str;  str =(char) key;
	if(str == "s") {
		m_pause = !m_pause;
	}
	if(str == "e") {		
		m_roi_index++;
		log->debugStream() << "m_roi_index " << m_roi_index;
	}
	if(str == "q") {
		m_roi_index--;
		log->debugStream() << "m_roi_index " << m_roi_index;
	}
	if(!m_pause) {
		nacitajDalsiuSnimku();
	}
	if(m_actual != NULL) {
		spracujJedenSnimok(m_actual);
	}
	return true; // okno obnovujeme stale
}

void SoccerBall::nacitajDalsiuSnimku() {
	// Ziskaj snimok ..
	SAFE_RELEASE(m_actual);
	try {
		m_actual = m_record->readNext();
	} catch(VideoRecord::EndOfStream stream) {
		// Tu nastane 5sec delay ked je koniec streamu, dovod neznamy
		if(log != NULL) {
			log->debugStream() << "Restartujem stream.";
		}
		m_record->doReset();
		delete m_actual;
		m_actual = m_record->readNext();
	}
	if(log != NULL) {
		log->debugStream() << "Snimok: " << m_actual->pos_msec;
	}
}

// ----------------------------------------------------
// ----------------------------------------------------
// ----------------------------------------------------


void SoccerBall::spracujJedenSnimok(Image* in) {
	// Predspracuj vstup podla potreby, vypocitaj fgMasku
	Mat image, fgMaskMOG;
	Size velkostVstupu = m_winSize; //Size(1920, 1080);
	image = in->data.clone();
	resize(image, image, velkostVstupu);
	m_pMOG2->operator()(image, fgMaskMOG);

	// Najdi objekty
	vector<RotatedRect> objects;
	findObjects(image, fgMaskMOG, objects);
	drawROI(image, objects);

	// Vykresli detekovane oblasti
	for( UINT i = 0; i < objects.size(); i++ ) { 
		ellipse(image, objects[i], Scalar(0,0,255), 1);
	}

	resize(image, image, m_winSize);
	resize(fgMaskMOG, fgMaskMOG, m_winSize);
	imshow("Vystup", image);
	imshow("Maska", fgMaskMOG);
}

void SoccerBall::buildSoccerObjects(vector<RotatedRect>& imageObjects, vector<SoccerObject>& realObjects) {
	const float VOLUME_BANNER = 0.8f;

	for( UINT a = 0; a < realObjects.size(); a++ ) { 
		for( UINT b = 0; b < imageObjects.size(); b++ ) { 
			// imageObjects[i]
			// realObjects.push_back(rec);
		}
	}
}

int SoccerBall::identifySoccerObjects(RotatedRect& rect) {
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

	return 0;
}


// Find ROI or select main image
void SoccerBall::drawROI(Mat& image, vector<RotatedRect>& objects) {
	Mat ROI;
	UINT size = objects.size();
	if(size > 0) {
		UINT index =  m_roi_index % size;
		Rect rec = objects[index].boundingRect();
		rec = rec & Rect(0, 0, image.cols, image.rows);
		ROI = Mat(image, rec).clone(); 
		
		// Vykresli pre obraz cluster a histogram
		/*
		Mat labels = computeClusters(ROI);
		resize(labels, labels, m_winSize);
		imshow("result", labels);	
		Mat histImage = computeHistogram(ROI); 
		imshow("Histogram", histImage );
		*/
	} else {
		ROI = Mat(image);
	}
	resize(ROI, ROI, m_winSize);
	imshow("Vybrana sekcia", ROI);
}

// Find objects
void SoccerBall::findObjects(Mat& image, Mat& fgMaskMOG, vector<RotatedRect>& objects) {
	// Vyber kontury
	vector<vector<Point>> contours;
	erode(fgMaskMOG, fgMaskMOG, Mat(), Point(-1,-1), 1);
	dilate(fgMaskMOG, fgMaskMOG, Mat(), Point(-1,-1), 4);
	findContours(fgMaskMOG, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

	// Spravuj a filtruj kontury
	const int MIN_PIXELS_IN_CONTOUR = 36;// 
	const int MIN_AREA = 100; // 868 velksot hraca
	const int MAX_AREA = 5000;
	Rect BANNER_AREA(0, 0, 640, 36);
	const float VOLUME_BANNER = 0.8f;
	const bool debugDraw = false;

	for( UINT i = 0; i < contours.size(); i++ ) { 
		vector<Point> kontura = contours[i];
		if( kontura.size() < MIN_PIXELS_IN_CONTOUR) { 
			if(debugDraw) drawContours( image, contours, i, Scalar(255,0,0), 1);
			continue;
		}
		RotatedRect rec = fitEllipse( Mat(kontura) );
		float takeSpace = rec.size.area();
		if(takeSpace < MIN_AREA || takeSpace > MAX_AREA) {
			if(debugDraw) drawContours( image, contours, i, Scalar(255,0,0), 1);
			continue;
		}

		if(isRelativeIntersection(kontura, BANNER_AREA, VOLUME_BANNER)) {
			if(debugDraw) drawContours( image, contours, i, Scalar(0,255,0), 1);
			continue;
		}

		objects.push_back(rec);
	}
}

void SoccerBall::Init() {
	m_record = new VideoRecord("data/filmrole5.avi");
	m_pMOG2 = new BackgroundSubtractorMOG2(3, 16.0, false);

	const char* const windows[] = { "Vystup", "Maska", "Vybrana sekcia", "Histogram", "result" };
	const size_t len = sizeof(windows) / sizeof(windows[0]);
	for (size_t i = 0; i < len; ++i) {
		const char* meno = windows[i];
		namedWindow(meno);
		resizeWindow(meno, 640, 480);
		moveWindow(meno, 0, 0);
	}
}