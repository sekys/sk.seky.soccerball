#include "class.SoccerBall.hpp"

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
	m_objects.clear();
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
	// Uprav vstup podla potreby
	Mat out, image, fgMaskMOG, ROI;
	Size velkostVstupu = m_winSize; //Size(1920, 1080);
	out = in->data.clone();
	resize(out, out, velkostVstupu);
	m_pMOG2->operator()(out, fgMaskMOG);
	ROI = Mat(out);

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

	vector<RotatedRect> ellipses;
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

		ellipses.push_back(rec);
	}

	// Spracuvaj detekovane oblasti
	UINT elSize = ellipses.size();
	for( UINT i = 0; i < elSize; i++ ) { 
		// Select ROI
		if(i == m_roi_index % elSize) {
			Rect rec = ellipses[i].boundingRect();
			rec = rec & Rect(0, 0, out.cols, out.rows);
			ROI = Mat(out, rec).clone(); 
			histogram(ROI);
		}
		ellipse(out, ellipses[i], Scalar(0,0,255), 1);
	}

	/*if(in->pos_msec == 4) {
	// Nastala chyba
	int debug = 1;
	debug++;
	}
	*/

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

	resize(ROI, ROI, m_winSize);
	resize(out, out, m_winSize);
	resize(fgMaskMOG, fgMaskMOG, m_winSize);
	imshow("Vybrana sekcia", ROI);
	imshow("Vystup", out);
	imshow("Maska", fgMaskMOG);
}

void SoccerBall::histogram(Mat src) {
	vector<cv::Mat> imgRGB;
	split(src, imgRGB);
	int k = 4; // dres ma 2 casti, biela ciara, trava
	int n = src.rows *src.cols;
	Mat img3xN(n,3, CV_8U);
	for(int i=0; i!=3; ++i) {  
		imgRGB[i].reshape(1,n).copyTo(img3xN.col(i));
	}

	img3xN.convertTo(img3xN, CV_32F);
	Mat bestLables;
	kmeans(img3xN, k, bestLables, cv::TermCriteria(), 10, KMEANS_RANDOM_CENTERS );
	bestLables = bestLables.reshape(0, src.rows);
	convertScaleAbs(bestLables, bestLables, int(255/k));
	resize(bestLables, bestLables, m_winSize);
	imshow("result", bestLables);	

	/// Separate the image in 3 places ( B, G and R )
	/*vector<Mat> bgr_planes;
	split( src, bgr_planes );

	/// Establish the number of bins
	int histSize = 256;

	/// Set the ranges ( for B,G,R) )
	float range[] = { 0, 256 } ;
	const float* histRange = { range };

	bool uniform = true; 
	bool accumulate = false;

	Mat b_hist, g_hist, r_hist;

	/// Compute the histograms:
	calcHist( &bgr_planes[0], 1, 0, Mat(), b_hist, 1, &histSize, &histRange, uniform, accumulate );
	calcHist( &bgr_planes[1], 1, 0, Mat(), g_hist, 1, &histSize, &histRange, uniform, accumulate );
	calcHist( &bgr_planes[2], 1, 0, Mat(), r_hist, 1, &histSize, &histRange, uniform, accumulate );

	// Draw the histograms for B, G and R
	int hist_w = 512; 
	int hist_h = 400;
	int bin_w = cvRound( (double) hist_w/histSize );

	Mat histImage( hist_h, hist_w, CV_8UC3, Scalar( 0,0,0) );

	/// Normalize the result to [ 0, histImage.rows ]
	normalize(b_hist, b_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat() );
	normalize(g_hist, g_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat() );
	normalize(r_hist, r_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat() );

	/// Draw for each channel
	for( int i = 1; i < histSize; i++ )
	{
		line( histImage, Point( bin_w*(i-1), hist_h - cvRound(b_hist.at<float>(i-1)) ) ,
			Point( bin_w*(i), hist_h - cvRound(b_hist.at<float>(i)) ),
			Scalar( 255, 0, 0), 2, 8, 0  );
		line( histImage, Point( bin_w*(i-1), hist_h - cvRound(g_hist.at<float>(i-1)) ) ,
			Point( bin_w*(i), hist_h - cvRound(g_hist.at<float>(i)) ),
			Scalar( 0, 255, 0), 2, 8, 0  );
		line( histImage, Point( bin_w*(i-1), hist_h - cvRound(r_hist.at<float>(i-1)) ) ,
			Point( bin_w*(i), hist_h - cvRound(r_hist.at<float>(i)) ),
			Scalar( 0, 0, 255), 2, 8, 0  );
	}

	/// Display
	imshow("Histogram", histImage );*/
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

	const char* const windows[] = { "Vystup", "Maska", "Vybrana sekcia", "Histogram", "result" };
	const size_t len = sizeof(windows) / sizeof(windows[0]);
	for (size_t i = 0; i < len; ++i) {
		const char* meno = windows[i];
		namedWindow(meno);
		resizeWindow(meno, 640, 480);
		moveWindow(meno, 0, 0);
	}
}