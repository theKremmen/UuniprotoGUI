/*
 * cartesian.cpp
 *
 * Created: 26.1.2015 11:39:48
 *  Author: Kremmen
 */
#include "cartesian.h"

PGM_TextArea::PGM_TextArea(ILI9341_due *ili, int16_t X1, int16_t Y1, int16_t X2, int16_t Y2, uint16_t but_color, uint16_t txt_color, PGM_P txt[]):ILI9341_due_gText(ili,X1,Y1,X2,Y2) {
	text = txt;
	_fontBgColor = but_color;
	_fontColor = txt_color;
}


void PGM_TextArea::show() {
uint16_t cnt,t_X,t_Y,fH;
	clearArea(_fontBgColor);
	selectFont( calibri );
	fH = fontHeight();
	setFontColor( _fontColor, _fontBgColor);
	cnt = 0;
	t_X = fH/2; t_Y = fH/2;
	while (text[cnt]) {
		drawString_P( text[cnt], t_X, t_Y);
		t_Y += fH;
		cnt++;	
	}
}

TextArea::TextArea(ILI9341_due *ili, int16_t X1, int16_t Y1, int16_t X2, int16_t Y2, uint16_t but_color, uint16_t txt_color, char *txt[]):ILI9341_due_gText(ili,X1,Y1,X2,Y2) {
	text = txt;
	_fontBgColor = but_color;
	_fontColor = txt_color;
}


void TextArea::show() {
uint16_t cnt,t_X,t_Y,fH;
	clearArea(_fontBgColor);
	selectFont( calibri );
	fH = fontHeight();
	setFontColor( _fontColor, _fontBgColor);
	cnt = 0;
	t_X = fH/2; t_Y = fH/2;
	while (text[cnt]) {
		drawString( text[cnt], t_X, t_Y);
		t_Y += fH;
		cnt++;
	}
}
Caption::Caption( ILI9341_due *ili, int16_t X1, int16_t Y1, int16_t X2, int16_t Y2, uint16_t but_color, uint16_t txt_color, char *txt ):ILI9341_due_gText(ili,X1,Y1,X2,Y2) {
char *textp;
uint8_t cnt;
	_fontBgColor = but_color;
	_fontColor = txt_color;
	
	textp = &text[0];
	for ( cnt = 0; cnt < CAPTIONLENGTH; cnt++ ) {
		if ( !( *textp++ = *txt++ ) ) break;
	}
	selectFont( calibri );
	setFontColor( _fontColor, _fontBgColor);
}

void Caption::show() {
	clearArea(_fontBgColor);
	drawString( text, gTextAlignMiddleCenter);
}

void Caption::appendText( char *txt ) {
char *textp;
uint8_t cnt;
	textp = &text[0];
	while ( *textp ) {
		cnt++;
		textp++;
	}
	while ( cnt < CAPTIONLENGTH ) {
		if ( !( *textp++ = *txt++ ) ) break;		
	}
	show();
}

void Caption::appendChar( char ch ) {
	char *textp;
	uint8_t cnt;
	textp = &text[0];
	while ( *textp ) {
		cnt++;
		textp++;
	}
	if ( cnt < CAPTIONLENGTH-1 ) {
		*textp++ = ch;
		*textp = 0x00;
	}
	show();
}

void Caption::backSpace() {
	char *textp;
	uint8_t cnt;
	textp = &text[0];
	while ( *textp ) {
		cnt++;
		textp++;
	}
	if ( cnt > 0 ) {
		textp--;
		*textp = 0x00;
	}
	show();
}
void Caption::clear() {
	for ( uint8_t cnt = 0; cnt < CAPTIONLENGTH; cnt++ ) text[cnt] = 0x00;	
}

Button::Button( ILI9341_due *ili, int16_t X1, int16_t Y1, int16_t X2, int16_t Y2, uint16_t but_color, uint16_t txt_color, const char *cap):ILI9341_due_gText(ili,X1,Y1,X2,Y2) {
	strcpy(caption,cap);
	selectFont( calibri );
	_fontBgColor = but_color;
	_fontColor = txt_color;
	setFontColor( _fontColor, _fontBgColor);
}

void Button::show() {
	clearArea(_fontBgColor);
	drawString( caption, gTextAlignMiddleCenter);
}

cartesian::cartesian( ILI9341_due *display ) {
	disp = display;
}

void cartesian::begin( uint16_t X, uint16_t Y, uint16_t W, uint16_t H, uint16_t color ) {
	c_X = p_X = X;
	c_Y = p_Y = Y;
	c_W = p_W = W;
	c_H = p_H = H;
	disp->fillRect(c_X,c_Y,c_W,c_H,color);
}

void cartesian::setAxis( uint8_t index, float Min, float Max, float majorTick, float minorTick ) {
	if ( !axis[index].defined ) {
		switch ( index ) {
			case AX_VASEN: {
				p_X += 20;
				p_W -= 20;
				break;
			}
			case AX_ALA: {
				p_H -= 20;
				break;
			}
			case AX_OIKEA: {
				p_W -= 20;
				break;
			}
		}
	}
	if ( index < MAXAXES ) {
		axis[index].defined = true;
		axis[index].axisMax = Max;
		axis[index].axisMin = Min;
		axis[index].majorTick = majorTick;
		axis[index].minorTick = minorTick;
	}
}
void cartesian::showTicks(uint8_t index, boolean majors, boolean minors) {
	axis[index].majorTicksOn = majors;
	axis[index].minorTicksOn = minors;
}


void cartesian::show() {
	for (uint8_t cnt = 0; cnt < MAXAXES; cnt++ ) {
		drawAxis( cnt );
		if ( axis[cnt].majorTicksOn ) {
			drawMajorTicks( cnt );
			drawMajorLabels( cnt );
		}
	}
}

void cartesian::drawAxis( uint8_t index ) {
	if ( axis[index].defined ) {
		switch ( index ) {
			case AX_VASEN: {
				disp->drawFastVLine( p_X,p_Y,p_H,0x0000);
				break;
			}
			case AX_ALA: {
				disp->drawFastHLine( p_X,p_H,p_W,0x0000);
				break;
			}
			case AX_OIKEA: {
				disp->drawFastVLine( p_X+p_W,p_Y,p_H,0x0000);
				break;
			}
		}
	}
}

 int16_t cartesian::getPXPos(uint8_t index, float val) {
 	if ( (index & 0x01) == 0x00 ){
		return (p_Y + p_H) - ( val - axis[index].axisMin ) * ( p_H / ( axis[index].axisMax - axis[index].axisMin ) );
	} else {
		return p_X + val * ( p_W / ( axis[index].axisMax - axis[index].axisMin ) );
	}
}

void cartesian::drawLine(float x0, float y0, float x1, float y1) {
	disp->drawLine(getPXPos(1, x0), getPXPos(0, y0), getPXPos(1, x1), getPXPos(0, y1), 0x0000 );
}

void cartesian::drawMajorTicks( uint8_t index ) {
float tickValue, tickValToPX;
int32_t tickIndex, tickPos;
uint16_t t_X, t_Y, t_H, t_W;

	if ( axis[index].defined ) {
		if ( (index & 0x01) == 0x00 ){
			tickValToPX = p_H / (axis[index].axisMax - axis[index].axisMin);	// muunnoskerroin pikseliä / yksikkö jos pystyakseli
		} else {
			tickValToPX = p_W / (axis[index].axisMax - axis[index].axisMin);	// muunnoskerroin pikseliä / yksikkö jos vaaka-akseli
		}
		tickIndex = ( uint32_t) ( ( axis[index].axisMin / axis[index].majorTick ) + 1 );
		tickValue = tickIndex * axis[index].majorTick;
		while ( tickValue <= axis[index].axisMax ) {
			switch ( index ) {
				case AX_VASEN: {
					t_X = p_X - MAJORTICKWIDTH;
					t_Y = ( p_Y + p_H ) - ( uint16_t ) ( tickValToPX * ( tickValue - axis[index].axisMin ) );
					t_W = MAJORTICKWIDTH;
					disp->drawFastHLine( t_X, t_Y, t_W, 0x0000 );
					break;
				}
				case AX_ALA: {
					t_H = MAJORTICKWIDTH;
					t_Y = p_H;
					t_X = p_X  + ( uint16_t ) ( tickValToPX * ( tickValue - axis[index].axisMin ) );
					disp->drawFastVLine( t_X, t_Y, t_H, 0x0000 );
					break;
				}
				case AX_OIKEA: {
					t_X = p_X + p_W;
					t_Y = ( p_Y + p_H ) - ( uint16_t ) ( tickValToPX * ( tickValue - axis[index].axisMin ) );
					t_W = MAJORTICKWIDTH;
					disp->drawFastHLine( t_X, t_Y, t_W, 0x0000 );
					break;
				}
			}
			tickIndex += 1;
			tickValue = tickIndex * axis[index].majorTick;
		}
	}
}

void cartesian::drawMajorLabels( uint8_t index ) {
float tickValue, tickValToPX;
int32_t tickIndex;
uint16_t t_X, t_Y;

	if ( axis[index].defined ) {
		disp->setTextSize(1);
		if ( (index & 0x01) == 0x00 ){
			tickValToPX = p_H / (axis[index].axisMax - axis[index].axisMin);	// muunnoskerroin pikseliä / yksikkö jos pystyakseli
		} else {
			tickValToPX = p_W / (axis[index].axisMax - axis[index].axisMin);	// muunnoskerroin pikseliä / yksikkö jos vaaka-akseli
		}
		tickIndex = ( uint32_t) ( ( axis[index].axisMin / axis[index].majorTick ) + 1 );
		tickValue = tickIndex * axis[index].majorTick;
		while ( tickValue <= axis[index].axisMax ) {
			switch ( index ) {
				case AX_VASEN: {
					t_X = c_X;
					t_Y = ( p_Y + p_H - 3 ) - ( uint16_t ) ( tickValToPX * ( tickValue - axis[index].axisMin ) );
					disp->setCursor( t_X, t_Y );
					disp->println( (int16_t) tickValue );
					break;
				}
				case AX_ALA: {
					t_Y = p_H + 6;
					t_X = p_X - 10 + ( uint16_t ) ( tickValToPX * ( tickValue - axis[index].axisMin ) );
					disp->setCursor( t_X, t_Y );
					disp->println( (int16_t) tickValue );
					break;
				}
				case AX_OIKEA: {
					t_X = p_X + p_W + 5;
					t_Y = ( p_Y + p_H - 3 ) - ( uint16_t ) ( tickValToPX * ( tickValue - axis[index].axisMin ) );
					disp->setCursor( t_X, t_Y );
					disp->println( (int16_t) tickValue );
					break;
				}
			}
			tickIndex += 1;
			tickValue = tickIndex * axis[index].majorTick;
		}
	}
}

