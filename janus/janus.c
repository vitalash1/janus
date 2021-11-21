
#include <tice.h>
#include <keypadc.h>
#include <graphx.h>
#include <time.h>

#include <debug.h>
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

struct janus_Vector2 janus_AddVectors(struct janus_Vector2* x, struct janus_Vector2* y) {
    struct janus_Vector2 out;
    out.x = x->x + y->x;
    out.y = x->y + y->y;
    return out;
}

struct janus_Vector2 janus_SubtractVectors(struct janus_Vector2* x, struct janus_Vector2* y) {
    struct janus_Vector2 out;
    out.x = x->x - y->x;
    out.y = x->y - y->y;
    return out;
}

struct janus_Vector2 janus_MultiplyVectors(struct janus_Vector2* x, struct janus_Vector2* y) {
    return (struct janus_Vector2) {x->x * y->x, x->y * y->y};
}

struct janus_Vector2 janus_DivideVectors(struct janus_Vector2* x, struct janus_Vector2* y) {
    return (struct janus_Vector2) {x->x / y->x, x->y / y->y};
}

int24_t janus_SmallestOfFour(int24_t a, int24_t b, int24_t c, int24_t d) {
    int24_t min_ab, min_cd;
    min_ab = a < b ? a : b;
    min_cd = c < d ? c : d;
    return min_ab < min_cd ? min_ab : min_cd;
}
int24_t janus_LerpInt24(int24_t x, int24_t y, float alpha) {
    return (x+(y-x)*alpha);
}
int24_t janus_LerpInt24ByInt(int24_t x, int24_t y, int24_t alpha) {
    return x + JANUS_APPLY_DELTA_TIME((y - x),alpha);
}
int24_t janus_Pow(int24_t x, int24_t y) {
    return y > 0 ? x*janus_Pow(x,y-1) : 1;
}

float janus_Sin(float x) {
    return (16.0f*x*(JANUS_PI - x)) / ((5.0f*JANUS_PI*JANUS_PI) - (4.0f*x*(JANUS_PI-x)));
}



bool janus_Collision(struct janus_Rect* object1, struct janus_Rect* object2) {
    return object1->position.x + object1->size.x > object2->position.x && object1->position.y + object1->size.y > object2->position.y && object2->position.y + object2->size.y > object1->position.y && object2->position.x + object2->size.x > object1->position.x;
}

bool janus_HandleObjectObjectCollision(struct janus_PhysicsObject* object1, struct janus_PhysicsObject* object2) {
    bool onFloor = false;
    if(!janus_Collision(&object1->rect,&object2->rect)) {
        return false;
    }
    int24_t obj1RightWall = object1->rect.position.x + object1->rect.size.x;
    int24_t obj2RightWall = object2->rect.position.x + object2->rect.size.x;
    int24_t obj1BottomWall = object1->rect.position.y + object1->rect.size.y;
    int24_t obj2BottomWall = object2->rect.position.y + object2->rect.size.y;
    
    int24_t pushObj1Right = (obj2RightWall - object1->rect.position.x);
    int24_t pushObj1Left = -(obj1RightWall - object2->rect.position.x);
    int24_t pushObj1Down = (obj2BottomWall - object1->rect.position.y);
    int24_t pushObj1Up = -(obj1BottomWall - object2->rect.position.y);

    int24_t smallestMovement = janus_SmallestOfFour(JANUS_ABS(pushObj1Right),JANUS_ABS(pushObj1Left),JANUS_ABS(pushObj1Down),JANUS_ABS(pushObj1Up));

    int24_t object1Influence = object1->anchored ? 0 : object1->resistance;
    int24_t object2Influence = object2->anchored ? 0 : object2->resistance;
    int24_t totalInfluence = object1Influence + object2Influence;

    if(smallestMovement == (JANUS_ABS(pushObj1Right))) {
        int24_t amt1 = pushObj1Right * object1Influence / totalInfluence;
        int24_t amt2 = pushObj1Right * object2Influence / totalInfluence;
        object1->rect.position.x += amt1;
        object2->rect.position.x -= amt2;
        object1->velocity.x -= ((object1->velocity.x) < 0 ? -1 : 0) * janus_Min(JANUS_ABS(amt1),JANUS_ABS(object1->velocity.x));
        object2->velocity.x -= ((object2->velocity.x) < 0 ? -1 : 0) * janus_Min(JANUS_ABS(amt2),JANUS_ABS(object2->velocity.x));
    } else if(smallestMovement == JANUS_ABS(pushObj1Left)) {
        int24_t amt1 = pushObj1Left * object1Influence / totalInfluence;
        int24_t amt2 = pushObj1Left * object2Influence / totalInfluence;
        object1->rect.position.x += amt1;
        object2->rect.position.x -= amt2;
        object1->velocity.x -= ((object1->velocity.x) < 0 ? -1 : 0) * janus_Min(JANUS_ABS(amt1),JANUS_ABS(object1->velocity.x));
        object2->velocity.x -= ((object2->velocity.x) < 0 ? -1 : 0) * janus_Min(JANUS_ABS(amt2),JANUS_ABS(object2->velocity.x));
    } else if(smallestMovement == JANUS_ABS(pushObj1Up)) {
        int24_t amt1 = pushObj1Up * object1Influence / totalInfluence;
        int24_t amt2 = pushObj1Up * object2Influence / totalInfluence;
        object1->rect.position.y += amt1;
        object2->rect.position.y -= amt2;
        object1->velocity.y -= ((object1->velocity.y) < 0 ? -1 : 0) * janus_Min(JANUS_ABS(amt1),JANUS_ABS(object1->velocity.y));
        object2->velocity.y -= ((object2->velocity.y) < 0 ? -1 : 0) * janus_Min(JANUS_ABS(amt2),JANUS_ABS(object2->velocity.y));
        onFloor = true;
    } else if(smallestMovement == JANUS_ABS(pushObj1Down)) {
        int24_t amt1 = pushObj1Down * object1Influence / totalInfluence;
        int24_t amt2 = pushObj1Down * object2Influence / totalInfluence;
        object1->rect.position.y += amt1;
        object2->rect.position.y -= amt2;
        object1->velocity.y -= ((object1->velocity.y) < 0 ? -1 : 0) * janus_Min(JANUS_ABS(amt1),JANUS_ABS(object1->velocity.y));
        object2->velocity.y -= ((object2->velocity.y) < 0 ? -1 : 0) * janus_Min(JANUS_ABS(amt2),JANUS_ABS(object2->velocity.y));
    }
    return onFloor;
}

bool janus_HandleObjectRectCollision(struct janus_PhysicsObject* object1, struct janus_Rect* rect) {
    bool onFloor = false;
    if(!janus_Collision(&object1->rect,rect)) {
        return false;
    }
    int24_t obj1RightWall = object1->rect.position.x + object1->rect.size.x;
    int24_t obj2RightWall = rect->position.x + rect->size.x;
    int24_t obj1BottomWall = object1->rect.position.y + object1->rect.size.y;
    int24_t obj2BottomWall = rect->position.y + rect->size.y;
    
    int24_t pushObj1Right = (obj2RightWall - object1->rect.position.x);
    int24_t pushObj1Left = -(obj1RightWall - rect->position.x);
    int24_t pushObj1Down = (obj2BottomWall - object1->rect.position.y);
    int24_t pushObj1Up = -(obj1BottomWall - rect->position.y);

    int24_t smallestMovement = janus_SmallestOfFour(JANUS_ABS(pushObj1Right),JANUS_ABS(pushObj1Left),JANUS_ABS(pushObj1Down),JANUS_ABS(pushObj1Up));

    if(smallestMovement == (JANUS_ABS(pushObj1Right))) {
        object1->rect.position.x += pushObj1Right;
        object1->velocity.x = 0;
    } else if(smallestMovement == JANUS_ABS(pushObj1Left)) {
        object1->rect.position.x += pushObj1Left;
        object1->velocity.x = 0;
    } else if(smallestMovement == JANUS_ABS(pushObj1Up)) {
        object1->rect.position.y += pushObj1Up;
        object1->velocity.y = 0;
        onFloor = true;
    } else if(smallestMovement == JANUS_ABS(pushObj1Down)) {
        object1->rect.position.y += pushObj1Down;
        object1->velocity.y = 0;
    }
    return onFloor;
}


void janus_AddForce(struct janus_PhysicsObject* object, struct janus_Vector2* velocity) { 
    object->velocity = janus_AddVectors(&object->velocity,velocity);
}

void janus_DampenVelocity(struct janus_PhysicsObject* object, struct janus_Vector2* dampen) {
    struct janus_Vector2 divided = janus_DivideVectors(&object->velocity,dampen);
    object->velocity = janus_SubtractVectors(&object->velocity,&divided);
}

void janus_ApplyVelocity(struct janus_PhysicsObject* object, int24_t deltaMS) {
    struct janus_Vector2 delta = (struct janus_Vector2){JANUS_APPLY_DELTA_TIME(object->velocity.x,deltaMS),JANUS_APPLY_DELTA_TIME(object->velocity.y,deltaMS)};
    object->rect.position = janus_AddVectors(&object->rect.position,&delta);
    object->velocity = janus_SubtractVectors(&object->velocity,&delta);
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

int24_t janus_GetDeltaTime(void) {
    static clock_t previousTime;
    clock_t currentTime = clock();
    int24_t output = 0;

    output = (currentTime - previousTime) * 1000 / CLOCKS_PER_SEC;

    previousTime = currentTime;
    return output;
}

int24_t janus_GetFPS(int24_t msElapsed) {
    return 1000 / msElapsed;
}



//https://easings.net/ for reference
uint24_t janus_GetEaseProgress(enum janus_EasingMode easingMode, uint24_t alpha) {
    switch(easingMode) {
        case LINEAR:
            return alpha;
        case QUAD_IN:
            return JANUS_APPLY_DELTA_TIME(alpha,alpha);
        case QUAD_OUT:
            return JANUS_ONE_SECOND - JANUS_APPLY_DELTA_TIME((JANUS_ONE_SECOND - alpha),(JANUS_ONE_SECOND - alpha));
        case QUAD_IN_OUT:
            return alpha <= JANUS_ONE_SECOND/2 ? (2 * JANUS_APPLY_DELTA_TIME(alpha,alpha)) : (1000 - JANUS_APPLY_DELTA_TIME((-2 * alpha + 2000),(-2 * alpha + 2000)) / 2);
    }
}
void janus_UpdateEase(struct janus_Ease* ease, int24_t msElapsed) {
    int24_t alpha;

    if(msElapsed == 0) {
        return;
    }
    
    ease->progress = ease->progress + (ease->reverse ? -msElapsed : msElapsed);
    if(ease->progress > ease->length) {
        ease->progress = ease->length;
    }
    if(ease->progress < 0) {
        ease->progress = 0;
    }
    alpha = janus_GetEaseProgress(ease->easingMode,ease->progress*1000/ease->length);
    if(ease->fromX != ease->toX) {
        ease->currentX = janus_LerpInt24ByInt(ease->fromX,ease->toX,alpha);
    } else {
        ease->currentX = ease->toX;
    }
    if(ease->fromY != ease->toY) {
        ease->currentY = janus_LerpInt24ByInt(ease->fromY,ease->toY,alpha);
    } else {
        ease->currentY = ease->toY;
    }
}

void janus_UpdateAnimation(struct janus_Animation* animation, int24_t msElapsed) {
    float maxLength = 0;
    animation->msElapsed += msElapsed;
    if(animation->useVariableTimings) {
        for(uint24_t i = 0; i < animation->frameCount; i++) {
            maxLength += animation->frameTimings.variableTimings[i];
        }
    } else {
        maxLength = animation->frameCount * animation->frameTimings.constantTiming;
    }
    if(animation->msElapsed >= maxLength) {
        if(animation->loop) {
            animation->msElapsed -= maxLength;
        } else {
            animation->currentFrame = animation->frameCount-1;
            return;
        }
    }

    if(animation->useVariableTimings) {
        uint24_t counterMS = 0;
        for(uint24_t i = 0; i < animation->frameCount; i++) {
            counterMS += animation->frameTimings.variableTimings[i];
            if(counterMS >= animation->msElapsed) {
                animation->currentFrame = i;
                break;
            }
        }
    } else {
        animation->currentFrame = animation->msElapsed / animation->frameTimings.constantTiming;
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
