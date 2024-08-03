//General utils

typedef enum{
    STATE_PAUSED,  
    STATE_WORKING,
    STATE_WAITING_FOR_REST,
    STATE_RESTING,
    STATE_WAITING_FOR_WORK
}workingStates;

void countSeconds(int *timeToCount, int *totalTime);
void buzzerSound(int buzzerPin, int volume);