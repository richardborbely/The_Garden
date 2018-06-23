#pragma once

// Post Process object that stores effect values and active effect ID
class PostProcess
{
	unsigned int activeMenuItemID;
	float brightness;
	float saturation;
	float chromaticAbberation;
	float sepia;
	float filmgrain;

public:
	PostProcess();

	unsigned int getActiveMenuItem();
	float getBrightness();
	float getSaturation();
	float getChromaticAbberation();
	float getSepia();
	float getFilmgrain();

	void setActiveMenuItem(int menuItemID);
	void setBrightness(float value);
	void setSaturation(float value);
	void setChromaticAbberation(float value);
	void setSepia(float value);
	void setFilmgrain(float value);
	void setDefaultValues();
};
