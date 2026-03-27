#include "game.h"
#include "gpio.h"
#include "systick.h"
#include "display.h"
#include <stdlib.h>
#include <stdio.h>

static const Pin_t Buzzer = {GPIOA, 0}; //Buzzer
static Time_t buzzerDelay; //used to delay the buzzer sound on an interval
static int buzzerActive; //flag used to see if the buzzer is on or off
static int beeps; //Number of times the buzzer beeped
#define BUZZER_INTERVAL 500 // the buzzer will make a sound

static int randomNumber;//Random number used to determine what position to spawn the mole
static int spawnPos; //Spawning position of a mole
static int posArray[8] = {0,0,0,0,0,0,0,0};
static int moleSpawnRate;//1s, 500ms, or 250ms
static int moleLifeTime[8] = {0,0,0,0,0,0,0,0}; //This is 4 times whatever the spawn rate is

static int gameTimer;
static const int GAME_LENGTH = 60000;
static int spawnTimer;
static int spawnSpeedSel;
static enum {IN_MENU, IN_GAME, GAME_OVER}gameState;

static int score; //The player's score, increments for each mole whacked, decrements for each miss
uint8_t LEDPos = 0;
#define DEBOUNCE_INTERVAL 250
static Time_t lastPress;
static int validPress;
static int resetTriggered;

void Init_Game (void) {
	//Setting up GPIO things
	GPIO_Enable(Buzzer);
	GPIO_Mode(Buzzer, OUTPUT);

	GPIO_PortEnable(GPIOX);
	DisplayEnable();
	DisplayPrint(GAME, 1, "Speed: Slow");
	spawnTimer = TimeNow();

	srand(TimeNow());
	randomNumber = 0;

	buzzerDelay = 0;
	buzzerActive = 0;
	beeps = 0;

	moleSpawnRate = 500;
	spawnPos = 0;
	gameState = IN_MENU;
	spawnSpeedSel = 0; //0-slow 1-Normal 2-Fast
}
void Task_Game (void) {
	if(GPIO_PortInput(GPIOX) && TimePassed(lastPress) > DEBOUNCE_INTERVAL){
		lastPress = TimeNow();
		validPress = 1;
	}

	if(gameState == IN_MENU){
		DisplayColour(GAME, WHITE);
		DisplayPrint(GAME, 0, "WHACK-A-MOLE");
		GPIO_Output(Buzzer, LOW);

		if(GPIO_PortInput(GPIOX) & (1 << 11) && validPress){
			if(spawnSpeedSel == 0){
				moleSpawnRate = 1000;
				DisplayPrint(GAME,1, "Speed: Slow");
				spawnSpeedSel++;
				spawnSpeedSel %= 3;
			}
			else if(spawnSpeedSel == 1){
				moleSpawnRate = 500;
				DisplayPrint(GAME,1, "Speed: Normal");
				spawnSpeedSel++;
				spawnSpeedSel %= 3;
			}
			else if(spawnSpeedSel == 2){
				moleSpawnRate = 250;
				DisplayPrint(GAME,1, "Speed: Fast");
				spawnSpeedSel++;
				spawnSpeedSel %= 3;
			}
		}
		if(GPIO_PortInput(GPIOX) & (1 << 12) && validPress){
			DisplayPrint(GAME, 0, "ONLY ONE FINGER!");
			DisplayColour(GAME, GREEN);

			for(int i = 0; i<8; i++){
				posArray[i] = 0;
			}

			gameState = IN_GAME;
			spawnTimer = TimeNow();
			gameTimer = TimeNow();
			lastPress = TimeNow();
			LEDPos = 0;
			score = 0; //Reset score
		}

		if((TimePassed(spawnTimer) >= moleSpawnRate)){
			int randNum = rand() % 8;
			spawnPos = randNum;

			if((posArray[spawnPos] != 1)){ //Check if there is a mole already there
				posArray[spawnPos] = 1;
				moleLifeTime[spawnPos] = TimeNow();

			}

			srand(TimeNow());
			resetTriggered = 1;
		}
		else{
			resetTriggered = 0;
		}

		for(int i = 0; i < 8; i++){
			if(TimePassed(moleLifeTime[i]) >= moleSpawnRate*4){
				LEDPos &= ~(posArray[i] << i);
				LEDPos &= ~(1 << i);
				moleLifeTime[i] = 0;
				posArray[i] = 0;
			}
			else{
				if(posArray[i] == 1){
					LEDPos |= (1<<i);
				}
			}
		}

		GPIO_PortOutput(GPIOX, LEDPos);

		if(resetTriggered){
			spawnTimer = TimeNow();
			resetTriggered = 0;
		}

	}
	else if(gameState == GAME_OVER){
		DisplayColour(GAME, RED);
		DisplayPrint(GAME, 0, "GAME OVER!");
		DisplayPrint(GAME, 1, "Score: %d", score);

		if(TimePassed(buzzerDelay) >= BUZZER_INTERVAL && beeps < 5){
			buzzerDelay = TimeNow();
			if(!buzzerActive){
				GPIO_Output(Buzzer, HIGH);
				buzzerActive = 1;
				beeps++;
			}
			else{
				GPIO_Output(Buzzer, LOW);
				buzzerActive = 0;
			}
		}
		else if(beeps >= 5){
			GPIO_Output(Buzzer, LOW);
		}


		if(GPIO_PortInput(GPIOX)){
			gameState = IN_MENU;
		}
	}
	else{

		if((TimePassed(spawnTimer) >= moleSpawnRate)){
			int randNum = rand() % 8;
			spawnPos = randNum;

			if((posArray[spawnPos] != 1)){ //Check if there is a mole already there
				posArray[spawnPos] = 1;
				moleLifeTime[spawnPos] = TimeNow();

			}

			srand(TimeNow());
			resetTriggered = 1;
		}
		else{
			resetTriggered = 0;
		}

		for(int i = 0; i < 8; i++){
			if(TimePassed(moleLifeTime[i]) >= moleSpawnRate*4){
				LEDPos &= ~(posArray[i] << i);
				LEDPos &= ~(1 << i);
				moleLifeTime[i] = 0;
				posArray[i] = 0;
			}
			else{
				if(posArray[i] == 1){
					LEDPos |= (1<<i);
				}
			}
		}

		GPIO_PortOutput(GPIOX, LEDPos);

		if(GPIO_PortInput(GPIOX) & (1 << 8) && validPress){
			if(posArray[0] == 1){
				posArray[0] = 0;
				LEDPos &= ~(1 << 0);
				moleLifeTime[0] = 0;
				score++;
			}
			else{
				score--;
			}
		}
		else if(GPIO_PortInput(GPIOX) & (1 << 9) && validPress){
			if(posArray[1] == 1){
				posArray[1] = 0;
				LEDPos &= ~(1 << 1);
				moleLifeTime[1] = 0;
				score++;
			}
			else{
				score--;
			}
		}
		else if(GPIO_PortInput(GPIOX) & (1 << 10) && validPress){
			if(posArray[2] == 1){
				posArray[2] = 0;
				LEDPos &= ~(1 << 2);
				moleLifeTime[2] = 0;
				score++;
			}
			else{
				score--;
			}
		}
		else if(GPIO_PortInput(GPIOX) & (1 << 11) && validPress){
			if(posArray[3] == 1){
				posArray[3] = 0;
				LEDPos &= ~(1 << 3);
				moleLifeTime[3] = 0;
				score++;
			}
			else{
				score--;
			}
		}
		else if(GPIO_PortInput(GPIOX) & (1 << 12) && validPress){
			if(posArray[4] == 1){
				posArray[4] = 0;
				LEDPos &= ~(1 << 4);
				moleLifeTime[4] = 0;
				score++;
			}
			else{
				score--;
			}
		}
		else if(GPIO_PortInput(GPIOX) & (1 << 13) && validPress){
			if(posArray[5] == 1){
				LEDPos &= ~(1 << 5);
				posArray[5] = 0;
				moleLifeTime[5] = 0;
				score++;
			}
			else{
				score--;
			}
		}
		else if(GPIO_PortInput(GPIOX) & (1 << 14) && validPress){
			if(posArray[6] == 1){
				LEDPos &= ~(1 << 6);
				posArray[6] = 0;
				moleLifeTime[6] = 0;
				score++;
			}
			else{
				score--;
			}
		}
		else if(GPIO_PortInput(GPIOX) & (1 << 15) && validPress){
			if(posArray[7] == 1){
				LEDPos &= ~(1 << 7);
				posArray[7] = 0;
				moleLifeTime[7] = 0;
				score++;
			}
			else{
				score--;
			}
		}

		//Update score
		DisplayPrint(GAME, 1, "Score: %d", score);

		if(resetTriggered){
			spawnTimer = TimeNow();
			resetTriggered = 0;
		}

		if(TimePassed(gameTimer) >= GAME_LENGTH){
			gameState = GAME_OVER;
		}
		else if(TimePassed(gameTimer) >= GAME_LENGTH/2){
			DisplayColour(GAME, YELLOW);
		}
	}
	validPress = 0;
}
