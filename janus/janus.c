#include <tice.h>
#include <keypadc.h>

#include "janus.h"

uint8_t janus_ReleasedKeys[KB_DATA_SIZE];
uint8_t janus_PressedKeys[KB_DATA_SIZE];
uint8_t janus_PressedOrReleasedKeys[KB_DATA_SIZE];


void janus_UpdateDebouncedKeys(void) {
    static uint8_t janus_LastPressed[KB_DATA_SIZE];
    uint8_t i;
    for(i = 1; i < KB_DATA_SIZE; i++) {
        janus_PressedOrReleasedKeys[i] = janus_LastPressed[i] ^ (kb_Data[i]);
        janus_ReleasedKeys[i] = janus_LastPressed[i] & janus_PressedOrReleasedKeys[i];
        janus_PressedKeys[i] = (~janus_LastPressed[i]) & janus_PressedOrReleasedKeys[i];
        
        // There's probably an easy way to copy the data from kb_Data to this variable
        // ...but I couldn't find it
        janus_LastPressed[i] = kb_Data[i];
    }
}

float janus_GetElapsedTime(void) {
    static uint8_t timerStarted = 0;
    static uint24_t previousTime = 0;
    if(timerStarted) {
        uint24_t newTime;
        float outputValue;
        newTime = timer_Get(JANUS_TIMER_NUMBER);
        outputValue = ((float)newTime - previousTime) / 32000;
        if(outputValue < 0) {
            // This condition is true when the timer overflows and restarts, so the line of code below makes sure to fix anomalies
            // (When it overflows, the float captures a negative value)
            outputValue = outputValue + (UINT24_MAX/32000.0f);
        }
        previousTime = newTime;
        return outputValue;
    } else {
        //Starts timer
        timer_Disable(JANUS_TIMER_NUMBER);
        timer_Set(JANUS_TIMER_NUMBER,0);
        timer_Enable(JANUS_TIMER_NUMBER,TIMER_32K,TIMER_NOINT,JANUS_TIMER_DIRECTION);
        timerStarted = 1;
        return 0.0f;
    }
}

int24_t janus_LerpInt24(int24_t x, int24_t y, float alpha) {
    return (int24_t)(x+(y-x)*alpha);
}
//https://easings.net/ for reference
static float janus_GetAnimationAlpha(enum janus_AnimationEasingMode easingMode, float progress, float length) {
    float percentage = progress/length;
    switch(easingMode) {
        case LINEAR:
            return percentage;
        case QUAD_IN:
            return percentage*percentage;
        case QUAD_OUT:
            return 1-(1-percentage)*(1-percentage);
        case QUAD_IN_OUT:
            return percentage <= 0.5 ? (2 * percentage * percentage) : (1-(-2* percentage + 2)*(-2* percentage + 2) / 2);
    }
}
void janus_UpdateAnimation(struct janus_Animation* animation, float elapsedTime) {
    float alpha;

    if(elapsedTime == 0) {
        return;
    }
    
    animation->progress = animation->progress + (animation->reverse ? -elapsedTime : elapsedTime);
    if(animation->progress > animation->length) {
        animation->progress = animation->length;
    }
    if(animation->progress < 0) {
        animation->progress = 0;
    }
    alpha = janus_GetAnimationAlpha(animation->easingMode,animation->progress,animation->length);
    if(animation->fromX != animation->toX) {
        animation->currentX = janus_LerpInt24(animation->fromX,animation->toX,alpha);
    } else {
        animation->currentX = animation->toX;
    }
    if(animation->fromY != animation->toY) {
        animation->currentY = janus_LerpInt24(animation->fromY,animation->toY,alpha);
    } else {
        animation->currentY = animation->toY;
    }
}
