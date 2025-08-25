#pragma once

#include "input/InputDriver.h"
#include "drivers/Adafruit_TCA8418.h"

constexpr byte LSHIFT = 0x16;
constexpr byte RSHIFT = 0x15;
constexpr byte KEY_SCANCODE_LCTRL = 0x17;
constexpr byte KEY_SCANCODE_LGUI = 0x18;       
constexpr byte KEY_SCANCODE_LALT = 0x19;       
constexpr byte KEY_SCANCODE_FN = 0x0c;
constexpr byte KEY_SCANCODE_RALT = 0x0D;


class TCA8418KeyboardInputDriver : public InputDriver
{
  public:
    static Adafruit_TCA8418 *keypad;

    TCA8418KeyboardInputDriver(void);
    virtual void init(void) override;
    virtual void task_handler(void) override;
    virtual ~TCA8418KeyboardInputDriver(void);

    static const char BASE_LAYER[80];
    static const char SHIFTED_LAYER[80];
    static bool lShifted;
    static bool rShifted;
  protected:
    static void keyboard_read(lv_indev_t *indev, lv_indev_data_t *data);
};