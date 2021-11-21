#ifndef JANUS_LIB
#define JANUS_LIB

#ifdef __cplusplus
extern "C" {
#endif

/* Allows us to index janus_ReleasedKeys, janus_PressedKeys, and janus_PressedOrReleasedKeys just as we would index kb_Data from keypadc.h */
#define KB_DATA_SIZE 8

/* PI approximation */
#define JANUS_PI 3.1415926535f

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

/* One second in milliseconds */
#define JANUS_ONE_SECOND 1000

/* Misc Helper Macros */

#define JANUS_ABS(number) ((number > 0) ? (number) : -(number))


/* Animation Initialization Macros */

/**
 * Puts false, then a uint24_t will become uint24_t constantTiming in the janus_FrameTiming union
 * 
 * @example JANUS_ANIMATION_INIT_CONSTANT_TIMING(500)
 * @note Use in place of JANUS_ANIMATION_INIT_VARIABLE_TIMING to use constant timing
**/
#define JANUS_ANIMATION_INIT_CONSTANT_TIMING(milliseconds) .useVariableTimings = false, {.constantTiming = milliseconds}

/**
 * Puts true, then a list of uint24_t in the input will become uint24_t variableTimings[JANUS_ANIMATION_MAX_FRAMES] in the janus_FrameTiming union
 * Useful for declaring a janus_Animation
 * 
 * @example JANUS_ANIMATION_INIT_VARIABLE_TIMING(1000,100,100,100,100,100,500,100,100,100,150,200)
 * @note Use in place of JANUS_ANIMATION_INIT_CONSTANT_TIMING to use variable timing
**/
#define JANUS_ANIMATION_INIT_VARIABLE_TIMING(...) .useVariableTimings = true, {.variableTimings = {__VA_ARGS__}}


/* Time Helper Macros */

/**
 * Helpful for using delta time to apply motion over a certain time period.
 * Gives the same result as multiplying x by msElapsed if msElapsed were a float repesenting a number of seconds
 * 
 * @param x A variable for msElapsed to be appleid to.
 * @param msElapsed An int representing a number of milliseconds in delta time
**/
#define JANUS_APPLY_DELTA_TIME(x,msElapsed) (x*msElapsed/JANUS_ONE_SECOND)

extern uint8_t janus_ReleasedKeys[KB_DATA_SIZE];
extern uint8_t janus_PressedKeys[KB_DATA_SIZE];
extern uint8_t janus_PressedOrReleasedKeys[KB_DATA_SIZE];

enum janus_EasingMode {
    LINEAR, //No easing
    QUAD_IN, //Ease into motion
    QUAD_OUT, //Ease out of motion
    QUAD_IN_OUT //Ease into and out of motion
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
    int24_t length; // How long the ease should be in milliseconds
    int24_t progress; // How far, in milliseconds, we are into the ease
};

union janus_FrameTiming {
    uint24_t variableTimings[JANUS_ANIMATION_MAX_FRAMES]; //Array of millisecond timings per frame
    uint24_t constantTiming; //Millisecond timings for every frame
};

struct janus_Animation {
    uint24_t frameCount; // Number of frames
    gfx_sprite_t* frames[JANUS_ANIMATION_MAX_FRAMES]; // List of pointers to gfx_sprites
    bool useVariableTimings; // If true, each frame will have their own timing, specified in "variableTimings." Otherwise, each frame will have a constant framerate
    union janus_FrameTiming frameTimings;
    bool loop; // Whether or not to loop animation
    uint24_t msElapsed; // Current progress (in milliseconds) of the animation; initialize at zero for most cases
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

struct janus_Vector2 {
    int24_t x;
    int24_t y;
};

struct janus_Rect {
    struct janus_Vector2 position;
    struct janus_Vector2 size;
};

struct janus_PhysicsObject {
    struct janus_Rect rect; //Position and size of object
    struct janus_Vector2 velocity; //Velocity of object
    int24_t resistance; //How willing an object is to move against other physics objects. The higher the value, the more it will move when pushed by an object with lower value.
    bool anchored;
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
 * Adds two vectors; x + y
 * 
 * @param x first vector
 * @param y second vector
 * @returns struct janus_Vector2 
 */
struct janus_Vector2 janus_AddVectors(struct janus_Vector2* x, struct janus_Vector2* y);

/**
 * Subtracts vectors; x - y
 * 
 * @param x this minus y
 * @param y x minus this
 * @returns struct janus_Vector2 
 */
struct janus_Vector2 janus_SubtractVectors(struct janus_Vector2* x, struct janus_Vector2* y);

/**
 * Multiplies vectors; x * y
 * 
 * @param x this times y
 * @param y x times this
 * @returns struct janus_Vector2 
 */
struct janus_Vector2 janus_MultiplyVectors(struct janus_Vector2* x, struct janus_Vector2* y);

/**
 * Divides vectors; x / y
 * 
 * @param x this divided by y
 * @param y x divided by this
 * @returns struct janus_Vector2 
 */
struct janus_Vector2 janus_DivideVectors(struct janus_Vector2* x, struct janus_Vector2* y);


// (https://stackoverflow.com/questions/19199473/biggest-and-smallest-of-four-integers-no-arrays-no-functions-fewest-if-stat/19199615)
/**
 * Finds the smallest value of four inputs
 * 
 * @returns the smallest of four inputs
**/ 
int24_t janus_SmallestOfFour(int24_t a, int24_t b, int24_t c, int24_t d);
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
 * Linearly interpolates between X and Y using alpha for the percentage between (0-1000)
 * 
 * @param x Interpolate from coordinate
 * @param y Interpolate to coordinate
 * @param alpha How far (0 to 1000) between from and to?
 * @returns integer interpolated between X and Y using alpha
**/ 
int24_t janus_LerpInt24ByInt(int24_t x, int24_t y, int24_t alpha);

/**
 * Returns x to the yth power
 * 
 * @param x Base number
 * @param y Exponent
 * @returns x^y
**/ 
int24_t janus_Pow(int24_t x, int24_t y);

/**
 * Approximates sine
 * 
 * @param x radians
 * @returns Sine of x radians
 * @note Probably works, but if something breaks this function could be at fault
**/ 
float janus_Sin(float x);


/***********************/
/* Physics & Collision */
/***********************/

/**
 * Returns whether or not two janus_Rect are colliding
 * 
 * @returns bool of whether or not there is a collision between object1 and object2
**/
bool janus_Collision(struct janus_Rect* object1, struct janus_Rect* object2);

/**
 * If colliding, two passed objects get pushed away from each other through the smallest possible movement, influenced by each object's "resistance" or how willing it is to not move against other physics objects
 * 
 * @param object1 first object handling collisions
 * @param object2 second obect handling collisions
 * 
 * @returns true if object1 is pushing on top of object2, treating 2 like a floor (good for making a player jump if true)
 */
bool janus_HandleObjectObjectCollision(struct janus_PhysicsObject* object1, struct janus_PhysicsObject* object2);

/**
 * A function that uses slightly less operations than janus_HandleObjectObjectCollision to be used for situations such as a physics object against the floor or a wall
 * 
 * @param object1 the physics object to handle collisions for
 * @param rect the rectangle the physics object is being tested with
 * 
 * @returns true if object1 is pushing on top of the rect like a floor (good for jump logic)
 */
bool janus_HandleObjectRectCollision(struct janus_PhysicsObject* object1, struct janus_Rect* rect);

/**
 * Adds the given input to an object's velocity
 * 
 * @param object object to add the velocity to
 * @param velocity velocity to add to the object
 */

void janus_AddForce(struct janus_PhysicsObject* object, struct janus_Vector2* velocity);

/**
 * You must call this if you want your object's velocity to move the object, or simply add velocity to position yourself (though you would miss out on the delta time being applied)
 * 
 * @param object
 */
void janus_ApplyVelocity(struct janus_PhysicsObject* object, int24_t deltaMS);


/**
 * Reduces the object's velocity using vector "dampen." The higher "dampen" is, the less the object will be dampened. {1,1} will result in zero velocity.
 * 
 * @param object object to dampen
 * @param dampen object's velocity reduced using this 
 */
void janus_DampenVelocity(struct janus_PhysicsObject* object, struct janus_Vector2* dampen);

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
 * Converts milliseconds delta time into frames per second
 * @param msElapsed acquired by calling janus_GetDeltaTime at the start of each frame
 * @returns frame rate based on input delta time
**/
int24_t janus_GetFPS(int24_t msElapsed);

/** 
 * Gives milliseconds since the last time the function was called.
 * 
 * @returns uint24_t containing the number of milliseconds since the function was last called.
 * 
 * @example To use this for a certain amount of movement per second, multiply milliseconds by your movement per second and divide that by a thousand *for* the amount of movement to apply that frame.
**/
int24_t janus_GetDeltaTime(void);


/************************/
/* Easing and Animation */
/************************/

/**
 * Adds elapsedTime to the ease pointer's progress and updates the ease's currentX and currentY based on the easing mode
 * 
 * @param ease A pointer to the ease object (janus_Ease) to update
 * @param msElapsed The time (in milliseconds) to add to the ease progress
**/
void janus_UpdateEase(struct janus_Ease* ease, int24_t msElapsed);

/**
 * Returns the progress of an easing mode based on alpha (0-1).
 * 
 * @param easingMode Takes a janus_EasingMode to determine what way to interpolate
 * @param alpha Takes a float between 0 (0%) and 1 (100%)
 * 
 * @returns float containing progress of ease
**/
uint24_t janus_GetEaseProgress(enum janus_EasingMode easingMode, uint24_t alpha);

/**
 * Updates an animation by setting the current frame/elapsed time.
 * 
 * @param animation Pointer to animation to update
 * @param elapsedTime Time since last call to update animation
**/ 
void janus_UpdateAnimation(struct janus_Animation* animation, int24_t msElapsed);

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
