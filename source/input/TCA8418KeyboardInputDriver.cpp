#ifdef INPUTDRIVER_TCA8418_KBD_TYPE
#include "drivers/Adafruit_TCA8418.h"
#include "input/TCA8418KeyboardInputDriver.h"
#include "util/ILog.h"
#include <Arduino.h>
#include <Wire.h>

TCA8418KeyboardInputDriver::TCA8418KeyboardInputDriver(void) {}

void TCA8418KeyboardInputDriver::init(void)
{
    keypad.begin(TCA8418_DEFAULT_ADDR, &Wire);
    keypad.matrix(8, 10);
    keyboard = lv_indev_create();
    lv_indev_set_type(keyboard, LV_INDEV_TYPE_KEYPAD);
    lv_indev_set_read_cb(keyboard, keyboard_read);

    if (!inputGroup) {
        inputGroup = lv_group_create();
        lv_group_set_default(inputGroup);
    }
    lv_indev_set_group(keyboard, inputGroup);
}

/******************************************************************
    LV_KEY_NEXT: Focus on the next object
    LV_KEY_PREV: Focus on the previous object
    LV_KEY_ENTER: Triggers LV_EVENT_PRESSED, LV_EVENT_CLICKED, or LV_EVENT_LONG_PRESSED etc. events
    LV_KEY_UP: Increase value or move upwards
    LV_KEY_DOWN: Decrease value or move downwards
    LV_KEY_RIGHT: Increase value or move to the right
    LV_KEY_LEFT: Decrease value or move to the left
    LV_KEY_ESC: Close or exit (E.g. close a Drop down list)
    LV_KEY_DEL: Delete (E.g. a character on the right in a Text area)
    LV_KEY_BACKSPACE: Delete a character on the left (E.g. in a Text area)
    LV_KEY_HOME: Go to the beginning/top (E.g. in a Text area)
    LV_KEY_END: Go to the end (E.g. in a Text area)

    LV_KEY_UP        = 17,  // 0x11
    LV_KEY_DOWN      = 18,  // 0x12
    LV_KEY_RIGHT     = 19,  // 0x13
    LV_KEY_LEFT      = 20,  // 0x14
    LV_KEY_ESC       = 27,  // 0x1B
    LV_KEY_DEL       = 127, // 0x7F
    LV_KEY_BACKSPACE = 8,   // 0x08
    LV_KEY_ENTER     = 10,  // 0x0A, '\n'
    LV_KEY_NEXT      = 9,   // 0x09, '\t'
    LV_KEY_PREV      = 11,  // 0x0B, '
    LV_KEY_HOME      = 2,   // 0x02, STX
    LV_KEY_END       = 3,   // 0x03, ETX
*******************************************************************/



void TCA8418KeyboardInputDriver::keyboard_read(lv_indev_t *indev, lv_indev_data_t *data)
{
    bool lShifted = false;
    bool rShifted = false;
    if (keypad.available() > 0)
    {
        int k = keypad.getEvent();
        bool pressed = k & 0x80;
        int rawKey = k & 0x7F;
        char keyValue = BASE_LAYER[rawKey - 1];
        switch (keyValue)
        {
        case LSHIFT:
            lShifted = pressed; break;
        case RSHIFT:
            rShifted = pressed; break;
        case KEY_SCANCODE_LCTRL:
        case KEY_SCANCODE_LGUI:
        case KEY_SCANCODE_LALT:
        case KEY_SCANCODE_FN:
        case KEY_SCANCODE_RALT:
            // todo: do something interesting with those?
            break;
        default:
            data->state = pressed ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
            if (lShifted || rShifted) {
                data->key = BASE_LAYER[rawKey - 1];
            } else {
                data->key = BASE_LAYER[rawKey - 1];
            }
            break;
        }
    }
}

void TCA8418KeyboardInputDriver::task_handler(void) {}

TCA8418KeyboardInputDriver::~TCA8418KeyboardInputDriver(void) {}

#endif