/*
 * cartesian.h
 *
 * Created: 26.1.2015 11:39:33
 *  Author: Kremmen
 
 * A small class for rendering cartesian x-y coordinate backgrounds and plotting arbitrary diagrams on same.
 * originally designed to display the temperature sequences of Hacklab's SMD soldering oven mod.
 
 */ 


#ifndef CARTESIAN_H_
#define CARTESIAN_H_

#include <ILI9341_due.h>
#include <ILI9341_due_gText.h>
#include "allFonts.h"



#define MAXAXES 3
#define AX_VASEN 0
#define AX_ALA 1
#define AX_OIKEA 2

#define MAJORTICKWIDTH 5

#define MAXPLOTS 20
#define CAPTIONLENGTH 20


class PGM_TextArea: public ILI9341_due_gText{
public:
	PGM_TextArea(ILI9341_due *ili, int16_t X1, int16_t Y1, int16_t X2, int16_t Y2, uint16_t but_color, uint16_t txt_color, PGM_P txt[] );
	void show();
private:
	PGM_P *text;
};

class TextArea: public ILI9341_due_gText{
	public:
	TextArea(ILI9341_due *ili, int16_t X1, int16_t Y1, int16_t X2, int16_t Y2, uint16_t but_color, uint16_t txt_color, char *txt[] );
	void show();
	private:
	char **text;
};

class Caption: public ILI9341_due_gText{
public:
	Caption( ILI9341_due *ili, int16_t X1, int16_t Y1, int16_t X2, int16_t Y2, uint16_t but_color, uint16_t txt_color, char *txt );
	void show();
	void appendText( char *txt );
	void appendChar( char ch );
	void backSpace();
	void clear();
private:
	char text[CAPTIONLENGTH];
};

class Button:public ILI9341_due_gText{
public:
	Button( ILI9341_due *ili, int16_t X1, int16_t Y1, int16_t X2, int16_t Y2, uint16_t but_color, uint16_t txt_color, const char *cap);
	void show();
	
private:
	char caption[20];
};
	
struct axisDefinition {
	boolean defined;
	float axisMax, axisMin;			// [0]=vasen, [1]=ala, [2]=oikea
	float majorTick, minorTick;
	boolean majorLabelsOn, minorLabelsOn;
	boolean majorTicksOn, minorTicksOn;
	boolean majorGridOn, minorGridOn;
};

// auxiliary class for an individual plot. This will be drawn on the plotarea using the associated X and Y coordinate axes
class plot {
public:

private:

};

class cartesian {
	friend class plot;
public:
	cartesian( ILI9341_due *display );
	void begin( uint16_t X, uint16_t Y, uint16_t W, uint16_t H, uint16_t color );
	void setAxis(uint8_t index, float Min, float Max, float majorTick, float minorTick);
	void showTicks(uint8_t index, boolean majors, boolean minors);
	void show();
	int16_t getPXPos(uint8_t index, float val);
	void drawLine(float x0, float y0, float x1, float y1);
	
protected:

private:
	ILI9341_due *disp;
	uint16_t c_X, c_Y, c_W, c_H, c_color;	// koordinaatiston ala
	uint16_t p_X, p_Y, p_W, p_H; // kuvaajien piirtoala
	uint8_t plotCount;
	plot *Pt[MAXPLOTS];
	axisDefinition axis[3];
	
	void drawAxis(uint8_t index);
	void drawMajorTicks(uint8_t index);
	void drawMajorLabels(uint8_t index);
	void render();
	
};



#endif /* CARTESIAN_H_ */