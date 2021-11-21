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
#include <debug.h>

#include "janus/janus.h"
#include "gfx/gfx.h"

#define SCENE_MAIN_DEMO 0
#define SCENE_EASING_DEMO 1
#define SCENE_ANIMATION_DEMO 2
#define SCENE_DEBOUNCING_DEMO 3
#define SCENE_PHYSICS_DEMO 4

#define PLAYER_SPEED 70
#define PLAYER_JUMP -350

#define PLAYER_STARTING_X LCD_WIDTH/2 + 50
#define PLAYER_STARTING_Y LCD_HEIGHT/2

#define WALL_COUNT 2

struct Demo
{
    uint24_t playerX;
    uint8_t playerY;
    uint8_t scene;
    uint8_t lastSelected;

    struct janus_Ease ease;

    struct janus_QuickUI ui;

    struct janus_Animation animation;

    struct janus_PhysicsObject object1;
    struct janus_PhysicsObject object2;
    struct janus_Rect walls[WALL_COUNT];
};

void renderFps(int24_t deltaMS);

void update(struct Demo* demo, int24_t deltaMS);
void render(struct Demo* demo, int24_t deltaMS);





int main(void)
{
    struct Demo demo = {
        .playerX = PLAYER_STARTING_X, .playerY = PLAYER_STARTING_Y, .scene = 0, .lastSelected = 0,
        /* Ease */
        .ease = {QUAD_IN_OUT,true,50,170,250,20,50,50,2000,0},
        /* QuickUI */
        .ui = {ELASTIC,"Janus Library Demo",0,0,0xFF,0x01,0xEC,0,5,{"All Demos","Easing Demo","Animation Demo","Debouncing Demo","Physics Demo"}},
        /* Animation */
        .animation = {
            .frameCount = 12, .frames = {wizard_tile_0,wizard_tile_1,wizard_tile_2,wizard_tile_3,wizard_tile_4,wizard_tile_5,wizard_tile_6,wizard_tile_7,wizard_tile_8,wizard_tile_9,wizard_tile_10,wizard_tile_11},
            JANUS_ANIMATION_INIT_VARIABLE_TIMING(1000,100,100,100,100,100,500,100,100,100,150,200),
            .loop = true,
            /*these can just be set to zero: */ .msElapsed = 0, .currentFrame = 0
        },
        /* Physics */
        .object1 = {
            .rect = {{10,80},{40,40}},
            .velocity = {0,0},
            .resistance = 1,
            .anchored = false
        },
        .object2 = {
            .rect = {{100,30},{40,40}},
            .velocity = {0,0},
            .resistance = 5,
            .anchored = false
        },
        .walls = {
            {.position = {0,140}, .size = {200,10}},
            {.position = {120,70}, .size = {10,70}}
        }
    };

    srand(rtc_Time());

    gfx_Begin();

    gfx_SetPalette(wizardpalette,sizeof_wizardpalette,0);
    gfx_SetTransparentColor(0);

    while(!(kb_Data[6] & kb_Clear)) {
        int24_t deltaMS = janus_GetDeltaTime();
        kb_Scan();
        //janus_UpdateDebouncedKeys can be called every frame just after kb_Scan is
        janus_UpdateDebouncedKeys();

        update(&demo,deltaMS);
        render(&demo,deltaMS);

        gfx_SwapDraw();
    }

    gfx_End();

    return 0;
}

void update(struct Demo* demo, int24_t deltaMS) {
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
        janus_UpdateEase(&demo->ease,deltaMS);

    }

    /* UI Demo */
    selected = janus_UpdateQuickUI(&demo->ui);
    
    if(selected) {
        demo->lastSelected = demo->ui.cursorPosition;
        demo->scene = demo->ui.cursorPosition;
        demo->playerX = PLAYER_STARTING_X;
        demo->playerY = PLAYER_STARTING_Y;
        demo->animation.msElapsed = 0;
    }

    /* Animation Demo */
    if(demo->scene == SCENE_MAIN_DEMO || demo->scene == SCENE_ANIMATION_DEMO) {
        janus_UpdateAnimation(&demo->animation,deltaMS);
    }

    /* Physics */
    if(demo->scene == SCENE_MAIN_DEMO || demo->scene == SCENE_PHYSICS_DEMO) {
        bool ableToJump = false;
        uint8_t i;

        // Apply velocities to positions (must do this for every physics object!)
        // I'm doing this to apply all velocity changes from the previous frame.
        janus_ApplyVelocity(&demo->object1, deltaMS);
        janus_ApplyVelocity(&demo->object2, deltaMS);

        // Gravity
        janus_AddForce(&demo->object1,&(struct janus_Vector2) {0,7});
        janus_AddForce(&demo->object2,&(struct janus_Vector2) {0,7});

        for(i = 0; i < WALL_COUNT; i++) {
            // Handle collisions for object 1, the player. If the result is true, that means the player is on the floor and we should be able to jump
            if(janus_HandleObjectRectCollision(&demo->object1,&demo->walls[i]))
                ableToJump = true;
            // Handle collisions for object 2, the physics object that is not the player
            janus_HandleObjectRectCollision(&demo->object2,&demo->walls[i]);
        }

        // Handle collisions between object 1 and object 2. If the result is true, that means the first argument, object 1, is pushing down on top of object 2. This means we can jump.
        if(janus_HandleObjectObjectCollision(&demo->object1,&demo->object2))
            ableToJump = true;
        
        // Left/Right Movement for Object 1, the player 
        if(kb_Data[7] & kb_Right) {
            janus_AddForce(&demo->object1,&(struct janus_Vector2){PLAYER_SPEED,0});
        }
        if(kb_Data[7] & kb_Left) {
            janus_AddForce(&demo->object1,&(struct janus_Vector2){-PLAYER_SPEED,0});
        }

        // Jumping for Object 1, the player
        if(kb_Data[7] & kb_Up && ableToJump) {
            janus_AddForce(&demo->object1,&(struct janus_Vector2){0,PLAYER_JUMP});
        }

        janus_DampenVelocity(&demo->object1,&(struct janus_Vector2){3,32});
        janus_DampenVelocity(&demo->object2,&(struct janus_Vector2){3,32});
    }

}
void render(struct Demo* demo, int24_t deltaMS) {
    /* Clear Screen */

    gfx_FillScreen(0x18);

    /* Debouncing Demo */
    if(demo->scene == SCENE_MAIN_DEMO || demo->scene == SCENE_DEBOUNCING_DEMO) {
        gfx_SetColor(0x0F);
        gfx_FillRectangle(demo->playerX,demo->playerY,10,10);
        if(demo->scene == SCENE_DEBOUNCING_DEMO) {
            gfx_SetTextFGColor(0xEF);
            gfx_PrintStringXY("USE ARROWS",5,LCD_HEIGHT-24);
        }
    }

    /* Easing Demo */
    if(demo->scene == SCENE_MAIN_DEMO || demo->scene == SCENE_EASING_DEMO) {
        gfx_SetColor(0xF0);
        gfx_FillRectangle(demo->ease.currentX,demo->ease.currentY,10,10);
    }

    /* FPS Demo */
    renderFps(deltaMS);

    /* Animation Demo */
    
    if(demo->scene == SCENE_MAIN_DEMO || demo->scene == SCENE_ANIMATION_DEMO) {
        char* frame = "12";
        gfx_ScaledTransparentSprite_NoClip(janus_GetAnimationFrame(&demo->animation),LCD_WIDTH/2-wizard_tile_0->width*2,LCD_HEIGHT-(wizard_tile_1->height*4),4,4);
        
        sprintf(frame,"%d",demo->animation.currentFrame);
        gfx_SetTextFGColor(0x1B);
        gfx_PrintStringXY(frame,LCD_WIDTH/2-wizard_tile_0->width,LCD_HEIGHT-wizard_tile_1->height*4-10);
    }
    /* Physics Demo */

    if(demo->scene == SCENE_MAIN_DEMO || demo->scene == SCENE_PHYSICS_DEMO) {
        uint8_t i;
        gfx_SetColor(0xF0);
        gfx_FillRectangle(demo->object1.rect.position.x,demo->object1.rect.position.y,demo->object1.rect.size.x,demo->object1.rect.size.y);
        gfx_SetColor(0xEF);
        gfx_FillRectangle(demo->object2.rect.position.x,demo->object2.rect.position.y,demo->object2.rect.size.x,demo->object2.rect.size.y);
        for(i = 0; i < WALL_COUNT; i++) {
            gfx_SetColor(0xCA);
            gfx_FillRectangle(demo->walls[i].position.x,demo->walls[i].position.y,demo->walls[i].size.x,demo->walls[i].size.y);
        }
    }

    /* UI Demo */
    janus_RenderQuickUI(&demo->ui);
    gfx_SetTextFGColor(0xEF);
    gfx_PrintStringXY(demo->ui.options[demo->lastSelected],5,230);
}

void renderFps(int24_t deltaMS) {
    char* text = "FPS: 00";
    sprintf(text,"FPS: %d",janus_Min(99,1000/deltaMS));
    gfx_SetTextFGColor(0xEF);
    gfx_PrintStringXY(text,270,10);
}
