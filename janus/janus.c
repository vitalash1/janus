#include <tice.h>
#include <keypadc.h>
#include <graphx.h>

#include "janus.h"

uint8_t janus_ReleasedKeys[KB_DATA_SIZE];
uint8_t janus_PressedKeys[KB_DATA_SIZE];
uint8_t janus_PressedOrReleasedKeys[KB_DATA_SIZE];


uint24_t janus_Max(uint24_t x, uint24_t y) {
    return y > x ? y : x;
}
uint24_t janus_Min(uint24_t x, uint24_t y) {
    return y > x ? x : y;
}
int24_t janus_LerpInt24(int24_t x, int24_t y, float alpha) {
    return (int24_t)(x+(y-x)*alpha);
}



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
        newTime = (float)timer_Get(JANUS_TIMER_NUMBER);
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



//https://easings.net/ for reference
float janus_GetEaseProgress(enum janus_EasingMode easingMode, float alpha) {
    switch(easingMode) {
        case LINEAR:
            return alpha;
        case QUAD_IN:
            return alpha * alpha;
        case QUAD_OUT:
            return 1 - (1 - alpha) * (1 - alpha);
        case QUAD_IN_OUT:
            return alpha <= 0.5 ? (2 * alpha * alpha) : (1 - (-2 * alpha + 2) * (-2 * alpha + 2) / 2);
    }
}
void janus_UpdateEase(struct janus_Ease* ease, float elapsedTime) {
    float alpha;

    if(elapsedTime == 0) {
        return;
    }
    
    ease->progress = ease->progress + (ease->reverse ? -elapsedTime : elapsedTime);
    if(ease->progress > ease->length) {
        ease->progress = ease->length;
    }
    if(ease->progress < 0) {
        ease->progress = 0;
    }
    alpha = janus_GetEaseProgress(ease->easingMode,ease->progress/ease->length);
    if(ease->fromX != ease->toX) {
        ease->currentX = janus_LerpInt24(ease->fromX,ease->toX,alpha);
    } else {
        ease->currentX = ease->toX;
    }
    if(ease->fromY != ease->toY) {
        ease->currentY = janus_LerpInt24(ease->fromY,ease->toY,alpha);
    } else {
        ease->currentY = ease->toY;
    }
}

void janus_UpdateAnimation(struct janus_Animation* animation, float elapsedTime) {
    float maxLength = 0;
    animation->currentElapsed += elapsedTime;
    
    if(animation->useVariableTimings) {
        for(uint24_t i = 0; i < animation->frameCount; i++) {
            maxLength += animation->variableTimings[i];
        }
    } else {
        maxLength = animation->frameCount * animation->constantTiming;
    }
    if(animation->currentElapsed >= maxLength) {
        animation->currentElapsed -= maxLength;
    }

    if(animation->useVariableTimings) {
        float counter = 0;
        for(uint24_t i = 0; i < animation->frameCount; i++) {
            counter += animation->variableTimings[i];
            if(counter >= animation->currentElapsed) {
                animation->currentFrame = i;
                break;
            }
        }
    } else {
        animation->currentFrame = animation->currentElapsed / animation->constantTiming;
    }
    animation->currentFrame %= animation->frameCount;
}
gfx_sprite_t* janus_GetAnimationFrame(struct janus_Animation* animation) {
    return animation->frames[animation->currentFrame];
}


bool janus_UpdateQuickUI(struct janus_QuickUI* ui) {
    if(janus_PressedKeys[7] & kb_Down && ui->cursorPosition + 1 != JANUS_QUICKUI_MAX_OPTIONS/2 && ui->cursorPosition + 1 < ui->numberOfOptions) {
        ui->cursorPosition++;
    } else if(janus_PressedKeys[7] & kb_Up && ui->cursorPosition - 1 != (JANUS_QUICKUI_MAX_OPTIONS/2) - 1 && ui->cursorPosition > 0 ) {
        ui->cursorPosition--;
    } else if(janus_PressedKeys[7] & kb_Right && ui->cursorPosition < JANUS_QUICKUI_MAX_OPTIONS/2 && ui->cursorPosition + JANUS_QUICKUI_MAX_OPTIONS / 2 < ui->numberOfOptions) {
        ui->cursorPosition += JANUS_QUICKUI_MAX_OPTIONS/2;
    } else if(janus_PressedKeys[7] & kb_Left && ui->cursorPosition >= JANUS_QUICKUI_MAX_OPTIONS/2) {
        ui->cursorPosition -= JANUS_QUICKUI_MAX_OPTIONS/2;
    }
    if(janus_ReleasedKeys[6] & kb_Enter) {
        return true;
    }
    return false;
}

void janus_RenderQuickUI(struct janus_QuickUI* ui) {
    uint8_t i;
    uint24_t largestWidthFirstColumn = 0;

    gfx_SetTextFGColor(ui->foregroundColor);
    gfx_SetTextScale(2,2);
    
    gfx_PrintStringXY(ui->title,ui->xOffset + JANUS_QUICKUI_PADDING,ui->yOffset + JANUS_QUICKUI_PADDING);
    
    gfx_SetTextScale(1,1);
    for(i = 0; i < ui->numberOfOptions; i++) {
        uint24_t x,w;
        uint8_t y,h;
        bool selecting;
        w = gfx_GetStringWidth(ui -> options[i])+(JANUS_QUICKUI_PADDING*2);
        h = JANUS_QUICKUI_BUTTON_HEIGHT;
        y = ui->yOffset + JANUS_QUICKUI_PADDING + JANUS_QUICKUI_BUTTONS_Y_OFFSET + ((h+JANUS_QUICKUI_PADDING) * (i % (JANUS_QUICKUI_MAX_OPTIONS/2)));
        /* UI Layout Styles*/
        if(ui->layoutStyle == STACK) {
            x = ui->xOffset + JANUS_QUICKUI_PADDING + ((i < JANUS_QUICKUI_MAX_OPTIONS/2) ? 0 : gfx_GetStringWidth(ui->options[i-(JANUS_QUICKUI_MAX_OPTIONS/2)]) + JANUS_QUICKUI_PADDING*4);
        } else if(ui->layoutStyle == UNIFORM) {
            x = ui->xOffset + JANUS_QUICKUI_PADDING + ((i < JANUS_QUICKUI_MAX_OPTIONS/2) ? 0 : largestWidthFirstColumn + (JANUS_QUICKUI_PADDING*4));
        } else if(ui->layoutStyle == ELASTIC) {
            x = ui->xOffset + JANUS_QUICKUI_PADDING;
            if(i >= JANUS_QUICKUI_MAX_OPTIONS/2) {
                uint24_t avg = 0;
                uint8_t j;
                for(j = janus_Max(i-1,JANUS_QUICKUI_MAX_OPTIONS/2); j <= janus_Min(i + 1,JANUS_QUICKUI_MAX_OPTIONS-1); j++) {
                    avg += gfx_GetStringWidth(ui->options[j-(JANUS_QUICKUI_MAX_OPTIONS/2)]);
                }
                x += avg/3 + largestWidthFirstColumn/2 + (JANUS_QUICKUI_PADDING*4);
            }
        }
        /* End UI Layout Styles */
        if(i < JANUS_QUICKUI_MAX_OPTIONS/2 && w > largestWidthFirstColumn && ui->layoutStyle != STACK) {
            largestWidthFirstColumn = w;
        }

        gfx_SetColor(ui->backgroundColor);
        selecting = ui->cursorPosition == i && kb_Data[6] & kb_Enter;
        if(ui->cursorPosition == i && !selecting) {
            //hovering button
            gfx_FillRectangle_NoClip(x - JANUS_QUICKUI_HOVER_OFFSET,y - JANUS_QUICKUI_HOVER_OFFSET,w,h);
            gfx_SetColor(ui->foregroundColor);
            gfx_FillRectangle_NoClip(x,y+h-JANUS_QUICKUI_HOVER_OFFSET,w,JANUS_QUICKUI_HOVER_OFFSET);
            gfx_FillRectangle_NoClip(x+w-JANUS_QUICKUI_HOVER_OFFSET,y,JANUS_QUICKUI_HOVER_OFFSET,h);
            gfx_Rectangle_NoClip(x - JANUS_QUICKUI_HOVER_OFFSET,y - JANUS_QUICKUI_HOVER_OFFSET,w,h);
            gfx_PrintStringXY(ui -> options[i],x + JANUS_QUICKUI_PADDING - JANUS_QUICKUI_HOVER_OFFSET,y+JANUS_QUICKUI_TEXT_Y_OFFSET - JANUS_QUICKUI_HOVER_OFFSET);
        } else {
            if(selecting) {
                //selecting button
                gfx_SetColor(ui->selectingColor);
            }
            gfx_FillRectangle_NoClip(x,y,w,h);
            gfx_SetColor(ui->foregroundColor);
            gfx_Rectangle_NoClip(x,y,w,h);
            gfx_PrintStringXY(ui -> options[i],x + JANUS_QUICKUI_PADDING,y + JANUS_QUICKUI_TEXT_Y_OFFSET);
        }
    }
}

void janus_Screenshake(uint24_t magnitudeX,uint24_t magnitudeY, uint24_t length) {
    uint8_t i;
    uint24_t magnitudeXOriginal,magnitudeYOriginal;
    magnitudeXOriginal = magnitudeX;
    magnitudeYOriginal = magnitudeY;
    for (i = 0; i < length; i++)
    {
        uint8_t l = randInt(1, 4);
        switch (l)
        {
        case 1:
            gfx_ShiftUp(magnitudeY);
            break;
        case 2:
            gfx_ShiftRight(magnitudeX);
            break;
        case 3:
            gfx_ShiftDown(magnitudeY);
            break;
        case 4:
            gfx_ShiftLeft(magnitudeX);
            break;
        }
        magnitudeX -= magnitudeXOriginal / length;
        magnitudeX -= magnitudeYOriginal / length;
        gfx_SwapDraw();
    }
}
