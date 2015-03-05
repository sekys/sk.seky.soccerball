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
	// Program konci, je potrebne uvolnit jeho casti
	if(log != NULL) log->debug("Ending SoccerBall");
}

int SoccerBall::getLockFPS() {
	return SoccerBall::m_lockFPS;
}

void SoccerBall::spracujJedenSnimok(Image in) {
	// Uprav snimok podla potreby
	Mat out, image;
	image = in.data;
	resize(image, out, Size(640, 480));
	//cvtColor(out, out, CV_BGR2GRAY);
    //GaussianBlur(out, out, Size(7,7), 1.5, 1.5);
    //Canny(out, out, 0, 30, 3, false);
	pMOG->operator()(out, fgMaskMOG);
    pMOG2->operator()(out, fgMaskMOG2);

	// Ukaz vysledok
	imshow("Vstup", out);
	imshow("FG Mask MOG", fgMaskMOG);
    imshow("FG Mask MOG 2", fgMaskMOG2);
	in.data.release();
}

void SoccerBall::Init() {
	record = new VideoRecord("data/filmrole5.avi");
	if(SoccerBall::m_fullscreen) {
		namedWindow("Vstup", WND_PROP_FULLSCREEN);
		cvSetWindowProperty("Vstup", CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);
	} else {
		namedWindow("FG Mask MOG");
		namedWindow("FG Mask MOG 2");
		namedWindow("Vstup");
		//resizeWindow("Vstup", 640, 480);
	}
	cv::moveWindow("Vstup", 0, 0);
	
	//create Background Subtractor objects
	pMOG = new BackgroundSubtractorMOG(); //MOG approach
	pMOG2 = new BackgroundSubtractorMOG2(); //MOG2 approach
}

bool SoccerBall::Run() {
	// Tato metoda sa spusta v kazdom cykle apliakcie
	nacitajDalsiuSnimku();
	return true; // okno obnovujeme stale
}

void SoccerBall::nacitajDalsiuSnimku() {
	// Ziskaj snimok ..
	Image image;
	try {
		try {
			image = record->readNext();
		} catch(VideoRecord::EndOfStream stream) {
			// Tu nastane 5sec delay ked je koniec streamu, dovod neznamy
			if(log != NULL) {
				log->debugStream() << "Restartujem stream.";
			}
			record->doReset();
			image = record->readNext();
		}
		if(log != NULL) {
			log->debugStream() << "Snimok: " << image.pos_msec;
		}
		spracujJedenSnimok(image);
	} catch(VideoRecord::EndOfStream stream) {
		// Cyklus Run skonci, skonci apliakcia, spusti sa dekonstruktor, zacne sa uvolnovat pamet a vsetko vypinat ...
		if(log != NULL) {
			log->debugStream() << "Koniec streamu";
		}
		this->stop();
	} catch (std::out_of_range e) {
		if(log != NULL) {
			log->debugStream() << e.what();
		}
	} 
}