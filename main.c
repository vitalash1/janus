/*
 *--------------------------------------
 * Program Name: Janus Library Demo
 * Author: slimeenergy
 * License: GNU General Public License v3.0
 * Description: test program
 *--------------------------------------
*/

#define JANUS_ANIMATION

#include <tice.h>
#include <graphx.h>
#include <keypadc.h>

#include "janus/janus.h"
#include "gfx/gfx.h"

#define SCENE_MAIN_DEMO 0
#define SCENE_EASING_DEMO 1
#define SCENE_ANIMATION_DEMO 2
#define SCENE_DEBOUNCING_DEMO 3

#define PLAYER_SPEED 10

#define PLAYER_STARTING_X LCD_WIDTH/2 + 50
#define PLAYER_STARTING_Y LCD_HEIGHT/2

struct Demo
{
    uint24_t playerX;
    uint8_t playerY;
    uint8_t scene;
    uint8_t lastSelected;
    struct janus_Ease ease;
    struct janus_QuickUI ui;
    struct janus_Animation animation;
};

void renderFps(float elapsedTime);

void update(struct Demo* demo, float elapsedTime);
void render(struct Demo* demo, float elapsedTime);





int main(void)
{

    struct Demo demo = {
        PLAYER_STARTING_X,PLAYER_STARTING_Y,0,0,
        /* Ease */
        {QUAD_IN_OUT,true,50,170,250,20,50,50,2,0},
        /* QuickUI */
        {ELASTIC,"Janus Library Demo",0,0,0xFF,0x01,0xEC,0,4,{"All Demos","Easing Demo","Animation Demo","Debouncing Demo"}},
        /* Animation */
        {
            12,{wizard_tile_0,wizard_tile_1,wizard_tile_2,wizard_tile_3,wizard_tile_4,wizard_tile_5,wizard_tile_6,wizard_tile_7,wizard_tile_8,wizard_tile_9,wizard_tile_10,wizard_tile_11},
            true,{2.0f,0.1f,1.0f,0.1f,0.1f,0.1f,0.5f,0.1f,0.1f,0.1f,0.2f,0.2f},
            0.0f,true,0.0f,0
        }
    };

    srand(rtc_Time());

    gfx_Begin();

    gfx_SetPalette(wizardpalette,sizeof_wizardpalette,0);
    gfx_SetTransparentColor(0);

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
    if(demo->scene == SCENE_MAIN_DEMO || demo->scene == SCENE_DEBOUNCING_DEMO) {

        if(janus_PressedKeys[7] & kb_Right) {
            demo->playerX += PLAYER_SPEED;
        }
        if(janus_PressedKeys[7] & kb_Left) {
            demo->playerX -= PLAYER_SPEED;
        }
        if(janus_PressedKeys[7] & kb_Up) {
            demo->playerY -= PLAYER_SPEED;
        }
        if(janus_PressedKeys[7] & kb_Down) {
            demo->playerY += PLAYER_SPEED;
        }

    }

    /* Easing Demo */
    if(demo->scene == SCENE_MAIN_DEMO || demo->scene == SCENE_EASING_DEMO) {

        if(demo->ease.progress == demo->ease.length || demo->ease.progress == 0) {
            // Reverse ease when it reaches an end
            demo->ease.reverse = !demo->ease.reverse;
        }
        janus_UpdateEase(&demo->ease,elapsedTime);

    }

    /* UI Demo */
    selected = janus_UpdateQuickUI(&demo->ui);
    
    if(selected) {
        demo->lastSelected = demo->ui.cursorPosition;
        demo->scene = demo->ui.cursorPosition;
        demo->playerX = PLAYER_STARTING_X;
        demo->playerY = PLAYER_STARTING_Y;
    }

    /* Animation Demo */
    if(demo->scene == SCENE_MAIN_DEMO || demo->scene == SCENE_ANIMATION_DEMO) {
        janus_UpdateAnimation(&demo->animation,elapsedTime);
    }

}
void render(struct Demo* demo, float elapsedTime) {
    /* Clear Screen */
    gfx_FillScreen(0x18);
    /* Debouncing Demo */
    if(demo->scene == SCENE_MAIN_DEMO || demo->scene == SCENE_DEBOUNCING_DEMO) {
        gfx_SetColor(0x0F);
        gfx_FillRectangle(demo->playerX,demo->playerY,10,10);
        if(demo->scene == SCENE_DEBOUNCING_DEMO) {
            gfx_SetTextFGColor(0x01);
            gfx_PrintStringXY("USE ARROWS",5,LCD_HEIGHT-24);
        }
    }
    /* Easing Demo */
    if(demo->scene == SCENE_MAIN_DEMO || demo->scene == SCENE_EASING_DEMO) {
        gfx_SetColor(0xF0);
        gfx_FillRectangle(demo->ease.currentX,demo->ease.currentY,10,10);
    }
    /* FPS Demo */
    renderFps(elapsedTime);
    /* UI Demo */
    janus_RenderQuickUI(&demo->ui);
    gfx_PrintStringXY(demo->ui.options[demo->lastSelected],5,230);
    /* Animation Demo */
    if(demo->scene == SCENE_MAIN_DEMO || demo->scene == SCENE_ANIMATION_DEMO) {
        char* frame = "12";
        gfx_ScaledTransparentSprite_NoClip(janus_GetAnimationFrame(&demo->animation),LCD_WIDTH/2-wizard_tile_0->width*2,LCD_HEIGHT-(wizard_tile_1->height*4),4,4);
        
        sprintf(frame,"%d",demo->animation.currentFrame);
        gfx_SetTextFGColor(0x1B);
        gfx_PrintStringXY(frame,LCD_WIDTH/2-wizard_tile_0->width,LCD_HEIGHT-wizard_tile_1->height*4-10);
    }
}

void renderFps(float elapsedTime) {
    char* text = "FPS: 00";
    if(elapsedTime < 1.0f/99)
        elapsedTime = 1.0f/99;
    sprintf(text,"FPS: %d",(uint8_t)(1/elapsedTime));
    gfx_SetTextFGColor(0x01);
    gfx_PrintStringXY(text,270,10);
}
