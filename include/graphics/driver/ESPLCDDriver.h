#if defined(ARCH_ESP32) && defined(USE_ESP_LCD_WHY2025)
#include "graphics/driver/TFTDriver.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_mipi_dsi.h"
#include "esp_lcd_st7703.h"
#include "esp_ldo_regulator.h"
#include "util/ILog.h"

class ESPLCDDriver : public TFTDriver<void>
{

public:
    ESPLCDDriver(uint16_t width, uint16_t height);
    ESPLCDDriver(const DisplayDriverConfig &cfg);
    ~ESPLCDDriver();
    void init(DeviceGUI *gui) override;
    bool hasTouch(void) override { return false; }
    bool hasButton(void) override { return false; }
    bool hasLight(void) override { return true; }
    bool isPowersaving(void) override { return false; } // todo

    static ESPLCDDriver &create(uint16_t width, uint16_t height);
protected:
    static void lvgl_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map);
    static bool notify_lvgl_flush_ready(esp_lcd_panel_handle_t panel, esp_lcd_dpi_panel_event_data_t *edata, void *user_ctx);
    static void enable_dsi_phy_power(void);
    
private:
    static ESPLCDDriver *ESPLCDDriver::fbDriver;
};
#endif