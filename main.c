/*
 *--------------------------------------
 * Program Name: Janus Library Demo
 * Author: slimeenergy
 * License: GNU General Public License v3.0
 * Description: test program
 *--------------------------------------
*/
#include <tice.h>
#include <graphx.h>
#include <keypadc.h>

#include "janus/janus.h"

struct Demo
{
    uint24_t playerX;
    uint8_t scene;
    uint8_t lastSelected;
    struct janus_Animation animation;
    struct janus_QuickUI ui;
};

void renderFps(float elapsedTime);

void update(struct Demo* demo, float elapsedTime);
void render(struct Demo* demo, float elapsedTime);





int main(void)
{
    struct Demo demo = {
        0,0,0,
        /* Animation */
        {QUAD_IN_OUT,true,50,170,250,20,50,50,2,0},
        /* QuickUI */
        {ELASTIC,"Janus Library Demo",0,0,0xFF,0x00,0xEC,0,3,{"Play","Options","Credits"}}
    };

    srand(rtc_Time());

    gfx_Begin();

    while(!(kb_Data[6] & kb_Clear)) {
        float elapsedTime = janus_GetElapsedTime();
        kb_Scan();
        //janus_UpdateDebouncedKeys can be called every frame just after kb_Scan is
        janus_UpdateDebouncedKeys();

        update(&demo,elapsedTime);
        render(&demo,elapsedTime);

        gfx_SwapDraw();
    }

    gfx_End();

    return 0;
}

void update(struct Demo* demo, float elapsedTime) {
    bool selected;
    /* Debouncing Demo */
    if(janus_PressedKeys[7] & kb_Right) {
        demo->playerX += 10;
    }
    
    if(janus_PressedKeys[7] & kb_Left) {
        demo->playerX -= 10;
    }
    /* Animation Easing Demo*/

    if(demo->animation.progress == demo->animation.length || demo->animation.progress == 0) {
        // Reverse animation when it reaches an end
        demo->animation.reverse = !demo->animation.reverse;
    }

    janus_UpdateAnimation(&demo->animation,elapsedTime);
    /* UI Demo */
    selected = janus_UpdateQuickUI(&demo->ui);
    
    if(selected) {
        demo->lastSelected = demo->ui.cursorPosition;
        janus_Screenshake(4,4,2);
    }
}
void render(struct Demo* demo, float elapsedTime) {
    /* Clear Screen */
    gfx_FillScreen(0xFF);
    /* Debouncing/Animation Easing Demo */
    gfx_SetColor(0x0F);
    gfx_FillRectangle(demo->playerX,50,10,10);
    /* Animation Easing Demo*/
    gfx_SetColor(0xF0);
    gfx_FillRectangle(demo->animation.currentX,demo->animation.currentY,10,10);
    /* FPS Demo */
    renderFps(elapsedTime);
    /* UI Demo */
    janus_RenderQuickUI(&demo->ui);
    gfx_PrintStringXY("Last Selected Option",5,220);
    gfx_PrintStringXY(demo->ui.options[demo->lastSelected],5,230);
}

void renderFps(float elapsedTime) {
    char* text = "FPS: 00";
    if(elapsedTime < 1.0f/99)
        elapsedTime = 1.0f/99;
    sprintf(text,"FPS: %d",(uint8_t)(1/elapsedTime));
    gfx_PrintStringXY(text,270,10);
}
