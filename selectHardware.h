/*hardwareChoice.h Copyright Paul Beaudet 2014 See license for reuse info
* Choose harware here, By commenting in your board and commenting out the
* default board
* Default settings will be that of current dev build : Yun, wired hid
* most arduinos with enough pins have planed support 
* some boards may have unsupported features
*************** BOARD SELECT ***********************************/
//#define LEO // Arduino Micro or Leonardo USB HID or Bluefruit
#define YUN //Bluefruit is only compatible with yun via software serial 
//#define UNO   // Arduinos using the 328p + bluefruit EZ-key HID

//--------- Haptic display
#include "Adafruit_PWM.h" // see readme for harware notes
#define NUMPAGERS 8 // can use up to 16
#define COUNTBACKPAGERS NUMPAGERS - 1

//--------- Button pin-out
const byte buttons[] = { 11,10,9,8,7,6,5,4,13,12 };// up to 16 possible
// pins can be aligned in here if miswired: try to do it right in hardware
// -------- Potentionmeter
#define ADJUST_POT A1

// -------- Serial options
#define YUN_BOOT_OUTPUT true // mark true to see yun boot msgs on serial

//-------------- KEY DEFINITIONS -----------------
#define NEW_LINE       '\n' //determines end of a message in buffer "10"
#define BACKSPACE      8    // output keys
#define TAB_KEY        9
#define CARIAGE_RETURN 13
#define SPACEBAR       32

#if defined(LEO) || defined(YUN) //HID Emulation values
 #define LEFT_CTRL   128
 #define LEFT_SHIFT  129
 #define LEFT_ALT    130
 #define LEFT_GUI    131
 #define RIGHT_CTRL  132
 #define RIGHT_SHIFT 133
 #define RIGHT_ALT   134
 #define RIGHT_GUI   135
 //---------- Macro Conflicts above -----
 #define UP_ARROW    218
 #define DOWN_ARROW  217
 #define LEFT_ARROW  216
 #define RIGHT_ARROW 215
 #define ESC         177
 #define INSERT      209
 #define DELETE      212
 #define PAGE_UP     211
 #define PAGE_DOWN   214
 #define HOME        210
 #define END         213
 #define CAPS_LOCK   193
 #define FUNC_F1     194
 #define FUNK_F2     195
 #define FUNC_F3     196
 #define FUNK_F4     197
 #define FUNC_F5     198
 #define FUNK_F6     199
 #define FUNC_F7     200
 #define FUNK_F8     201
 #define FUNC_F9     202
 #define FUNK_F10    203
 #define FUNC_F11    204
 #define FUNK_F12    205
#endif

#ifdef UNO //HID Emulation values
 #define LEFT_CTRL   0xEO
 #define LEFT_SHIFT  0xE1
 #define LEFT_ALT    0xE2
 #define LEFT_GUI    0xE3
 #define RIGHT_CTRL  0xE4
 #define RIGHT_SHIFT 0xE5
 #define RIGHT_ALT   0xE6
 #define RIGHT_GUI   0xE7
 //---------- Macro Conflicts above -----
 #define UP_ARROW    0x0E
 #define DOWN_ARROW  0x0C
 #define LEFT_ARROW  0x0B
 #define RIGHT_ARROW 0x07
 #define ESC         0x1B
 #define INSERT      0x01
 #define DELETE      0x04
 #define PAGE_UP     0x03
 #define PAGE_DOWN   0x06
 #define HOME        Ox02
 #define END         0x05
 #define CAPS_LOCK   0x1C
 #define FUNC_F1     0x0F
 #define FUNK_F2     0x10
 #define FUNC_F3     0x11
 #define FUNK_F4     0x12
 #define FUNC_F5     0x13
 #define FUNK_F6     0x14
 #define FUNC_F7     0x15
 #define FUNK_F8     0x16
 #define FUNC_F9     0x17
 #define FUNK_F10    0x18
 #define FUNC_F11    0x19
 #define FUNK_F12    0x1A
#endif
