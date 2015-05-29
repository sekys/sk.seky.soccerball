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
	SAFE_DELETE(m_tracer);
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
	resize(m_actual->data, m_actual->data, WIN_SIZE);

	// Learning MOG algorithm
	if(m_learning) {
		if(in->pos_msec < m_mogLearnFrames) {
			Mat mask;
			m_pMOG2->operator()(in->data, mask, 1.0 / m_mogLearnFrames);
			return;
		}

		if(in->pos_msec == m_mogLearnFrames) {
			learningEnd();
			return;
		}
		return;
	} 
	
	// Pauza pre konkretny snimok
	if(in->pos_msec == 355) {
		m_pause = true;
	}

	processImage(m_actual->data.clone());
}

void Soccer::learningEnd() {
	log->debugStream() << "Restartujem stream.";
	m_record->doReset();
	m_learning = false;
	log->debugStream() << "Koniec ucenia.";

	m_detector = new ObjectDetector();
	m_drawer = new Drawer();
	m_tracer = new ObjectTracer();
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
	
	// Najdi objekty
	vector<FrameObject*> objects;
	m_detector->findObjects(input, finalMask, objects);
	m_tracer->process(input, objects);
	m_drawer->draw(input, finalMask, objects);
}


void Soccer::Init() {
	m_record = new VideoRecord("data/filmrole5.avi");
	m_pMOG2 = new BackgroundSubtractorMOG2(200, 16.0, false);
	m_grass = new ThresholdColor(Scalar(35, 72, 50), Scalar(51, 142, 144));
	m_learning = true;
	m_mogLearnFrames = 200;
	m_pause = false;
	m_actual = NULL;

	const char* windows[] = { 
		//"mogMask", 
		//"grassMask",
		//"Color mask", 
		//"Roi", 
		"Vystup" 
	};
	//createWindows(windows);
	m_grass->createTrackBars("grassMask");
}

Size FrameObject::WIN_SIZE = Size(640, 480);  //Size(1920, 1080);
Size Soccer::WIN_SIZE = Size(640, 480); 
Size Drawer::WIN_SIZE = Size(640, 480); 

// TODO: - skupina, ked sa dotykaju rukou tak pouzijem opening a zistim, ci tam nebude torso hraca 2x, 3x
// TODO: - torso zistim extra eroziou, kde ruky a hlavu odstranim
// TODO: - pomocou trajektorie hraca zistim, kolko hracov tam je
// TODO: - lopta sa riesi druhym tokom, potom hladam cez hugh circle o urcitej velkosti a biele pozadie motion gradient

