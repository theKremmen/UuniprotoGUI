/*
Pienen juotosuunin ohjaus Arduino Duen avulla.

*/

#include <ILI_SdSpi.h>
#include <ILI_SdFatConfig.h>
#include <ILI9341_due_gText.h>
#include <ILI9341_due.h>

#include "cartesian.h"
#include "allFonts.h"

//Arduino Due käyttää näitä DMA-moodissa
#define TFT_DC 9
#define TFT_CS 10
#define TFT_RST 8

// ruudun koko
#define DISP_W 240
#define DISP_H 320


// kosketusnäytön pinnit
#define XM A2 
#define XP 50   
#define YM A3
#define YP 52

//Kosketusnäytön A/D-muunnosarvot
#define MIN_X 191
#define MAX_X 858
#define MIN_Y 108
#define MAX_Y 907

#define BGCOLOR1 ILI9341_CYAN

// Buttonrasterin mittoja näytöllä
#define X_MARGIN 10
#define Y_MARGIN 10

#define B_LEFT_X1 X_MARGIN
#define B_LEFT_X2 ((DISP_W/2)-(X_MARGIN/2))

#define B_RIGHT_X1 ((DISP_W/2)+(X_MARGIN/2))
#define B_RIGHT_X2 (DISP_W-X_MARGIN)

#define B_TOP_Y1 ((DISP_H/2)+Y_MARGIN)
#define B_TOP_Y2 ((DISP_H*3/4)-(Y_MARGIN/2))

#define B_BOTTOM_Y1 ((DISP_H*3/4)+(Y_MARGIN/2))
#define B_BOTTOM_Y2 (DISP_H-Y_MARGIN)

inline void digitalWriteDirect(int pin, boolean val){
	if(val) g_APinDescription[pin].pPort -> PIO_SODR = g_APinDescription[pin].ulPin;
	else    g_APinDescription[pin].pPort -> PIO_CODR = g_APinDescription[pin].ulPin;
}

ILI9341_due tft = ILI9341_due( TFT_CS, TFT_DC, TFT_RST );

cartesian graph = cartesian( &tft );

enum eventID_t {eventNone, eventTouch, eventTimer}; 
	
struct event_t {
	eventID_t eventID;
	union {
		uint32_t time;
		struct {
			uint16_t x;
			uint16_t y;
			} pos;
		uint8_t byte[4];
	} event_data;
} event;

 uint8_t (*viewHandler)(struct event_t event);

uint8_t touching() {
	if (digitalRead( YM ) == LOW) return 1;
	else return 0;
}

static void insert_sort(uint16_t array[], uint8_t size) {
	uint16_t j;
	uint16_t save;
	
	for (uint16_t i = 1; i < size; i++) {
		save = array[i];
		for (j = i; j >= 1 && save < array[j - 1]; j--)
		array[j] = array[j - 1];
		array[j] = save;
	}
}

#define NUMSAMPLES 5

uint16_t x, y, z;

void resetTouchPins() {
	pinMode( XP, OUTPUT );
	digitalWrite( XP, LOW );
	
	pinMode( XM, INPUT );
	digitalWrite( XM, LOW );
	
	pinMode( YP, INPUT );
	digitalWrite( YM, LOW );

	pinMode( YM, INPUT_PULLUP );
	digitalWrite( YM, HIGH );
	
}

void setTouchPinsForX() {
	pinMode(YP, INPUT);
	pinMode(YM, INPUT);
	digitalWrite(YP, LOW);
	digitalWrite(YM, LOW);
	
	pinMode(XP, OUTPUT);
	pinMode(XM, OUTPUT);
	digitalWrite(XP, HIGH);
	digitalWrite(XM, LOW);
}

void setTouchPinsForY() {
	pinMode(XP, INPUT);
	pinMode(XM, INPUT);
	digitalWrite(XP, LOW);
	digitalWrite(XM, LOW);
	
	pinMode(YP, OUTPUT);
	pinMode(YM, OUTPUT);
	digitalWrite(YP, HIGH);
	digitalWrite(YM, LOW);
}

void getPoint(uint16_t *x, uint16_t *y) {
	uint16_t samples[NUMSAMPLES];
	uint8_t i;
	uint16_t _x, _y;

	setTouchPinsForX();
	
	for (i=0; i<NUMSAMPLES; i++) {
		samples[i] = analogRead(YP);
	}

	#if NUMSAMPLES > 2
	insert_sort(samples, NUMSAMPLES);
	#endif
	#if NUMSAMPLES == 2
	if (samples[0] != samples[1]) { valid = 0; }
	#endif

	_x = (samples[NUMSAMPLES/2]);

	setTouchPinsForY();
	
	for (i=0; i<NUMSAMPLES; i++) {
		samples[i] = analogRead(XM);
	}

	#if NUMSAMPLES > 2
	insert_sort(samples, NUMSAMPLES);
	#endif
	#if NUMSAMPLES == 2
	if (samples[0] != samples[1]) { valid = 0; }
	#endif

	_y = (samples[NUMSAMPLES/2]);

	resetTouchPins();
			Serial.print("_X ");
			Serial.print(_x);
			Serial.print(" _Y ");
			Serial.println(_y);

	*x = map(_x,MIN_X,MAX_X, 0, DISP_W);
	*y = map(_y,MIN_Y,MAX_Y, 0, DISP_H);

}


// staattisesti varatut näyttöelementit

#define UINTSTACKDEPTH 20
class UintStack {
public:
	void push( uint16_t item );
	uint16_t pop();
private:
	uint8_t sptr;
	uint16_t stk[UINTSTACKDEPTH];
};

void UintStack::push( uint16_t item) {
	if ( sptr < UINTSTACKDEPTH ) {
		stk[sptr++] = item;
	}
}

uint16_t UintStack::pop() {
	if ( sptr > 0 ) {
		return stk[--sptr];
	}
	else return 1;
}

UintStack viewStack;

// Numerosyöttönäyttö
//----------------------

#define BMARGIN 6
#define BTOP 80
#define BBOTTOM (DISP_H-BMARGIN/2)
#define BHEIGHT ((BBOTTOM-BTOP)/5)
#define BCOLUMNS 3

#define X1_1 (BMARGIN/2)
#define X1_2 (DISP_W/BCOLUMNS-(BMARGIN/2))
#define X2_1 (DISP_W/BCOLUMNS+(BMARGIN/2))
#define X2_2 (2*DISP_W/BCOLUMNS-(BMARGIN/2))
#define X3_1 (2*DISP_W/BCOLUMNS+(BMARGIN/2))
#define X3_2 (DISP_W-BMARGIN/2)

#define Y1_1 (BTOP+BMARGIN/2)
#define Y1_2 (BTOP+BHEIGHT-BMARGIN/2)
#define Y2_1 (BTOP+BHEIGHT+BMARGIN/2)
#define Y2_2 (BTOP+2*BHEIGHT-BMARGIN/2)
#define Y3_1 (BTOP+2*BHEIGHT+BMARGIN/2)
#define Y3_2 (BTOP+3*BHEIGHT-BMARGIN/2)
#define Y4_1 (BTOP+3*BHEIGHT+BMARGIN/2)
#define Y4_2 (BTOP+4*BHEIGHT-BMARGIN/2)
#define Y5_1 (BTOP+4*BHEIGHT+BMARGIN/2)
#define Y5_2 BBOTTOM


#define GETDIGITS 9
#define BGCOLOR9 ILI9341_BLACK
Caption caption( &tft, X1_1, BMARGIN/2, X3_2, BTOP/2-BMARGIN/2, ILI9341_YELLOW, ILI9341_BLACK, "caption");
Caption entry( &tft, X1_1, BTOP/2+BMARGIN/2, X3_2, BTOP-BMARGIN/2, ILI9341_GREEN, ILI9341_BLACK, "");
Button digit7Button( &tft, X1_1, Y1_1, X1_2, Y1_2, ILI9341_BLUE, ILI9341_YELLOW, "7");
Button digit8Button( &tft, X2_1, Y1_1, X2_2, Y1_2, ILI9341_BLUE, ILI9341_YELLOW, "8");
Button digit9Button( &tft, X3_1, Y1_1, X3_2, Y1_2, ILI9341_BLUE, ILI9341_YELLOW, "9");
Button digit4Button( &tft, X1_1, Y2_1, X1_2, Y2_2, ILI9341_BLUE, ILI9341_YELLOW, "4");
Button digit5Button( &tft, X2_1, Y2_1, X2_2, Y2_2, ILI9341_BLUE, ILI9341_YELLOW, "5");
Button digit6Button( &tft, X3_1, Y2_1, X3_2, Y2_2, ILI9341_BLUE, ILI9341_YELLOW, "6");
Button digit1Button( &tft, X1_1, Y3_1, X1_2, Y3_2, ILI9341_BLUE, ILI9341_YELLOW, "1");
Button digit2Button( &tft, X2_1, Y3_1, X2_2, Y3_2, ILI9341_BLUE, ILI9341_YELLOW, "2");
Button digit3Button( &tft, X3_1, Y3_1, X3_2, Y3_2, ILI9341_BLUE, ILI9341_YELLOW, "3");
Button digitBackButton( &tft, X1_1, Y4_1, X1_2, Y4_2, ILI9341_CYAN, ILI9341_BLACK, "back");
Button digit0Button( &tft, X2_1, Y4_1, X2_2, Y4_2, ILI9341_BLUE, ILI9341_YELLOW, "0");
Button digitDecButton( &tft, X3_1, Y4_1, X3_2, Y4_2, ILI9341_BLUE, ILI9341_YELLOW, ",");
Button digitCancelButton( &tft, X1_1, Y5_1, B_LEFT_X2, Y5_2, ILI9341_RED, ILI9341_YELLOW, "CANCEL");
Button digitOKButton( &tft, B_RIGHT_X1, Y5_1, X3_2, Y5_2, ILI9341_GREEN, ILI9341_BLACK, "OK");

Button digitBtns[] = {
	digit0Button,
	digit1Button,
	digit2Button,
	digit3Button,
	digit4Button,
	digit5Button,
	digit6Button,
	digit7Button,
	digit8Button,
	digit9Button
};

void drawDigitEntryView() {
	tft.fillScreen(BGCOLOR9);
	caption.show();
	entry.clear();
	entry.selectFont(fixednums8x16);
	entry.show();
	for ( uint8_t cnt = 0; cnt < 10; cnt++ ) {
		digitBtns[cnt].selectFont(fixednums8x16);
		digitBtns[cnt].show();
	}
	digitBackButton.show();
	digitDecButton.selectFont(fixednums8x16);
	digitDecButton.show();
	digitCancelButton.show();
	digitOKButton.show();
}

uint8_t digitEntryViewHandler(struct event_t event) {
	boolean hit;
	uint8_t cnt;
	uint16_t X,Y;
	X = Y = 0;
	if ( event.eventID == eventTouch ) {
		X = event.event_data.pos.x;
		Y = event.event_data.pos.y;
	}
	hit = false;
	for ( cnt = 0; cnt < 10; cnt++ ) {
		if ( digitBtns[cnt].isInside( X, Y ) ) {
			hit = true;
			entry.appendChar( (char)(0x30+cnt) );
			return 0;
		}
	}
	if ( !hit ) {
		if ( digitDecButton.isInside( X, Y ) ) {
			entry.appendChar(',');
			return 0;
		}
		else if ( digitBackButton.isInside( X, Y ) ) {
			entry.backSpace();
			return 0;
		}
		else if ( digitOKButton.isInside( X, Y ) ) {
			return viewStack.pop();
		}
		else if ( digitCancelButton.isInside( X, Y ) ) {
			return viewStack.pop();
		}
		else return 0;
	}
	return 0;
}

//Juotossekvenssinäyttö

#define JUOTOSNAYTTO 2
Button holdButton( &tft, B_LEFT_X1, B_BOTTOM_Y1, B_LEFT_X2, B_BOTTOM_Y2, ILI9341_YELLOW, ILI9341_BLACK, "HOLD");
Button abortButton( &tft, B_RIGHT_X1, B_BOTTOM_Y1, B_RIGHT_X2, B_BOTTOM_Y2, ILI9341_RED, ILI9341_BLACK, "ABORT");

void drawSequenceView() {
	tft.fillScreen(BGCOLOR1);
	graph.begin( 0,0,240,240,ILI9341_PANTHER );
	graph.setAxis( AX_VASEN, 20.0, 260.0, 50.0, 10.0 );
	graph.showTicks( AX_VASEN, true, false );
	graph.setAxis( AX_ALA, 0.0, 300.0, 60.0, 10.0 );
	graph.showTicks( AX_ALA, true, false );
	graph.setAxis( AX_OIKEA, 0, 100.0, 10.0, 5.0 );
	graph.showTicks( AX_OIKEA, true, false );
	graph.show();
	graph.drawLine(30,60,180,150);
	holdButton.show();
	abortButton.show();
}

uint8_t sequenceViewHandler(struct event_t event) {
	uint16_t X,Y;
	X = Y = 0;
	if ( event.eventID == eventTouch ) {
		X = event.event_data.pos.x;
		Y = event.event_data.pos.y;
		if ( abortButton.isInside(X,Y) ) {
			return 1;
		}
		if ( holdButton.isInside(X,Y) ) {
		}
	}
	return 0;
}

// Profiilinäyttö
//----------------------------------------------------------

#define PROFIILINAYTTO 3
#define BGCOLOR3 ILI9341_PANTHER

Caption stdLead( &tft, X1_1, BMARGIN/2, X3_2, BTOP/2-BMARGIN/2, ILI9341_GRAY, ILI9341_BLACK, "std LEAD");
Caption stdRoHS( &tft, X1_1, BTOP/2+BMARGIN/2, X3_2, BTOP-BMARGIN/2, ILI9341_GRAY, ILI9341_BLACK, "std RoHS");
Button profOKButton( &tft, B_RIGHT_X1, Y5_1, X3_2, Y5_2, ILI9341_GREEN, ILI9341_BLACK, "OK");


void drawProfileView() {
	tft.fillScreen(BGCOLOR3);
	stdLead.show();
	stdRoHS.show();
	profOKButton.show();
}

uint8_t profileViewHandler(struct event_t event) {
	uint16_t X,Y;
	X = Y = 0;
	if ( event.eventID == eventTouch ) {
		X = event.event_data.pos.x;
		Y = event.event_data.pos.y;
		if ( stdLead.isInside(X,Y) ) {
			stdLead.setFontColor(ILI9341_BLACK,ILI9341_YELLOW);
			stdRoHS.setFontColor(ILI9341_BLACK,ILI9341_GRAY);
			stdLead.show();
			stdRoHS.show();
			return 0;
		}
		if ( stdRoHS.isInside(X,Y) ) {
			stdLead.setFontColor(ILI9341_BLACK,ILI9341_GRAY);
			stdRoHS.setFontColor(ILI9341_BLACK,ILI9341_YELLOW);
			stdLead.show();
			stdRoHS.show();
			return 0;
		}
		if ( profOKButton.isInside(X,Y) ) {
			return viewStack.pop();
		}
	}
	return 0;
}

// Starttinäyttö
//----------------------------------------------------------

PGM_P introteksti[] = {
	"Juotosuuni v 1.0.2",
	"Programmed by Kremmen",
	"Last compilation",
	"22 Feb 2015",
	NULL
};

#define ALOITUSNAYTTO 1
Button startButton( &tft, B_LEFT_X1, B_TOP_Y1, B_LEFT_X2, B_TOP_Y2, ILI9341_PANTHER, ILI9341_BLACK, "START");
Button profButton( &tft, B_RIGHT_X1, B_TOP_Y1, B_RIGHT_X2, B_TOP_Y2, ILI9341_RED, ILI9341_BLACK, "PROFILE");

Button confButton( &tft, B_LEFT_X1, B_BOTTOM_Y1, B_LEFT_X2, B_BOTTOM_Y2, ILI9341_GREEN, ILI9341_BLACK, "CONFIGURE");
Button reportButton( &tft, B_RIGHT_X1, B_BOTTOM_Y1, B_RIGHT_X2, B_BOTTOM_Y2, ILI9341_BLUE, ILI9341_WHITE, "REPORT");

PGM_TextArea intro( &tft, X_MARGIN, Y_MARGIN, (DISP_W-X_MARGIN), (DISP_H/2-Y_MARGIN), ILI9341_YELLOW, ILI9341_BLACK, introteksti);

void drawStartView() {					// Näytön piirto näkyviin
	tft.fillScreen(BGCOLOR1);
	intro.show();
	startButton.show();
	profButton.show();
	confButton.show();
	reportButton.show();
}

uint8_t startViewHandler(struct event_t event) {
	uint16_t X,Y;
	X = Y = 0;
	if ( event.eventID == eventTouch ) {
		X = event.event_data.pos.x;
		Y = event.event_data.pos.y;
		if ( startButton.isInside( X,Y) ) {
			viewStack.push(1);
			return 2;
		}
		if ( profButton.isInside( X,Y) ) {
			viewStack.push(1);
			return 3;
		}
		if ( confButton.isInside( X,Y ) ) {
			viewStack.push(1);
			return 9;
		}
	}
	return 0;
}


//============================================================================
void setup() {
	Serial.begin(9600);
	resetTouchPins();
	
	// LCD
	tft.begin();
	tft.setRotation( (iliRotation)0 );
	
	drawStartView();
	viewHandler = startViewHandler;
}

void loop(void) {
uint8_t result;
uint16_t x, y;
	if ( touching() ) {
		getPoint(&x, &y);
		event.eventID = eventTouch;
		event.event_data.pos.x = x;
		event.event_data.pos.y = y;
		result = viewHandler( event );
		switch ( result ) {
			case 0: break;	// ei vaihdeta näyttöä
			case ALOITUSNAYTTO: {		// alkunäyttö
				drawStartView();
				viewHandler = startViewHandler;
				break;
			}
			case JUOTOSNAYTTO: {		// juotossekvenssi
				drawSequenceView();
				viewHandler = sequenceViewHandler;
				break;	
			}
			case PROFIILINAYTTO: {		// juotossekvenssi
				drawProfileView();
				viewHandler = profileViewHandler;
				break;
			}
			case GETDIGITS: {		// juotossekvenssi
				drawDigitEntryView();
				viewHandler = digitEntryViewHandler;
				break;
			}
			default: {		// alkunäyttö
				drawStartView();
				viewHandler = startViewHandler;
				break;
			}
		}
		while ( touching() ) {
		}
		delay(50);
	}
}