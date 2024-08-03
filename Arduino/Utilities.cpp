#include <Arduino.h>

//Counts and decrease 1 by second
unsigned long previousTime = 0;

void countSeconds(int *timeToCount, int *totalTime)
{
  if (millis() >= (previousTime)) 
  {
    previousTime = previousTime + 1000;  // use 100000 for uS

    if (timeToCount)
      *timeToCount = *timeToCount - 1;
      *totalTime = *totalTime + 1;
  }
}



//Emits a sound from the buzzer
void buzzerSound(int buzzerPin, int volume)
{
    analogWrite(buzzerPin, volume);
    delay(70);
    noTone(buzzerPin);
    delay(20);
    analogWrite(buzzerPin, volume);
    delay(70);
    noTone(buzzerPin);
}