#ifndef JANUS_LIB
#define JANUS_LIB

#ifdef __cplusplus
extern "C" {
#endif

/* Allows us to index janus_ReleasedKeys, janus_PressedKeys, and janus_PressedOrReleasedKeys just as we would index kb_Data from keypadc.h */
#define KB_DATA_SIZE 8

/* Timer macros */
#define JANUS_TIMER_NUMBER 1
#define JANUS_TIMER_DIRECTION TIMER_UP

extern uint8_t janus_ReleasedKeys[KB_DATA_SIZE];
extern uint8_t janus_PressedKeys[KB_DATA_SIZE];
extern uint8_t janus_PressedOrReleasedKeys[KB_DATA_SIZE];

enum janus_AnimationEasingMode {
    LINEAR,
    QUAD_IN,
    QUAD_OUT,
    QUAD_IN_OUT
};
struct janus_Animation {
    enum janus_AnimationEasingMode easingMode;
    float length; //How long the animation should be in seconds.
    float progress; //How far (0-1) we are in the animation
    bool reverse;
    int24_t fromX;
    int24_t fromY;
    int24_t toX;
    int24_t toY;
    int24_t currentX;
    int24_t currentY;
};

/**
 * Updates keys based on whether or not they were just pressed or released.
 * janus_ReleasedKeys, janus_PressedKeys, or janus_PressedOrReleasedKeys can be indexed just as kb_Data can be.
 * @note make sure kb_Scan is called before calling this function
**/
void janus_UpdateDebouncedKeys(void);

/** 
 * Gives time (in seconds) since last time the function was called.
 * If not called previously, the function sets a 32k Hz timer in Timer *1* (https://ce-programming.github.io/toolchain/master/headers/tice.html#c.timer_Enable)
 * and returns zero-seconds.
 * @returns float containing time since last function call (in seconds)
**/
float janus_GetElapsedTime(void);

/**
 * Linearly interpolates between X and Y using alpha for the percentage between (0-1)
 * 
 * @param x Interpolate from coordinate
 * @param y Interpolate to coordinate
 * @param alpha How far (0 to 1) between from and to?
 * @returns integer interpolated between X and Y using alpha
**/ 
int24_t janus_LerpInt24(int24_t x, int24_t y, float alpha);

/**
 * Adds elapsedTime to the animation pointer's progress and updates the animation's currentX and currentY based on the animation's easing mode
 * 
 * @param animation A pointer to the animation object to update
 * @param elapsedTime The time to add to the animation progress
**/
void janus_UpdateAnimation(struct janus_Animation* animation, float elapsedTime);

#ifdef __cplusplus
}
#endif

#endif