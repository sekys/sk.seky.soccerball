#pragma once
#include "class.App.hpp"
#include "log4cpp.h"
#include "entities.hpp"
#include "class.VideoRecord.hpp"
#include <opencv2/video/background_segm.hpp>
#include "class.ThresholdColor.hpp"

/**
* Trieda reprezentuje hlavny projekt Carlos a jeho halvny algoritmus, ktory spaja ostatne moduly
*/
class SoccerBall : public App
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
	void processImage(Mat& input);
	vector<FrameObject> m_realObjects;
	ThresholdColor* m_grass;
	ThresholdColor* m_roi;
	int m_roi_index;
	bool m_roiDraw;
	bool m_debugDraw;

	void findObjects(Mat& image, Mat& fbMask, vector<RotatedRect>& objects);
	void buildSoccerObjects(vector<RotatedRect>& imageObjects, vector<FrameObject>& realObjects);
	void drawROI(Mat& image, Mat& mask, vector<RotatedRect>& objects);
	int identifySoccerObjects(RotatedRect& object);
	
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
	// Inicializacia Carlosu
	virtual void Init();

	// Run() metoda je vola v cykle dokedy bezi apliakcia
	virtual bool Run();
	virtual int getLockFPS();

public:

	SoccerBall();
	~SoccerBall();
};
