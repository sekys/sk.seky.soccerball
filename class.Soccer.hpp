#pragma once
#include "class.App.hpp"
#include "log4cpp.h"
#include "entities.hpp"
#include "class.VideoRecord.hpp"
#include <opencv2/video/background_segm.hpp>
#include "class.ThresholdColor.hpp"
#include "class.ObjectDetector.hpp"
#include "class.Drawer.hpp"

class Soccer : public App
{
private:
	// Atributy aplikacie
	log4cpp::Category* log;
	Size m_winSize;
	void commandArrive(string& str);

	// Spracovanie videa
	bool m_pause;
	VideoRecord* m_record;
	Frame* m_actual;
	void processFrame(Frame* image); 
	void loadNextFrame(); 

	// Mog atributy
	Ptr<BackgroundSubtractor> m_pMOG2; //MOG2 Background subtractor
	int m_mogLearnFrames;
	bool m_learning;

	// Determine objects
	ObjectDetector* m_detector;
	Drawer* m_drawer;
	void processImage(Mat& input);
	//vector<FrameObject> m_realObjects;
	ThresholdColor* m_grass;
	
	// Flow
	int m_maxNumberOfPoints;
	void opticalFlow(Mat& inputFrame, Mat& outputFrame);
	Mat m_prevImg;
	Mat m_nextImg;
	Mat m_mask;
	vector<unsigned char> m_status;
	vector<float>         m_error;
	vector<cv::Point2f>  m_prevPts;
	vector<cv::Point2f>  m_nextPts;

protected:
	// Inicializacia programu
	virtual void Init();

	// Run() metoda je vola v cykle dokedy bezi apliakcia
	virtual bool Run();
	virtual int getLockFPS();

public:

	Soccer();
	~Soccer();
};
