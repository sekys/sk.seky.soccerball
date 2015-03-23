#include "class.SoccerBall.hpp"
#include <opencv2/imgproc/imgproc.hpp>
#include "util.h"

SoccerBall::SoccerBall() 
{
	// Nacitaj vsetky casti SoccerBalla
	log = CREATE_LOG4CPP();
	if(log != NULL) {
		log->debug("Starting SoccerBall");
	}
}

SoccerBall::~SoccerBall() {
	SAFE_DELETE(m_record);
	SAFE_DELETE(m_pMOG2);
	SAFE_DELETE(m_roi);
	SAFE_DELETE(m_grass);
	SAFE_RELEASE(m_actual);
	m_realObjects.clear();
	if(log != NULL) log->debug("Ending SoccerBall");
}

int SoccerBall::getLockFPS() {
	return 60;
}

bool SoccerBall::Run() {
	int key = cv::waitKey(30);
	string str;  
	str =(char) key;
	commandArrive(str);
	if(!m_pause) {
		loadNextFrame();
	}
	if(m_actual != NULL) {
		processFrame(m_actual);
	}
	return true;
}

void SoccerBall::commandArrive(string& cmd) {
	// Pauza
	if(cmd == "s") {
		m_pause = !m_pause;
		log->debugStream() << "m_pause " << m_pause;
		return;
	}
	if(cmd == "d") {		
		m_debugDraw = !m_debugDraw; 
		log->debugStream() << "m_debugDraw " << m_debugDraw;
		return;
	}

	// ROI oblast
	if(cmd == "w") {		
		m_roiDraw = !m_roiDraw;
		log->debugStream() << "m_roiDraw " << m_roiDraw;
		return;
	}
	if(cmd == "e") {		
		m_roi_index++;
		log->debugStream() << "m_roi_index " << m_roi_index;
		return;
	}
	if(cmd == "q") {
		m_roi_index--;
		log->debugStream() << "m_roi_index " << m_roi_index;
		return;
	}
}

void SoccerBall::loadNextFrame() {
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

void SoccerBall::processFrame(Frame* in) {
	// Predspracuj vstup podla potreby, vypocitaj fgMasku
	Size velkostVstupu = m_winSize; //Size(1920, 1080);
	resize(m_actual->data, m_actual->data, velkostVstupu);

	// Learning MOG algorithm
	if(m_learning) {
		if(in->pos_msec < m_mogLearnFrames) {
			Mat mask;
			m_pMOG2->operator()(in->data, mask, 1.0 / m_mogLearnFrames);
			return;
		}

		if(in->pos_msec == m_mogLearnFrames) {
			m_record->doReset();
			m_learning = false;
			log->debugStream() << "Koniec ucenia.";
			log->debugStream() << "Restartujem stream.";
			return;
		}
		return;
	} 
	
	// Pauza pre konkretny snimok
	if(in->pos_msec == 490) {
		m_pause = true;
	}

	processImage(m_actual->data.clone());
}

void SoccerBall::processImage(Mat& input) {
	// Ziskaj masku pohybu cez MOG algoritmus
	Mat mogMask;
	m_pMOG2->operator()(input, mogMask);
	erode(mogMask, mogMask, Mat(), Point(-1,-1), 1);
	dilate(mogMask, mogMask, Mat(), Point(-1,-1), 3);
	//imshow("mogMask", mogMask); 

	// Ziskaj masku travy cez farbu
	Mat grassMask = m_grass->getMask(input);
	bitwise_not(grassMask, grassMask);
	//imshow("grassMask",grassMask); 
	
	// Vypracuj spolocnu masku
	Mat finalMask;
	bitwise_and(grassMask, mogMask, finalMask);
	//imshow("finalMask", finalMask); 
	//opticalFlow(oper, oper);
	//imshow("oper2", oper); 
	
	// Najdi objekty
	vector<RotatedRect> objects;
	findObjects(input, finalMask, objects);
	if(m_roiDraw) drawROI(input, finalMask, objects);

	// Vykresli detekovane oblasti
	for( UINT i = 0; i < objects.size(); i++ ) { 
		ellipse(input, objects[i], Scalar(0,0,255), 1);
	}
	imshow("Vystup", input);
}

void SoccerBall::drawROI(Mat& image, Mat& mask, vector<RotatedRect>& objects) {
	// Find ROI or select main image
	Mat ROI;
	UINT size = objects.size();
	if(size > 0) {
		UINT index =  m_roi_index % size;
		Rect rec = objects[index].boundingRect();
		rec = rec & Rect(0, 0, image.cols, image.rows);
		ROI = Mat(image, rec).clone(); 

		// Vykresli pre obraz cluster a histogram
		Mat prehlad;
		ROI.copyTo(prehlad, Mat(mask, rec)); 
		ROI = prehlad;

		Mat roiMask = m_roi->getMask(ROI);
		resize(roiMask, roiMask, m_winSize);
		imshow("roiMask", roiMask);
		//Mat histogram = computeHistogram(labColor);
		//resize(histogram, histogram, m_winSize);
		//imshow("Histogram HSV",  computeHistogram(labColor) );
		Mat labels = computeClusters(ROI);
		resize(labels, labels, m_winSize);
		imshow("result", labels);			
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
	findContours(fgMaskMOG, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

	// Spravuj a filtruj kontury
	const int MIN_PIXELS_IN_CONTOUR = 15;// 
	const int MIN_AREA = 20; // 868 velksot hraca
	const int MAX_AREA = 5000;
	Rect BANNER_AREA(0, 0, 640, 18);
	const float VOLUME_BANNER = 0.8f;

	for( UINT i = 0; i < contours.size(); i++ ) { 
		vector<Point> kontura = contours[i];
		if( kontura.size() < MIN_PIXELS_IN_CONTOUR) { 
			if(m_debugDraw) drawContours( image, contours, i, Scalar(255,255,0), 1);
			continue;
		}
		RotatedRect rec = fitEllipse( Mat(kontura) );
		float takeSpace = rec.size.area();
		if(takeSpace < MIN_AREA || takeSpace > MAX_AREA) {
			if(m_debugDraw) drawContours( image, contours, i, Scalar(255,0,0), 1);
			continue;
		}

		if(isRelativeIntersection(kontura, BANNER_AREA, VOLUME_BANNER)) {
			if(m_debugDraw) drawContours( image, contours, i, Scalar(0,255,0), 1);
			continue;
		}

		objects.push_back(rec);
	}
}

void SoccerBall::Init() {
	m_record = new VideoRecord("data/filmrole5.avi");
	m_pMOG2 = new BackgroundSubtractorMOG2(200, 16.0, false);
	m_grass = new ThresholdColor(Scalar(35, 72, 50), Scalar(51, 142, 144));
	m_roi = new ThresholdColor(Scalar(35, 72, 50), Scalar(51, 142, 144));

	m_maxNumberOfPoints = 50;
	m_learning = true;
	m_mogLearnFrames = 200;
	m_winSize = Size(640, 480); 
	m_pause = false;
	m_actual = NULL;
	m_roi_index = 0;
	m_roiDraw = false;
	m_debugDraw = false;

	const char* windows[] = { 
		//"mogMask", 
		//"grassMask",
		//"finalMask", 
		"roiMask", 
		"Vystup" 
	};
	createWindows(windows);
	m_grass->createTrackBars("grassMask");
	m_roi->createTrackBars("roiMask");
}
/*
bool determineVisibility(FrameObject& object) {
	return !(object.type == ARTEFACT || object.type == BANNER);

}

Scalar determineColor(FrameObject& object) {
	if(type) {

	}
}
*/
void SoccerBall::opticalFlow(Mat& inputFrame, Mat& outputFrame) {
	if (m_mask.rows != inputFrame.rows || m_mask.cols != inputFrame.cols)
		m_mask.create(inputFrame.rows, inputFrame.cols, CV_8UC1);

	if (m_prevPts.size() > 0)
	{
		cv::calcOpticalFlowPyrLK(m_prevImg, m_nextImg, m_prevPts, m_nextPts, m_status, m_error);
	}

	m_mask = cv::Scalar(255);

	std::vector<cv::Point2f> trackedPts;

	for (size_t i=0; i<m_status.size(); i++)
	{
		if (m_status[i])
		{
			trackedPts.push_back(m_nextPts[i]);

			cv::circle(m_mask, m_prevPts[i], 15, cv::Scalar(0), -1);
			cv::line(outputFrame, m_prevPts[i], m_nextPts[i], cv::Scalar(0,250,0));
			cv::circle(outputFrame, m_nextPts[i], 3, cv::Scalar(0,250,0), -1);
		}
	}
	
	/*bool needDetectAdditionalPoints = trackedPts.size() < m_maxNumberOfPoints;
	if (needDetectAdditionalPoints)
	{
		Mat threshold, labColor;
		cvtColor(m_nextImg, labColor, CV_BGR2HSV);
		Scalar min = m_min;
		Scalar max = m_max;
		inRange(labColor, min, max, threshold);
		bitwise_not(threshold, m_mask);
		
		m_detector->detect(m_nextImg, m_nextKeypoints, m_mask);
		int pointsToDetect = m_maxNumberOfPoints -  trackedPts.size();

		if (m_nextKeypoints.size() > pointsToDetect)
		{
			std::random_shuffle(m_nextKeypoints.begin(), m_nextKeypoints.end());
			m_nextKeypoints.resize(pointsToDetect);
		}

		std::cout << "Detected additional " << m_nextKeypoints.size() << " points" << std::endl;

		for (size_t i=0; i<m_nextKeypoints.size(); i++)
		{
			trackedPts.push_back(m_nextKeypoints[i].pt);
			cv::circle(outputFrame, m_nextKeypoints[i].pt, 5, cv::Scalar(255,0,255), -1);
		}
	}*/
	
	m_prevPts = trackedPts;
	m_nextImg.copyTo(m_prevImg);
}

void SoccerBall::buildSoccerObjects(vector<RotatedRect>& imageObjects, vector<FrameObject>& realObjects) {
	const float VOLUME_BANNER = 0.8f;

	for( UINT a = 0; a < realObjects.size(); a++ ) { 
		for( UINT b = 0; b < imageObjects.size(); b++ ) { 
			// imageObjects[i]
			// realObjects.push_back(rec);
		}
	}
}

int SoccerBall::identifySoccerObjects(RotatedRect& rect) {
	/*const double MAX_DISTANCE = 100;

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
	*/
	return 0;
}
