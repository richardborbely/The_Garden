#include "postprocess.h"

// Create PostProcess object and set default values
PostProcess::PostProcess() { setDefaultValues(); }

// Get object values
unsigned int PostProcess::getActiveMenuItem() { return activeMenuItemID; }
float PostProcess::getBrightness() { return brightness; }
float PostProcess::getSaturation() { return saturation; }
float PostProcess::getChromaticAbberation() { return chromaticAbberation; }
float PostProcess::getSepia() { return sepia; }
float PostProcess::getFilmgrain() { return filmgrain; }

// Set object values
void PostProcess::setActiveMenuItem(int menuItemID) { activeMenuItemID = menuItemID; }
void PostProcess::setBrightness(float value) { brightness = value; }
void PostProcess::setSaturation(float value) { saturation = value; }
void PostProcess::setChromaticAbberation(float value) { chromaticAbberation = value; }
void PostProcess::setSepia(float value) { sepia = value; }
void PostProcess::setFilmgrain(float value) { filmgrain = value; }
void PostProcess::setDefaultValues() 
{// Reset values to default
	activeMenuItemID = 0;
	brightness = 0.0f;
	saturation = 1.0f;
	chromaticAbberation = 0.000f;
	sepia = 0.0f;
	filmgrain = 0.0f;
}