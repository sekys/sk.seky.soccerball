#include "class.SoccerBall.hpp"

SoccerBall::SoccerBall()  {
	// Nacitaj vsetky casti SoccerBalla
	log = CREATE_LOG4CPP();
	if(log != NULL) {
		log->debug("Starting SoccerBall");
	}
}

SoccerBall::~SoccerBall() {
	SAFE_DELETE(record);
	if(log != NULL) log->debug("Ending SoccerBall");
}

int SoccerBall::getLockFPS() {
	return SoccerBall::m_lockFPS;
}

void SoccerBall::spracujJedenSnimok(Image* in) {
	// Uprav snimok podla potreby
	Mat out, image;
	image = in->data;
	resize(image, out, Size(640, 480));
	//cvtColor(out, out, CV_BGR2GRAY);
	//GaussianBlur(out, out, Size(7,7), 1.5, 1.5);
	//Canny(out, out, 0, 30, 3, false);
	//pMOG->operator()(out, fgMaskMOG);
	pMOG2->operator()(out, fgMaskMOG);

	vector<vector<Point> > contours;
	Mat bg, anotacie;
	//pMOG->getBackgroundImage(bg);
	pMOG2->getBackgroundImage(bg);
	erode(fgMaskMOG, fgMaskMOG, Mat(), Point(-1,-1), 2);
	dilate(fgMaskMOG, fgMaskMOG, Mat(), Point(-1,-1), 5);
	findContours(fgMaskMOG, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
	drawContours(out, contours, -1, Scalar(0,0,255), 2);

	// Ukaz vysledok
	imshow("Vstup", out);
	imshow("FG Mask MOG", fgMaskMOG);
	//imshow("Bg", bg);
}

void SoccerBall::Init() {
	record = new VideoRecord("data/filmrole5.avi");
	if(SoccerBall::m_fullscreen) {
		namedWindow("Vstup", WND_PROP_FULLSCREEN);
		cvSetWindowProperty("Vstup", CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);
	} else {
		namedWindow("FG Mask MOG");
		//namedWindow("Bg");
		namedWindow("Vstup");
		//resizeWindow("Vstup", 640, 480);
	}
	cv::moveWindow("Vstup", 0, 0);
	pMOG = new BackgroundSubtractorMOG(3, 8, 0.8);
	pMOG2 = new BackgroundSubtractorMOG2(3, 16.0, false);
	//record->readAll();
}

bool SoccerBall::Run() {
	// Tato metoda sa spusta v kazdom cykle apliakcie
	nacitajDalsiuSnimku();
	return true; // okno obnovujeme stale
}

void SoccerBall::nacitajDalsiuSnimku() {
	// Ziskaj snimok ..
	Image* image = NULL;
	try {
		image = record->readNext();
	} catch(VideoRecord::EndOfStream stream) {
		// Tu nastane 5sec delay ked je koniec streamu, dovod neznamy
		if(log != NULL) {
			log->debugStream() << "Restartujem stream.";
		}
		record->doReset();
		delete image;
		image = record->readNext();
	}
	if(log != NULL) {
		//log->debugStream() << "Snimok: " << image->pos_msec;
	}
	spracujJedenSnimok(image);
	image->data.release();
	delete image;
}