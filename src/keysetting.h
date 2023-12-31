#include <Arduino.h>
#include <BleKeyboard.h>

#define key1 18
#define key2 19

#define PRESS 0

#define UNABLE 1
#define DISABLE 0

/* one btn mode */
    /* 押す時間 */
    // short
    #define threshold1 200
    // long

    /* 離す時間 */
    // continue
    #define threshold2 200
    // confirm
    #define threshold3 800
    // confirm&space

    /* バックスペースの繰り返し */
    #define backspace_repeat 200

void clearbuffer1();
void clearbuffer2();
int searchKeyCode_forMorse();
int searchKeyCode_forModifier();

static BleKeyboard blekb("Morse Keyboard");


/* setting */
bool auto_space;



bool key1_pressed;
bool key2_pressed;
ulong key1_press_t;
ulong key2_press_t;

int inputbuffer1[8]; // 入力
int inputbuffer2[8]; // 入力
int buffer1index;
int buffer2index;

bool morse_space_flag;

#define morseinput_len 51
ushort morseinput[morseinput_len][10] = {
    {8, 8, 1,1,1,1,1,1,1,1}, // BS
    {65, 2, 1,2,0,0,0,0,0,0}, // A
    {66, 4, 2,1,1,1,0,0,0,0}, // B
    {67, 4, 2,1,2,1,0,0,0,0}, // C
    {68, 3, 2,1,1,0,0,0,0,0}, // D
    {69, 1, 1,0,0,0,0,0,0,0}, // E
    {70, 4, 1,1,2,1,0,0,0,0}, // F
    {71, 3, 2,2,1,0,0,0,0,0}, // G
    {72, 4, 1,1,1,1,0,0,0,0}, // H
    {73, 2, 1,1,0,0,0,0,0,0}, // I
    {74, 4, 1,2,2,2,0,0,0,0}, // J
    {75, 3, 2,1,2,0,0,0,0,0}, // K
    {76, 4, 1,2,1,1,0,0,0,0}, // L
    {77, 2, 2,2,0,0,0,0,0,0}, // M
    {78, 2, 2,1,0,0,0,0,0,0}, // N
    {79, 3, 2,2,2,0,0,0,0,0}, // O
    {80, 4, 1,2,2,1,0,0,0,0}, // P
    {81, 4, 2,2,1,2,0,0,0,0}, // Q
    {82, 3, 1,2,1,0,0,0,0,0}, // R
    {83, 3, 1,1,1,0,0,0,0,0}, // S
    {84, 1, 2,0,0,0,0,0,0,0}, // T
    {85, 3, 1,1,2,0,0,0,0,0}, // U
    {86, 4, 1,1,1,2,0,0,0,0}, // V
    {87, 3, 1,2,2,0,0,0,0,0}, // W
    {88, 4, 2,1,1,2,0,0,0,0}, // X
    {89, 4, 2,1,2,2,0,0,0,0}, // Y
    {90, 4, 2,2,1,1,0,0,0,0}, // Z
    {49, 5, 1,2,2,2,2,0,0,0}, // 1
    {50, 5, 1,1,2,2,2,0,0,0}, // 2
    {51, 5, 1,1,1,2,2,0,0,0}, // 3
    {52, 5, 1,1,1,1,2,0,0,0}, // 4
    {53, 5, 2,1,1,1,2,0,0,0}, // 5
    {54, 5, 2,2,1,1,1,0,0,0}, // 6
    {55, 5, 2,2,1,1,1,0,0,0}, // 7
    {56, 5, 2,2,2,1,1,0,0,0}, // 8
    {57, 5, 2,2,2,2,1,0,0,0}, // 9
    {48, 5, 2,2,2,2,2,0,0,0}, // 0
    {46, 6, 1,2,1,2,1,2,0,0}, // .
    {44, 6, 2,2,1,1,2,2,0,0}, // ,
    {58, 6, 2,2,2,1,1,1,0,0}, // :
    {63, 6, 1,1,2,2,1,1,0,0}, // ?
    {95, 6, 1,1,2,2,1,2,0,0}, // _
    {43, 5, 1,2,1,2,1,0,0,0}, // +
    {45, 6, 2,1,1,1,1,2,0,0}, // -
    {94, 6, 1,1,1,1,1,1,0,0}, // ^
    {47, 5, 2,1,1,2,1,0,0,0}, // /
    {64, 6, 1,2,2,1,2,1,0,0}, // @
    {40, 5, 2,1,2,2,1,0,0,0}, // (
    {41, 6, 2,1,2,2,1,2,0,0}, // )
    {34, 6, 1,2,1,1,2,1,0,0}, // "
    {39, 6, 1,2,2,2,2,1,0,0}, // '
};

#define modifierinput_len 15
ushort modifierinput[modifierinput_len][8] = {
    // { keycode, type, pattern }
    // type 0:write 1:press 2:release 3:conf_auto_space
    // {KEY_BACKSPACE, 0, 1,0,0,0,0}, 1個目の入力で確定させる
    {KEY_RETURN     , 0, 1, 2,0,0,0,0}, // enter
    {0x20           , 0, 2, 2,1,0,0,0}, // space
    {0              , 2, 2, 2,2,0,0,0}, // release all
    {KEY_CAPS_LOCK  , 0, 3, 2,1,2,0,0}, // capslock
    {KEY_LEFT_SHIFT , 1, 3, 2,1,1,0,0}, // shift press
    {KEY_ESC        , 0, 3, 2,2,1,0,0}, // escape
    {KEY_TAB        , 0, 3, 2,2,2,0,0}, // tab
    {KEY_LEFT_CTRL  , 1, 3, 2,1,1,1,0}, // ctrl press
    {KEY_LEFT_ALT   , 1, 3, 2,1,1,2,0}, // alt press
    {UNABLE         , 3, 4, 2,1,2,1,0}, // unable auto space
    {DISABLE        , 3, 4, 2,1,2,2,0}, // disable auto space
    {KEY_UP_ARROW   , 0, 4, 2,2,1,1,0}, // ARROW UP
    {KEY_DOWN_ARROW , 0, 4, 2,2,1,2,0}, // ARROW DOWN
    {KEY_LEFT_ARROW , 0, 4, 2,2,2,1,0}, // ARROW LEFT
    {KEY_RIGHT_ARROW, 0, 4, 2,2,2,2,0}, // ARROW RIGHT
};