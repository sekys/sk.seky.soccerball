#include <iostream>
#include <fstream>
#include <log4cpp/Category.hh>
#include <log4cpp/PropertyConfigurator.hh>
#include "class.Soccer.hpp"

/**
* Metoda za pomoci ktorej sa spsuti program.
*/

int main()
{
	// Nastav logy
	// ShowWindow( GetConsoleWindow(), SW_HIDE );
	std::string initFileName = "data/log4cpp.properties";
	log4cpp::PropertyConfigurator::configure(initFileName);
	log4cpp::Category::getRoot().debug("Starting log4cpp, configuration %s", initFileName.c_str());

	// Appku postupne inicializujeme, spustime, deinicializujeme
	Soccer* app = new Soccer();
	app->start();
	delete app;

	return 0;
}
