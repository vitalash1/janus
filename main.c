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

void renderFps(float elapsedTime);

int main(void)
{
    struct janus_Animation animation = {QUAD_IN_OUT,2,0,true,50,170,250,20,50,50};
    uint24_t x = 50;

    gfx_Begin();

    while(!(kb_Data[6] & kb_Clear)) {
        float elapsed = janus_GetElapsedTime();

        kb_Scan();
        janus_UpdateDebouncedKeys();
        janus_UpdateAnimation(&animation,elapsed);

        if(animation.progress == animation.length || animation.progress == 0) {
            animation.reverse = !animation.reverse;
        }

        if(janus_PressedKeys[7] & kb_Right) {
            x+=10;
        }
        
        if(janus_PressedKeys[7] & kb_Left) {
            x-=10;
        }

        gfx_FillScreen(0xFF);
        gfx_SetColor(0x0F);
        gfx_FillRectangle(x,50,10,10);
        gfx_SetColor(0xF0);
        gfx_FillRectangle(animation.currentX,animation.currentY,10,10);

        renderFps(elapsed);

        gfx_SwapDraw();
    }

    gfx_End();

    return 0;
}

void renderFps(float elapsedTime) {
    char* text = "FPS: 00";
    if(elapsedTime < 1.0f/99)
        elapsedTime = 1.0f/99;
    sprintf(text,"FPS: %d",(uint8_t)(1/elapsedTime));
    gfx_PrintStringXY(text,5,5);
}
