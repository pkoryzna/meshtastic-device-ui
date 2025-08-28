#include "input/I2CKeyboardInputDriver.h"
#include "input/WHYBadgeKeyboardInputDriver.h"
#include "util/ILog.h"
#include <Arduino.h>
#include <Wire.h>

WHYBadgeKeyboardInputDriver::WHYBadgeKeyboardInputDriver(uint8_t address) : I2CKeyboardInputDriver() {
    // TODO: use TCA8418 class when ready and remove Adafruit driver
    registerI2CKeyboard(this, "WHY2025 Badge Keyboard", address);
    this->address = address;
    this->keypad = new Adafruit_TCA8418();
    
    this->lShifted = false;
    this->rShifted = false;
}

static const char WHYBadgeKeyboardInputDriver::BASE_LAYER[80] = {
  LV_KEY_ESC, //     ESCAPE,    // 0x1
  LV_KEY_HOME, //     KEY_SCANCODE_SQUARE,    // 0x2
  LV_KEY_END, //     KEY_SCANCODE_TRIANGLE,  // 0x3
  LV_KEY_DEL, //     KEY_SCANCODE_CROSS,     // 0x4
  LV_KEY_PREV, //     KEY_SCANCODE_CIRCLE,    // 0x5
  LV_KEY_NEXT, //     KEY_SCANCODE_CLOUD,     // 0x6
  LV_KEY_ENTER, //     KEY_SCANCODE_DIAMOND,   // 0x7
  LV_KEY_BACKSPACE, //     KEY_SCANCODE_BACKSPACE, // 0x8
  '0',//     KEY_SCANCODE_0,         // 0x9
  '-',//     KEY_SCANCODE_MINUS,     // 0xa
  '`',//     KEY_SCANCODE_GRAVE,     // 0xb
  '1',//     KEY_SCANCODE_1,         // 0xc
  '2',//     KEY_SCANCODE_2,         // 0xd
  '3',//     KEY_SCANCODE_3,         // 0xe
  '4',//     KEY_SCANCODE_4,         // 0xf

  '5',//     KEY_SCANCODE_5,   // 0x10
  '6',//     KEY_SCANCODE_6,   // 0x11
  '7',//     KEY_SCANCODE_7,   // 0x12
  '8',//     KEY_SCANCODE_8,   // 0x13
  '9',//     KEY_SCANCODE_9,   // 0x14
  '\t',//     KEY_SCANCODE_TAB, // 0x15
  'q',//     KEY_SCANCODE_Q,   // 0x16
  'w',//     KEY_SCANCODE_W,   // 0x17
  'e',//     KEY_SCANCODE_E,   // 0x18
  'r',//     KEY_SCANCODE_R,   // 0x19
  't',//     KEY_SCANCODE_T,   // 0x1a
  'y',//     KEY_SCANCODE_Y,   // 0x1b
  'u',//     KEY_SCANCODE_U,   // 0x1c
  'i',//     KEY_SCANCODE_I,   // 0x1d
  'o',//     KEY_SCANCODE_O,   // 0x1e
  KEY_SCANCODE_FN,//     KEY_SCANCODE_FN,  // 0x1f

  'a',//     KEY_SCANCODE_A,      // 0x20
  's',//     KEY_SCANCODE_S,      // 0x21
  'd',//     KEY_SCANCODE_D,      // 0x22
  'f',//     KEY_SCANCODE_F,      // 0x23
  'g',//     KEY_SCANCODE_G,      // 0x24
  'h',//     KEY_SCANCODE_H,      // 0x25
  'j',//     KEY_SCANCODE_J,      // 0x26
  'k',//     KEY_SCANCODE_K,      // 0x27
  'l',//     KEY_SCANCODE_L,      // 0x28
  LSHIFT,//     KEY_SCANCODE_LSHIFT, // 0x29
  'z',//     KEY_SCANCODE_Z,      // 0x2a
  'x',//     KEY_SCANCODE_X,      // 0x2b
  'c',//     KEY_SCANCODE_C,      // 0x2c
  'v',//     KEY_SCANCODE_V,      // 0x2d
  'b',//     KEY_SCANCODE_B,      // 0x2e
  'n',//     KEY_SCANCODE_N,      // 0x2f

  'm',//     KEY_SCANCODE_M,          // 0x30
  ',',//     KEY_SCANCODE_COMMA,      // 0x31
  '.',//     KEY_SCANCODE_PERIOD,     // 0x32
  LV_KEY_LEFT,//     KEY_SCANCODE_LEFT,       // 0x33
  LV_KEY_DOWN,//     KEY_SCANCODE_DOWN,       // 0x34
  LV_KEY_RIGHT,//     KEY_SCANCODE_RIGHT,      // 0x35
  '/',//     KEY_SCANCODE_SLASH,      // 0x36
  LV_KEY_UP,//     KEY_SCANCODE_UP,         // 0x37
  RSHIFT,//     KEY_SCANCODE_RSHIFT,     // 0x38
  ';',//     KEY_SCANCODE_SEMICOLON,  // 0x39
  '\'',//     KEY_SCANCODE_APOSTROPHE, // 0x3a
  LV_KEY_ENTER,//     KEY_SCANCODE_RETURN,     // 0x3b
  '=',//     KEY_SCANCODE_EQUALS,     // 0x3c
  KEY_SCANCODE_LCTRL,//     KEY_SCANCODE_LCTRL,      // 0x3d
  KEY_SCANCODE_LGUI,//     KEY_SCANCODE_LGUI,       // 0x3e
  KEY_SCANCODE_LALT,//     KEY_SCANCODE_LALT,       // 0x3f

  '\\',//     KEY_SCANCODE_BACKSLASH,   // 0x40
  ' ',//     KEY_SCANCODE_SPACE,       // 0x41
  ' ',//     KEY_SCANCODE_SPACE,       // 0x42
  ' ',//     KEY_SCANCODE_SPACE,       // 0x43
  KEY_SCANCODE_RALT,//     KEY_SCANCODE_RALT,        // 0x44
  'p',//     KEY_SCANCODE_P,           // 0x45
  '[',//     KEY_SCANCODE_LEFTBRACKET, // 0x46
  0,//     KEY_SCANCODE_UNKNOWN,     // 0x47
  0,//     KEY_SCANCODE_UNKNOWN,     // 0x48
  0,//     KEY_SCANCODE_UNKNOWN,     // 0x49
  0,//     KEY_SCANCODE_UNKNOWN,     // 0x4a
  0,//     KEY_SCANCODE_UNKNOWN,     // 0x4b
  0,//     KEY_SCANCODE_UNKNOWN,     // 0x4c
  0,//     KEY_SCANCODE_UNKNOWN,     // 0x4d
  0,//     KEY_SCANCODE_UNKNOWN,     // 0x4e
  0,//     KEY_SCANCODE_UNKNOWN,     // 0x4f

  ']'//     KEY_SCANCODE_RIGHTBRACKET, // 0x50
};

static const char WHYBadgeKeyboardInputDriver::SHIFTED_LAYER[80] = {
  LV_KEY_ESC, //     ESCAPE,    // 0x1
  LV_KEY_HOME, //     KEY_SCANCODE_SQUARE,    // 0x2
  LV_KEY_END, //     KEY_SCANCODE_TRIANGLE,  // 0x3
  LV_KEY_DEL, //     KEY_SCANCODE_CROSS,     // 0x4
  LV_KEY_PREV, //     KEY_SCANCODE_CIRCLE,    // 0x5
  LV_KEY_NEXT, //     KEY_SCANCODE_CLOUD,     // 0x6
  LV_KEY_ENTER, //     KEY_SCANCODE_DIAMOND,   // 0x7
  LV_KEY_BACKSPACE, //     KEY_SCANCODE_BACKSPACE, // 0x8
  ')',//     KEY_SCANCODE_0,         // 0x9
  '_',//     KEY_SCANCODE_MINUS,     // 0xa
  '~',//     KEY_SCANCODE_GRAVE,     // 0xb
  '!',//     KEY_SCANCODE_1,         // 0xc
  '@',//     KEY_SCANCODE_2,         // 0xd
  '#',//     KEY_SCANCODE_3,         // 0xe
  '$',//     KEY_SCANCODE_4,         // 0xf

  '%',//     KEY_SCANCODE_5,   // 0x10
  '^',//     KEY_SCANCODE_6,   // 0x11
  '&',//     KEY_SCANCODE_7,   // 0x12
  '*',//     KEY_SCANCODE_8,   // 0x13
  '(',//     KEY_SCANCODE_9,   // 0x14
  LV_KEY_PREV,//     KEY_SCANCODE_TAB, // 0x15
  'Q',//     KEY_SCANCODE_Q,   // 0x16
  'W',//     KEY_SCANCODE_W,   // 0x17
  'E',//     KEY_SCANCODE_E,   // 0x18
  'R',//     KEY_SCANCODE_R,   // 0x19
  'T',//     KEY_SCANCODE_T,   // 0x1a
  'Y',//     KEY_SCANCODE_Y,   // 0x1b
  'U',//     KEY_SCANCODE_U,   // 0x1c
  'I',//     KEY_SCANCODE_I,   // 0x1d
  'O',//     KEY_SCANCODE_O,   // 0x1e
  KEY_SCANCODE_FN,//     KEY_SCANCODE_FN,  // 0x1f

  'A',//     KEY_SCANCODE_A,      // 0x20
  'S',//     KEY_SCANCODE_S,      // 0x21
  'D',//     KEY_SCANCODE_D,      // 0x22
  'F',//     KEY_SCANCODE_F,      // 0x23
  'G',//     KEY_SCANCODE_G,      // 0x24
  'H',//     KEY_SCANCODE_H,      // 0x25
  'J',//     KEY_SCANCODE_J,      // 0x26
  'K',//     KEY_SCANCODE_K,      // 0x27
  'L',//     KEY_SCANCODE_L,      // 0x28
  LSHIFT,//     KEY_SCANCODE_LSHIFT, // 0x29
  'Z',//     KEY_SCANCODE_Z,      // 0x2a
  'X',//     KEY_SCANCODE_X,      // 0x2b
  'C',//     KEY_SCANCODE_C,      // 0x2c
  'V',//     KEY_SCANCODE_V,      // 0x2d
  'B',//     KEY_SCANCODE_B,      // 0x2e
  'N',//     KEY_SCANCODE_N,      // 0x2f

  'M',//     KEY_SCANCODE_M,          // 0x30
  '<',//     KEY_SCANCODE_COMMA,      // 0x31
  '>',//     KEY_SCANCODE_PERIOD,     // 0x32
  LV_KEY_LEFT,//     KEY_SCANCODE_LEFT,       // 0x33
  LV_KEY_DOWN,//     KEY_SCANCODE_DOWN,       // 0x34
  LV_KEY_RIGHT,//     KEY_SCANCODE_RIGHT,      // 0x35
  '?',//     KEY_SCANCODE_SLASH,      // 0x36
  LV_KEY_UP,//     KEY_SCANCODE_UP,         // 0x37
  RSHIFT,//     KEY_SCANCODE_RSHIFT,     // 0x38
  ':',//     KEY_SCANCODE_SEMICOLON,  // 0x39
  '"',//     KEY_SCANCODE_APOSTROPHE, // 0x3a
  LV_KEY_ENTER,//     KEY_SCANCODE_RETURN,     // 0x3b
  '+',//     KEY_SCANCODE_EQUALS,     // 0x3c
  KEY_SCANCODE_LCTRL,//     KEY_SCANCODE_LCTRL,      // 0x3d
  KEY_SCANCODE_LGUI,//     KEY_SCANCODE_LGUI,       // 0x3e
  KEY_SCANCODE_LALT,//     KEY_SCANCODE_LALT,       // 0x3f

  '|',//     KEY_SCANCODE_BACKSLASH,   // 0x40
  ' ',//     KEY_SCANCODE_SPACE,       // 0x41
  ' ',//     KEY_SCANCODE_SPACE,       // 0x42
  ' ',//     KEY_SCANCODE_SPACE,       // 0x43
  KEY_SCANCODE_RALT,//     KEY_SCANCODE_RALT,        // 0x44
  'P',//     KEY_SCANCODE_P,           // 0x45
  '{',//     KEY_SCANCODE_LEFTBRACKET, // 0x46
  0,//     KEY_SCANCODE_UNKNOWN,     // 0x47
  0,//     KEY_SCANCODE_UNKNOWN,     // 0x48
  0,//     KEY_SCANCODE_UNKNOWN,     // 0x49
  0,//     KEY_SCANCODE_UNKNOWN,     // 0x4a
  0,//     KEY_SCANCODE_UNKNOWN,     // 0x4b
  0,//     KEY_SCANCODE_UNKNOWN,     // 0x4c
  0,//     KEY_SCANCODE_UNKNOWN,     // 0x4d
  0,//     KEY_SCANCODE_UNKNOWN,     // 0x4e
  0,//     KEY_SCANCODE_UNKNOWN,     // 0x4f

  '}'//     KEY_SCANCODE_RIGHTBRACKET, // 0x50
};

void WHYBadgeKeyboardInputDriver::init(void)
{   
    I2CKeyboardInputDriver::init();
    ILOG_INFO("WHYBadgeKeyboardInputDriver::init");
    this->keypad->begin(this->address, &Wire);
    this->keypad->matrix(8, 10);
}

void WHYBadgeKeyboardInputDriver::readKeyboard(uint8_t address, lv_indev_t *indev, lv_indev_data_t *data)
{
    if (keypad->available() > 0)
    {
        int k = keypad->getEvent();
        bool pressed = k & 0x80;
        int rawKey = k & 0x7F;
        char keyValue = BASE_LAYER[rawKey - 1];
        data->state = pressed ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;

        switch (keyValue)
        {
        case LSHIFT:
            data->state = LV_INDEV_STATE_RELEASED;
            lShifted = pressed; break;
        case RSHIFT:
            data->state = LV_INDEV_STATE_RELEASED;
            rShifted = pressed; break;
        case 0:
        case KEY_SCANCODE_LCTRL:
        case KEY_SCANCODE_LGUI:
        case KEY_SCANCODE_LALT:
        case KEY_SCANCODE_FN:
        case KEY_SCANCODE_RALT:
            // todo: do something interesting with those?
            break;
        // remaps
        case LV_KEY_UP:
            data->key = LV_KEY_PREV;
            break;
        case LV_KEY_DOWN:
            data->key = LV_KEY_NEXT;
            break;
        default:
            if (lShifted || rShifted) {
                data->key = SHIFTED_LAYER[rawKey - 1];
            } else {
                data->key = BASE_LAYER[rawKey - 1];
            }
            break;
        }
        // ILOG_DEBUG("WHYBadgeKeyboardInputDriver Read key pressed=%d code=%02x", pressed, keyValue);
    }
}