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

/* Quick UI macros */
#define JANUS_QUICKUI_MAX_OPTIONS 20
#define JANUS_QUICKUI_PADDING 5
#define JANUS_QUICKUI_HOVER_OFFSET 3
/* Height & Y positions */
#define JANUS_QUICKUI_BUTTON_HEIGHT 12
#define JANUS_QUICKUI_TEXT_Y_OFFSET 2
#define JANUS_QUICKUI_BUTTONS_Y_OFFSET 20

/* Animation Macros */
#define JANUS_ANIMATION_MAX_FRAMES 64

extern uint8_t janus_ReleasedKeys[KB_DATA_SIZE];
extern uint8_t janus_PressedKeys[KB_DATA_SIZE];
extern uint8_t janus_PressedOrReleasedKeys[KB_DATA_SIZE];

enum janus_EasingMode {
    LINEAR,
    QUAD_IN,
    QUAD_OUT,
    QUAD_IN_OUT
};

struct janus_Ease {
    enum janus_EasingMode easingMode; // LINEAR, QUAD_IN, QUAD_OUT, or QUAD_IN_OUT
    bool reverse; // Whether we are reversing the ease (To -> From instead of From -> To)
    int24_t fromX; // Start of ease X
    int24_t fromY; // Start of ease Y
    int24_t toX; // End of ease X
    int24_t toY; // End of ease Y
    int24_t currentX; // These values are animated when updating the ease using janus_UpdateEase(...)
    int24_t currentY; // These values are animated when updating the ease using janus_UpdateEase(...)
    float length; // How long the ease should be in seconds
    float progress; // How far, in seconds, we are in the ease
};

struct janus_Animation {
    uint24_t frameCount; // Number of frames
    gfx_sprite_t* frames[JANUS_ANIMATION_MAX_FRAMES]; // List of pointers to gfx_sprites
    bool useVariableTimings; // If true, each frame will have their own timing, specified in "variableTimings." Otherwise, each frame will have a constant framerate
    float variableTimings[JANUS_ANIMATION_MAX_FRAMES]; // Used if useVariableTimings is true - List should specify number of seconds for each respective frame
    float constantTiming; // Used if useVaraibleTimings is false - Each frame will last for this amount of time
    bool loop; // Whether or not to loop animation
    float currentElapsed; // Current progress (in seconds) in the animation; initialize at zero for most cases
    uint24_t currentFrame; // Current frame in the animation; initialize at zero for most cases
};

/* Determines whether the second column of elements follows the countour of the first or is all on the same X position */
enum janus_QuickUILayoutStyle {
    STACK,
    UNIFORM,
    ELASTIC
};
struct janus_QuickUI {
    enum janus_QuickUILayoutStyle layoutStyle; // STACK, UNIFORM, or ELASTIC
    char* title; // Title to be displayed above the options
    uint24_t xOffset; // Offset entire UI
    uint8_t yOffset; // Offset entire UI
    uint8_t backgroundColor; // Button background colors
    uint8_t foregroundColor; // Text and outline colors
    uint8_t selectingColor; // Color for when enter is held on an option
    uint8_t cursorPosition; // Position of cursor
    uint8_t numberOfOptions; //Number of options to be used (Don't exceed JANUS_QUICKUI_MAX_OPTIONS)
    char* options[JANUS_QUICKUI_MAX_OPTIONS]; //Array of option texts
};

/********************/
/* HELPER FUNCTIONS */
/********************/

/**
 * Returns the largest value of two inputs
 * 
 * @param x First input to compare
 * @param y Second input to compare
 * 
 * @returns X if X is largest; Y if Y is largest
**/
uint24_t janus_Max(uint24_t x, uint24_t y);

/**
 * Returns the smallest value of two inputs
 * 
 * @param x First input to compare
 * @param y Second input to compare
 * 
 * @returns X if X is smallest; Y if Y is smallest
**/
uint24_t janus_Min(uint24_t x, uint24_t y);

/**
 * Linearly interpolates between X and Y using alpha for the percentage between (0-1)
 * 
 * @param x Interpolate from coordinate
 * @param y Interpolate to coordinate
 * @param alpha How far (0 to 1) between from and to?
 * @returns integer interpolated between X and Y using alpha
**/ 
int24_t janus_LerpInt24(int24_t x, int24_t y, float alpha);



/***************************/
/* Debouncing & Delta Time */
/***************************/

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



/************************/
/* Easing and Animation */
/************************/

/**
 * Adds elapsedTime to the ease pointer's progress and updates the ease's currentX and currentY based on the easing mode
 * 
 * @param ease A pointer to the ease object (janus_Ease) to update
 * @param elapsedTime The time to add to the ease progress
**/
void janus_UpdateEase(struct janus_Ease* ease, float elapsedTime);

/**
 * Returns the progress of an easing mode based on alpha (0-1).
 * 
 * @param easingMode Takes a janus_EasingMode to determine what way to interpolate
 * @param alpha Takes a float between 0 (0%) and 1 (100%)
 * 
 * @returns float containing progress of ease
**/
float janus_GetEaseProgress(enum janus_EasingMode easingMode, float alpha);

/**
 * Updates an animation by setting the current frame/elapsed time.
 * 
 * @param animation Pointer to animation to update
 * @param elapsedTime Time since last call to update animation
**/ 
void janus_UpdateAnimation(struct janus_Animation* animation, float elapsedTime);

/**
 * Returns a pointer to the current frame of an animation
 * 
 * @param animation
 * 
 * @returns pointer to gfx_sprite_t
**/ 
gfx_sprite_t* janus_GetAnimationFrame(struct janus_Animation* animation);



/*************/
/* Quick UIs */
/*************/

/**
 * Updates a UI's cursor and selection
 * 
 * @param ui A pointer to a janus_QuickUI to update
 * @returns true if the user selected any options (use cursorPosition to find which one); returns false the user did not select any options.
 * 
 * @warning janus_UpdateDebouncedKeys must be called prior to this function
**/
bool janus_UpdateQuickUI(struct janus_QuickUI* ui);


/**
 * Renders a QuickUI
 * 
 * @param ui A pointer to a janus_QuickUI to render
 * 
 * @warning unclipped; be careful with non-zero UI offsets or really long option texts
**/
void janus_RenderQuickUI(struct janus_QuickUI* ui);



/********/
/* Misc */
/********/

/**
 * Gives the screen a shake
 * 
 * @param magnitudeX Amount of screenshake along X axis
 * @param magnitudeY Amount of screenshake along Y axis
 * @param length How many shakes to give the screen
 * 
 * @note Random numbers should already be seeded; "srand(rtc_Time());" can be called in initialization to do this.
**/
void janus_Screenshake(uint24_t magnitudeX, uint24_t magnitudeY, uint24_t length);

#ifdef __cplusplus
}
#endif

#endif
