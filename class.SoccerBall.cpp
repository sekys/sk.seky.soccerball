#include "class.SoccerBall.hpp"

SoccerBall::SoccerBall() : m_lockFPS(60), m_winSize(640, 480) 
{
	// Nacitaj vsetky casti SoccerBalla
	log = CREATE_LOG4CPP();
	if(log != NULL) {
		log->debug("Starting SoccerBall");
	}
	m_pause = false;
}

SoccerBall::~SoccerBall() {
	SAFE_DELETE(m_record);
	SAFE_DELETE(m_pMOG2);
	m_objects.clear();
	if(log != NULL) log->debug("Ending SoccerBall");
}

int SoccerBall::getLockFPS() {
	return m_lockFPS;
}

bool SoccerBall::Run() {
	int key = cv::waitKey(30); // caka najmenej 30ms, zalezi od win procesov
	string str;  
	str =(char) key;
	if(str == "s") {
		m_pause = !m_pause;
	}
	if(!m_pause) {
		nacitajDalsiuSnimku();
	}
	return true; // okno obnovujeme stale
}

void SoccerBall::nacitajDalsiuSnimku() {
	// Ziskaj snimok ..
	Image* image = NULL;
	try {
		image = m_record->readNext();
	} catch(VideoRecord::EndOfStream stream) {
		// Tu nastane 5sec delay ked je koniec streamu, dovod neznamy
		if(log != NULL) {
			log->debugStream() << "Restartujem stream.";
		}
		m_record->doReset();
		delete image;
		image = m_record->readNext();
	}
	if(log != NULL) {
		log->debugStream() << "Snimok: " << image->pos_msec;
	}
	spracujJedenSnimok(image);
	image->data.release();
	delete image;
}

// ----------------------------------------------------
// ----------------------------------------------------
// ----------------------------------------------------


void SoccerBall::spracujJedenSnimok(Image* in) {
	// Uprav vstup podla potreby
	Mat out, image, fgMaskMOG;
	Size velkostVstupu = m_winSize; //Size(1920, 1080);
	out = in->data;
	resize(out, out, velkostVstupu);
	m_pMOG2->operator()(out, fgMaskMOG);

	// Vyber kontury
	vector<vector<Point>> contours;
	erode(fgMaskMOG, fgMaskMOG, Mat(), Point(-1,-1), 1);
	dilate(fgMaskMOG, fgMaskMOG, Mat(), Point(-1,-1), 4);
	findContours(fgMaskMOG, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

	// Spravuj kontury
	const int MIN_PIXELS_IN_CONTOUR = 36;// 
	const int MIN_AREA = 100; // 868 velksot hraca
	const int MAX_AREA = 5000;
	Rect BANNER_AREA(0, 0, 640, 36);
	const float VOLUME_BANNER = 0.8f;
	const bool debugDraw = false;
	//vector<RotatedRect> ellipses( contours.size() );
	for( UINT i = 0; i < contours.size(); i++ ) { 
		vector<Point> kontura = contours[i];
		if( kontura.size() < MIN_PIXELS_IN_CONTOUR) { 
			if(debugDraw) drawContours( out, contours, i, Scalar(255,0,0), 1);
			continue;
		}
		RotatedRect rec = fitEllipse( Mat(kontura) ); 
		float takeSpace = rec.size.area();
		if(takeSpace < MIN_AREA || takeSpace > MAX_AREA) {
			if(debugDraw) drawContours( out, contours, i, Scalar(255,0,0), 1);
			continue;
		}

		int podobnost = intersection(kontura, BANNER_AREA);
		if(podobnost > ((int) kontura.size() * VOLUME_BANNER)) {
			if(debugDraw) drawContours( out, contours, i, Scalar(0,255,0), 1);
			continue;
		}

		//ellipses[i] = rec;
		ellipse(out, rec, Scalar(0,0,255), 1);
		//drawContours( out, contours, i, color, 1);
	}

	// Elipsi
	/*
	zoznam objektov
	pozeraj sa, ze ci ci existuje elipsa blizka v zozname MAX_SPEED
	ak existuje , pouzi jej referenciu .. sprav zoznam pohybov cez linkedlist
	potom k neexistuje pridaj ju do zoznamu a vygeneruj nove ID pre nu
	sleduj, pre vsetky elipsi ci sa hybu, ked posledne 3000 snimkov nespravila pohyb, oznac ju ako staticky objekt
	Ak elipsa vznikla, je mensia ako treshold a dalsiu snimku nema nasledovaca (zmizla) tak ju znac ako artefakt.
	Vsetky elipsi, ktore maju nad Y hodnotu tak ju zneviditelni. (Druha kamera by ho zachytila).
	Ak je elipsa priliz velka a dlho staticka, banner, tak ju schovaj tiez.
	Renderuj aktualne elipsi, objekty. Stare nie. ( v debugu ano).
	OBSERVER pattern

	nastavoval som velkosti
	odstranil so martefakty
	definoval si motion tracker
	prezentaciu mam

	objekt sa moze hybat, ked rpekona MIN_THRES a ked sa uz nehybe ale v minulosti sa hybal tak to asi bude hrac
	*/

	resize(out, out, m_winSize);
	resize(fgMaskMOG, fgMaskMOG, m_winSize);
	imshow("Vystup", out);
	imshow("Maska", fgMaskMOG);
}

// Vypocitaj v kolkych bodoch dochadza k prieniku
int SoccerBall::intersection(vector<Point>& contour, Rect& rec) {
	int pocet = 0;
	for( UINT i = 0; i < contour.size(); i++ ) { 
		if (rec.contains(contour[i])) {
			pocet++;
		}
	}
	return pocet;
}

void SoccerBall::Init() {
	m_record = new VideoRecord("data/filmrole5.avi");
	m_pMOG2 = new BackgroundSubtractorMOG2(3, 16.0, false);

	const char* const windows[] = { "Vystup", "Maska" };
	const size_t len = sizeof(windows) / sizeof(windows[0]);
	for (size_t i = 0; i < len; ++i) {
		const char* meno = windows[i];
		namedWindow(meno);
		resizeWindow(meno, 640, 480);
		moveWindow(meno, 0, 0);
	}
}