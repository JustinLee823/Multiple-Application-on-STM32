/*
 * display.h
 *
 *  Created on: Oct 22, 2025
 *      Author: justinlee
 */

#ifndef DISPLAY_H_
#define DISPLAY_H_

typedef enum {ALARM=0, CALC=1, GAME=2, ENVIR=3, MOTOR=4}Page_t;
#define PAGES 5

typedef enum{ RED=0xFF00000, GREEN=0x00FF00, BLUE=0x0000FF, YELLOW=0xFFFF00,
		ORANGE=0xFFA500, CYAN=0x00FFFF, MAGENTA=0xFF00FF, WHITE=0xFFFFFF, OFF=0x000000
} Colour_t;

void DisplayEnable(void);
void DisplayPrint(const Page_t page, const int line, const char *msg, ...);
void DisplayColour(const Page_t, Colour_t colour);

void UpdateDisplay(void);
Page_t GetPage(void);

#endif /* DISPLAY_H_ */
