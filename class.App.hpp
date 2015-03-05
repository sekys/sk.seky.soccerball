#pragma once
#include <windows.h>
#include "log4cpp.h"

/**
* Trieda reprezentuju aplikaciu v najvyssej podobe.
* teda tato trieda sa spusta v maine ako prva a predstavuje
* urcitu komunikaciu s prostredim, teda operacnym systemom.
*/
class App {
protected:
	log4cpp::Category* blog;
	bool m_runnig; 

	virtual int getLockFPS() = 0;
	virtual void Init() = 0;
	virtual bool Run()  = 0;
	virtual void LimitFPS(double delta);
	virtual void Refresh()  {}
	virtual void MainCycle();
public:
	// Prvotna inicializacia
	App();

	// Spusti aplikaciu po celkovej inicializacii 
	virtual void start();
	// Moze bezat nadalej aplikacia, nenastal problem ?
	bool canRun(); 

	// Prikaz na zastavenie behu aplikacie
	void stop();
};