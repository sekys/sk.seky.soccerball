#pragma once
#include "class.App.hpp"
#include "log4cpp.h"
#include "entities.hpp"
#include "class.VideoRecord.hpp"
#include <opencv2/video/background_segm.hpp>

/**
* Trieda reprezentuje hlavny projekt Carlos a jeho halvny algoritmus, ktory spaja ostatne moduly
*/
class SoccerBall : public App
{
private:
	log4cpp::Category* log;
	const int m_lockFPS;
	const Size m_winSize;
	
	bool m_pause;
	VideoRecord* m_record;
	Ptr<BackgroundSubtractor> m_pMOG2; //MOG2 Background subtractor
	vector<SoccerObject> m_objects;
	Image* m_actual;
	int m_roi_index;

	void histogram(Mat src);
	void spracujJedenSnimok(Image* image); 	// Metoda na spracovanie snimku
	void nacitajDalsiuSnimku(); 	// Metoda pre ziskanie snimku
	int intersection(vector<Point>& contour, Rect& rec);

protected:
	// Inicializacia Carlosu
	virtual void Init();

	// Run() metoda je vola v cykle dokedy bezi apliakcia
	virtual bool Run();
	virtual int getLockFPS();

public:

	SoccerBall();
	~SoccerBall();
};
