/** @file class.App.cpp
*
* Implementacia triedy App.
*/
#include "class.App.hpp"
#include "class.Time.hpp"
#include <iostream>

void App::MainCycle() {
	// Toto je hlavny cyklus aplikacie.
	// V nom sa udrzuje beh apliakcie v cykle.
	// Po kazdom cykle prebieha preposielanie sprav z oper. systemom.
	MSG msg;
	ZeroMemory( &msg, sizeof(msg) );
	while(canRun()) {
		if( PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE ) ) { // prekladame spravy
			TranslateMessage( &msg );  // prekladame spravy
			DispatchMessage( &msg ); // uvolnujeme
		} else {
			double time = Time::getInstance().GetAbsolute();
			bool refreshApp = Run();
			double deltaTime = time - Time::getInstance().GetAbsolute();;
			if(refreshApp) {
				// nemozme priamo volat UpdateWindow ale toto mozme ..je take iste
				Refresh();
			}
			LimitFPS(deltaTime);
		}
	}
}

void App::LimitFPS(double delta) {
	double sleepTime = ( (1.0 / getLockFPS()) - delta) * 1000.0f;
	/*if(blog != NULL) {
	blog->debugStream() << "delta " << delta << "\n";
	}*/
	if(sleepTime < 0) {
		// Ak vypocet snimku klesol pod nase FPS
		Sleep(0); // pomahame windowsu sa vyrovnat s hrou
	} else {
		/*if(blog != NULL) {
			blog->debugStream() << "sleep " << sleepTime << "\n";
		}*/
		Sleep((DWORD) sleepTime);
	}
}

App::App() {
	blog = CREATE_LOG4CPP();
}

void App::start() {
	Init();
	m_runnig = true;
	MainCycle();
}

bool App::canRun() {
	return m_runnig;
}
void App::stop() {
	m_runnig = false;
}