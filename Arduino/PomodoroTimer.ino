#include <Arduino.h>
#include <Encoder.h>
#include <TM1637Display.h>

#include "Utilities.h"

#define CLK_YELLOW 4
#define DIO_YELLOW 5

#define CLK_BLUE 6
#define DIO_BLUE 7

#define CLK_RED 8
#define DIO_RED 9

#define BUTTON_START 12      // Start/Stop Push Button
#define ROTARY_OUTA 2
#define ROTARY_OUTB 3

#define BUZZER 10

#define CONST_WORK 1500
#define CONST_REST 300

//Words definitions

const uint8_t SEG_DONE[] = {
	SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,           // d
	SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,   // O
	SEG_C | SEG_E | SEG_G,                           // n
	SEG_A | SEG_D | SEG_E | SEG_F | SEG_G            // E
	};

const uint8_t SEG_PUSH[] = {
	SEG_E | SEG_F | SEG_A | SEG_B | SEG_G,           // p
	SEG_F | SEG_E | SEG_D | SEG_C | SEG_B,           // u
	SEG_A | SEG_F | SEG_G | SEG_C | SEG_D,           // s
	SEG_F | SEG_G | SEG_B | SEG_E | SEG_C            // h
	};

const uint8_t SEG_STOP[] = {
	SEG_A | SEG_F | SEG_G | SEG_C | SEG_D,          // s
	SEG_F | SEG_E | SEG_D | SEG_G,                  // t
	SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,  // o
	SEG_E | SEG_F | SEG_A | SEG_B | SEG_G,          // p
	};


int oldEncoderValue;
Encoder encoder(ROTARY_OUTA, ROTARY_OUTB);

TM1637Display displayYellow(CLK_YELLOW, DIO_YELLOW);
TM1637Display displayBlue(CLK_BLUE, DIO_BLUE);
TM1637Display displayRed(CLK_RED, DIO_RED);

TM1637Display *currentDisplay = NULL;

workingStates currentState = STATE_WAITING_FOR_WORK;
workingStates pausedState = STATE_WORKING;

int workSeconds;
int restSeconds;
int pausedSeconds;
int totalTime = 0;
int auxTime = 0;
int encoderValue;

int *timeToCount;
int brightness = 5;

void setup() {

  workSeconds = CONST_WORK;
  restSeconds = CONST_REST;


  pinMode(BUTTON_START, INPUT_PULLUP);
  pinMode(BUZZER, OUTPUT);

  displayYellow.setBrightness(brightness);
  displayBlue.setBrightness(brightness);
  displayRed.setBrightness(brightness);

  displayYellow.showNumberDec(0, false);
  displayBlue.showNumberDec(0, false);
  displayRed.showNumberDec(0, false);

}

void loop() {
  // put your main code here, to run repeatedly:

    if ((millis()/1000)%2 == 0)
    {
      displayYellow.showNumberDecEx(totalTime/3600, 0b11100000, true, 2, 0);
      displayYellow.showNumberDecEx((totalTime / 60) % 60, 0b11100000, true, 2, 2);
    }
    else
    {
      displayYellow.showNumberDecEx(totalTime/3600, 0, true, 2, 0);
      displayYellow.showNumberDecEx((totalTime / 60) % 60, 0, true, 2, 2);
    }

    switch (currentState) 
    {
      case STATE_PAUSED:
        paused();
        break;

      case STATE_WORKING:
        working(&workSeconds);
        break;

      case STATE_WAITING_FOR_REST:
        waitingForRest();
        break;

      case STATE_RESTING:
        resting(&restSeconds);
        break;

      case STATE_WAITING_FOR_WORK:
        waitingForWork();
        break;

      default:
        break;
    }

    encoderValue = (encoder.read()/4);

    if (encoderHasChanged(encoderValue))
    {
        
        if (encoderHasIncreased(encoderValue) && (currentState == STATE_PAUSED || currentState == STATE_WAITING_FOR_REST || currentState == STATE_WAITING_FOR_WORK))
        {
          displayBlue.showNumberDec(0000, false);
          currentState = STATE_WAITING_FOR_WORK;
        }
        else if (!encoderHasIncreased(encoderValue) && (currentState == STATE_PAUSED || currentState == STATE_WAITING_FOR_REST || currentState == STATE_WAITING_FOR_WORK))
        {
          displayRed.showNumberDec(0000, false);
          currentState = STATE_WAITING_FOR_REST;
        }

        oldEncoderValue = encoderValue;
    }
}


//*******************************************************************************
//*******************************************************************************
//*******************************************************************************

//Encoder Functions
bool encoderHasChanged(int encoderValue)
{
    if (encoderValue != oldEncoderValue)
        return true;
    else
        return false;
}

bool encoderHasIncreased(int encoderValue)
{
    if (encoderValue < oldEncoderValue)
        return false;
    else
        return true;
}


//*******************************************************************************
//*******************************************************************************
//*******************************************************************************

void paused()
{
  countSeconds(auxTime, auxTime);

  if (digitalRead(BUTTON_START) == LOW)
  {
    delay(300);
    pausedSeconds = *timeToCount;
    currentState = pausedState;
  }
}

int working(int *timeToCount)
{
  displayRed.showNumberDec(*timeToCount, false);
  countSeconds(timeToCount, &totalTime);

  if (digitalRead(BUTTON_START) == LOW)
  {
    displayRed.setSegments(SEG_STOP);
    delay(300);
    pausedSeconds = *timeToCount;
    pausedState = STATE_WORKING;
    currentState = STATE_PAUSED;
  }

  if (*timeToCount < 0)
  {
    displayRed.setSegments(SEG_DONE);
    buzzerSound(BUZZER, 10);
    currentState = STATE_WAITING_FOR_REST;
  }
}


int waitingForRest()
{
  displayBlue.setSegments(SEG_PUSH);
  countSeconds(auxTime, auxTime);
  if (digitalRead(BUTTON_START) == LOW)
  {
    delay(300);
    restSeconds = CONST_REST;
    currentState = STATE_RESTING;
  }
}


int resting(int *timeToCount)
{
  displayBlue.showNumberDec(*timeToCount, false);
  countSeconds(timeToCount, &totalTime);

  if (digitalRead(BUTTON_START) == LOW)
  {
    displayBlue.setSegments(SEG_STOP);
    delay(300);
    pausedSeconds = *timeToCount;
    pausedState = STATE_RESTING;
    currentState = STATE_PAUSED;
  }

  if (*timeToCount < 0)
  {
    displayBlue.setSegments(SEG_DONE);
    buzzerSound(BUZZER, 10);
    currentState = STATE_WAITING_FOR_WORK;
  }
}


int waitingForWork()
{
  displayRed.setSegments(SEG_PUSH);
  countSeconds(auxTime, auxTime);
  if (digitalRead(BUTTON_START) == LOW)
  {
    delay(300);
    workSeconds = CONST_WORK;
    currentState = STATE_WORKING;
  }
}

//*******************************************************************************
//*******************************************************************************
//*******************************************************************************
