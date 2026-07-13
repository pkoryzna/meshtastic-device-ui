#if defined(ARCH_ESP32) && defined(USE_ESP_LCD_WHY2025)

#include "graphics/driver/ESPLCDDriver.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_mipi_dsi.h"
#include "esp_lcd_st7703.h"
#include "esp_ldo_regulator.h"
#include "st7703.h"
#include "esp_heap_caps.h"
#include "esp_check.h"
#include "driver/gpio.h"

ESPLCDDriver::ESPLCDDriver(uint16_t width, uint16_t height) : TFTDriver(nullptr, width, height){}

ESPLCDDriver::ESPLCDDriver(const DisplayDriverConfig &cfg) : TFTDriver(nullptr, 720, 720){}

ESPLCDDriver::~ESPLCDDriver(){}

ESPLCDDriver *ESPLCDDriver::fbDriver = nullptr;
ESPLCDDriver &ESPLCDDriver::create(uint16_t width, uint16_t height)
{
    if (!fbDriver)
        fbDriver = new ESPLCDDriver(width, height);
    return *fbDriver;
}

static bool ESPLCDDriver::notify_lvgl_flush_ready(esp_lcd_panel_handle_t panel, esp_lcd_dpi_panel_event_data_t *edata, void *user_ctx)
{
    lv_display_t *disp = (lv_display_t *)user_ctx;
    lv_display_flush_ready(disp);
    return true;
}

static st7703_lcd_init_cmd_t const custom_init[] = CUSTOM_INIT_CMDS();

static void ESPLCDDriver::enable_dsi_phy_power() {
    ILOG_DEBUG("ESPLCDDriver: Powering on MIPI DSI PHY");
    // Turn on the power for MIPI DSI PHY, so it can go from "No Power" state to "Shutdown" state
    static esp_ldo_channel_handle_t phy_pwr_chan = NULL;
    esp_ldo_channel_config_t        ldo_cfg      = {
                    .chan_id    = MIPI_DSI_PHY_PWR_LDO_CHAN,
                    .voltage_mv = MIPI_DSI_PHY_PWR_LDO_VOLTAGE_MV,
    };
    ESP_ERROR_CHECK(esp_ldo_acquire_channel(&ldo_cfg, &phy_pwr_chan));
}

static void ESPLCDDriver::lvgl_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t)lv_display_get_user_data(disp);
    int offsetx1 = area->x1;
    int offsetx2 = area->x2;
    int offsety1 = area->y1;
    int offsety2 = area->y2;
    // pass the draw buffer to the driver
    // TODO: rotate the screen lol
    esp_lcd_panel_draw_bitmap(panel_handle, offsetx1, offsety1, offsetx2 + 1, offsety2 + 1, px_map);
}

void ESPLCDDriver::init(DeviceGUI *gui)
{
    TFTDriver<void>::init(gui);
    enable_dsi_phy_power();
    ILOG_DEBUG("ESPLCDDriver: creating MIPI DSI bus");
    esp_lcd_dsi_bus_handle_t mipi_dsi_bus = NULL;
    esp_lcd_dsi_bus_config_t bus_config   = {
          .bus_id             = LCD_MIPI_DSI_BUS_ID,
          .num_data_lanes     = LCD_MIPI_DSI_LANE_NUM,
          .phy_clk_src        = MIPI_DSI_PHY_CLK_SRC_DEFAULT,
          .lane_bit_rate_mbps = LCD_MIPI_DSI_LANE_BITRATE_MBPS,
    };
    ESP_ERROR_CHECK(esp_lcd_new_dsi_bus(&bus_config, &mipi_dsi_bus));
    ILOG_DEBUG("ESPLCDDriver: Installing panel io");
    esp_lcd_panel_io_handle_t io         = NULL;
    // we use DBI interface to send LCD commands and parameters
    esp_lcd_dbi_io_config_t  dbi_config = {
          .virtual_channel = 0,
          .lcd_cmd_bits    = 8, // according to the LCD spec
          .lcd_param_bits  = 8, // according to the LCD spec
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_dbi(mipi_dsi_bus, &dbi_config, &io));

    ILOG_DEBUG("Seting up ST7703 LCD device");
    esp_lcd_dpi_panel_config_t dpi_config;

    dpi_config.dpi_clk_src = MIPI_DSI_DPI_CLK_SRC_DEFAULT;
    dpi_config.dpi_clock_freq_mhz = 47;
    dpi_config.virtual_channel = 0;
    dpi_config.pixel_format = (FRAMEBUFFER_BPP == 2 ? LCD_COLOR_PIXEL_FORMAT_RGB565 : LCD_COLOR_PIXEL_FORMAT_RGB888);
    dpi_config.num_fbs = DISPLAY_FRAMEBUFFERS;
    dpi_config.video_timing = esp_lcd_video_timing_t{};

    dpi_config.video_timing.h_size = 720;
    dpi_config.video_timing.v_size = 720;
    dpi_config.video_timing.hsync_back_porch = 120;
    dpi_config.video_timing.hsync_pulse_width = 60;
    dpi_config.video_timing.hsync_front_porch = 106;
    dpi_config.video_timing.vsync_back_porch = 20;
    dpi_config.video_timing.vsync_pulse_width = 4;
    dpi_config.video_timing.vsync_front_porch = 20;

    dpi_config.flags.use_dma2d = true;
    dpi_config.flags.disable_lp = false;


    st7703_vendor_config_t vendor_config = {};
    vendor_config.init_cmds = custom_init;
    vendor_config.init_cmds_size = sizeof(custom_init) / sizeof(st7703_lcd_init_cmd_t);
    vendor_config.init_in_command_mode = true; // this made it work. badgevms firmware may be doing something different

    vendor_config.mipi_config.dsi_bus = mipi_dsi_bus;
    vendor_config.mipi_config.dpi_config = &dpi_config;


    esp_lcd_panel_dev_config_t lcd_dev_config = esp_lcd_panel_dev_config_t{};
    lcd_dev_config.reset_gpio_num = LCD_IO_RST;
    lcd_dev_config.rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB;
    lcd_dev_config.data_endian = LCD_RGB_DATA_ENDIAN_BIG;
    lcd_dev_config.bits_per_pixel = (FRAMEBUFFER_BPP * 8);
    lcd_dev_config.flags.reset_active_high = 1;
    lcd_dev_config.vendor_config = &vendor_config;


    esp_lcd_panel_handle_t panel_handle = NULL;
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7703(io, &lcd_dev_config, &panel_handle));
    // Not supported by this panel!
    // ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(panel_handle, true));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));

    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));

    ILOG_DEBUG("Setting up LVGL display callbacks");

    // lv_init();
    lv_display_t *display = lv_display_create(FRAMEBUFFER_MAX_W, FRAMEBUFFER_MAX_H);
    lv_display_set_color_format(display, LV_COLOR_FORMAT_RGB565);

    // store the MIPI panel handle as our LVGL display's user data, to be passed into callbacks later
    lv_display_set_user_data(display, panel_handle);
    DisplayDriver::display = display;
    // create draw buffers
    void *buf1 = NULL;
    void *buf2 = NULL;

    size_t draw_buffer_sz = FRAMEBUFFER_MAX_W * FRAMEBUFFER_MAX_H * FRAMEBUFFER_BPP / 3;

    buf1 = heap_caps_malloc(draw_buffer_sz, MALLOC_CAP_SPIRAM);
    buf2 = heap_caps_malloc(draw_buffer_sz, MALLOC_CAP_SPIRAM);
    assert(buf1);
    assert(buf2);

    // initialize LVGL draw buffers
    lv_display_set_buffers(display, buf1, buf2, draw_buffer_sz, LV_DISPLAY_RENDER_MODE_PARTIAL);
    // set the callback to copy the updated buffer to the display
    lv_display_set_flush_cb(display, ESPLCDDriver::lvgl_flush_cb);
    
    esp_lcd_dpi_panel_event_callbacks_t cbs = {
        // notify LVGL when our data has been copied to LCD driver's fb
        .on_color_trans_done = ESPLCDDriver::notify_lvgl_flush_ready,
        // don't need to wait for the full vertical refresh, i think?
        // .on_refresh_done = notify_lvgl_flush_ready,
   };
    ESP_ERROR_CHECK(esp_lcd_dpi_panel_register_event_callbacks(panel_handle, &cbs, display));
}
#endif // defined(ARCH_ESP32)
