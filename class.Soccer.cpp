#include "class.Soccer.hpp"
#include <opencv2/imgproc/imgproc.hpp>
#include "util.h"

Soccer::Soccer() 
{
	// Nacitaj vsetky casti Soccera
	log = CREATE_LOG4CPP();
	if(log != NULL) {
		log->debug("Starting Soccer");
	}
}

Soccer::~Soccer() {
	SAFE_DELETE(m_record);
	SAFE_DELETE(m_pMOG2);
	SAFE_DELETE(m_detector);
	SAFE_DELETE(m_drawer);
	SAFE_DELETE(m_grass);
	SAFE_RELEASE(m_actual);
	//m_realObjects.clear();
	if(log != NULL) log->debug("Ending Soccer");
}

int Soccer::getLockFPS() {
	return 60;
}

bool Soccer::Run() {
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

void Soccer::commandArrive(string& cmd) {
	// Pauza
	if(cmd == "s") {
		m_pause = !m_pause;
		log->debugStream() << "m_pause " << m_pause;
		return;
	}
	if(cmd == "d") {		
		m_drawer->switchDebugDraw();
		return;
	}
	if(cmd == "f") {
		m_drawer->switchDrawType();
		return;
	}
	if(cmd == "c") {
		m_drawer->switchTeamColoring();
		return;
	}

	// ROI oblast
	if(cmd == "w") {		
		m_drawer->switchROIDraw();
		return;
	}
	if(cmd == "e") {		
		m_drawer->nextROI();
		return;
	}
	if(cmd == "q") {
		m_drawer->previousROI();
		return;
	}
}

void Soccer::loadNextFrame() {
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

void Soccer::processFrame(Frame* in) {
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

void Soccer::processImage(Mat& input) {
	// Ziskaj masku pohybu cez MOG algoritmus
	Mat mogMask;
	m_pMOG2->operator()(input, mogMask);

	// Opening
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
	vector<FrameObject*> objects;
	m_detector->findObjects(input, finalMask, objects);
	Mat out;
	opticalFlow(input, out, objects);
	m_drawer->draw(input, finalMask, objects);
}


void Soccer::Init() {
	m_record = new VideoRecord("data/filmrole5.avi");
	m_pMOG2 = new BackgroundSubtractorMOG2(200, 16.0, false);
	m_grass = new ThresholdColor(Scalar(35, 72, 50), Scalar(51, 142, 144));
	m_detector = new ObjectDetector();
	m_drawer = new Drawer();
	m_maxNumberOfPoints = 50;
	m_learning = true;
	m_mogLearnFrames = 200;
	m_winSize = Size(640, 480); 
	m_pause = false;
	m_actual = NULL;

	const char* windows[] = { 
		//"mogMask", 
		//"grassMask",
		"Color mask", 
		"Roi", 
		"Vystup" 
	};
	//createWindows(windows);
	m_grass->createTrackBars("grassMask");
	//m_detector2 = cv::FeatureDetector::create("GridFAST");
}

// http://opencv-python-tutroals.readthedocs.org/en/latest/py_tutorials/py_video/py_lucas_kanade/py_lucas_kanade.html
void Soccer::opticalFlow(Mat& inputFrame, Mat& outputFrame, vector<FrameObject*>& objs) {
	inputFrame.copyTo(outputFrame); 
	cvtColor(inputFrame, m_nextImg, cv::COLOR_BGR2GRAY);

	if (m_mask.rows != inputFrame.rows || m_mask.cols != inputFrame.cols)
		m_mask.create(inputFrame.rows, inputFrame.cols, CV_8UC1); // maska je vytvorena len jeden krat
	if (m_prevPts.size() > 0) {
		// http://docs.opencv.org/modules/video/doc/motion_analysis_and_object_tracking.html
		cv::calcOpticalFlowPyrLK(m_prevImg, m_nextImg, m_prevPts, m_nextPts, m_status, m_error); //body pre jednu crtu?
		// algoritmus mi vlastne povie, ze tieto body X,Y kam sa presunuli
		// lenz eja potrebujem vediet, ze hrac ID(X, Y) je tam a tam
	}

	// VYkresli kruhy a ciary do vysledneho obrazka a v maske zaznaci oblast, kde sa mozu vyskytovat podobny objekt 
	m_mask = cv::Scalar(255); // maska je vzdy plna
	std::vector<cv::Point2f> trackedPts;
	for (size_t i=0; i<m_status.size(); i++) {
		if (m_status[i]) {
			trackedPts.push_back(m_nextPts[i]);
			cv::circle(m_mask, m_prevPts[i], 15, cv::Scalar(0), -1);
			cv::line(outputFrame, m_prevPts[i], m_nextPts[i], cv::Scalar(0,250,0));
			cv::circle(outputFrame, m_nextPts[i], 3, cv::Scalar(0,250,0), -1);
		}
	}
	
	// Ked sa cast keypointov stratila
	bool needDetectAdditionalPoints = trackedPts.size() < m_maxNumberOfPoints;
	if (needDetectAdditionalPoints)
	{		
		// obraz zachytava le cast ihriska a tak sa moze stat, ze skupina hracov pride uz do sceny
		imshow("Flow mask1", m_mask); 
		
		// ja by som skor potreboval vediet, ze maska ROI a druha maska jak su si podobne a vektor mozneho smeru
		vector<cv::Point2f> m_nextKeypoints;
		bool detected = false;
		for( UINT i = 0; i < objs.size(); i++ ) { 
			FrameObject* obj = objs.at(i);
			if(obj->type != PLAYER_A) {
				continue;
			}

			// skopiruj klucove body z kontury
			for (size_t i=0; i<obj->m_countour.size(); i++) {
				m_nextKeypoints.push_back(obj->m_countour[i]);
			}
			detected = true;
			break;
		}

		if(!detected) {
			return; // nemame ani jednu ROI oblast
		}
		// Vyhladaj nove keypointy, vsetky alebo cast z nich na zaklade masky
		//m_detector2->detect(m_nextImg, m_nextKeypoints, m_mask);
		
		// nahodne vyber cast z nich a pridaj ich ze ich sledujeme
		int pointsToDetect = m_maxNumberOfPoints -  trackedPts.size();
		if (m_nextKeypoints.size() > pointsToDetect) {
			std::random_shuffle(m_nextKeypoints.begin(), m_nextKeypoints.end());
			m_nextKeypoints.resize(pointsToDetect);
		}

		// Nove keypointy uloz a zaznac do obrazku
		std::cout << "Detected additional " << m_nextKeypoints.size() << " points" << std::endl;
		for (size_t i=0; i<m_nextKeypoints.size(); i++) {
			trackedPts.push_back(m_nextKeypoints[i]);
			cv::circle(outputFrame, m_nextKeypoints[i], 5, cv::Scalar(255,0,255), -1);
		}
	}
	
	imshow("Flow mask2", m_mask); 
	imshow("Flow m_prevImg", m_prevImg); 
	imshow("Flow m_nextImg", m_nextImg); 
	m_prevPts = trackedPts;
	m_nextImg.copyTo(m_prevImg);
}

