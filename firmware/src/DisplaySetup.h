#pragma once

#ifndef USE_SDL
#include <LovyanGFX.hpp>
#include <lvgl.h>

class LGFX : public lgfx::LGFX_Device {
    lgfx::Panel_GC9A01 _panel_instance;
    lgfx::Bus_SPI _bus_instance;

public:
    LGFX(void) {
        {
            auto cfg = _bus_instance.config();
            cfg.spi_host = SPI2_HOST;
            cfg.spi_mode = 0;
            cfg.freq_write = 20000000;
            cfg.freq_read  = 16000000;
            cfg.spi_3wire  = false;
            cfg.use_lock   = true;
            cfg.dma_channel = SPI_DMA_CH_AUTO;
            // Left-side wiring we proved works!
            cfg.pin_sclk = 3; // XIAO D2 (row 2)
            cfg.pin_mosi = 2; // XIAO D1 (row 1)
            cfg.pin_miso = -1;
            cfg.pin_dc   = 5; // XIAO D4 (row 4)
            _bus_instance.config(cfg);
            _panel_instance.setBus(&_bus_instance);
        }

        {
            auto cfg = _panel_instance.config();
            cfg.pin_cs           = 4; // XIAO D3 (row 3)
            cfg.pin_rst          = 6; // XIAO D5 (row 5)
            cfg.pin_busy         = -1;
            cfg.memory_width     = 240;
            cfg.memory_height    = 240;
            cfg.panel_width      = 240;
            cfg.panel_height     = 240;
            cfg.offset_x         = 0;
            cfg.offset_y         = 0;
            cfg.offset_rotation  = 0;
            cfg.dummy_read_pixel = 8;
            cfg.dummy_read_bits  = 1;
            cfg.readable         = true;
            cfg.invert           = true;
            cfg.rgb_order        = false;
            cfg.dlen_16bit       = false;
            cfg.bus_shared       = true;
            _panel_instance.config(cfg);
        }

        setPanel(&_panel_instance);
    }
};

LGFX lcd;

/* LVGL display flushing */
static void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    lcd.startWrite();
    lcd.setAddrWindow(area->x1, area->y1, w, h);
    lcd.writePixels((lgfx::rgb565_t *)&color_p->full, w * h);
    lcd.endWrite();

    lv_disp_flush_ready(disp);
}

#else
#include <lvgl.h>
#include "sdl/sdl.h"
#endif

static const uint16_t screenWidth  = 240;
static const uint16_t screenHeight = 240;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[screenWidth * screenHeight / 10];

lv_obj_t * gear_label;

void initDisplay() {
    Serial.println("initDisplay: starting lv_init...");
    lv_init();

#ifndef USE_SDL
    Serial.println("initDisplay: starting lcd.init...");
    lcd.init();
    lcd.setBrightness(255);
    lcd.fillScreen(TFT_BLACK);
    Serial.println("initDisplay: lcd init done, screen filled black.");
#else
    sdl_init();
#endif

    lv_disp_draw_buf_init(&draw_buf, buf, NULL, screenWidth * screenHeight / 10);

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = screenWidth;
    disp_drv.ver_res = screenHeight;
#ifndef USE_SDL
    disp_drv.flush_cb = my_disp_flush;
#else
    disp_drv.flush_cb = sdl_display_flush;
#endif
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

#ifdef USE_SDL
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = sdl_mouse_read;
    lv_indev_drv_register(&indev_drv);
#endif

    // Create a massive label for the gear
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x050505), LV_PART_MAIN); // Slightly off-black for premium feel
    
    gear_label = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_color(gear_label, lv_color_hex(0x00FF00), LV_PART_MAIN); // Default green for N
    
    // Declare and use the massive custom font
    LV_FONT_DECLARE(ui_font_180);
    lv_obj_set_style_text_font(gear_label, &ui_font_180, LV_PART_MAIN);
    
    lv_label_set_text(gear_label, "N");
    lv_obj_center(gear_label);
}

#ifndef USE_SDL
void updateDisplayGear(String gear) {
    if (gear_label) {
        lv_label_set_text(gear_label, gear.c_str());
        
        // Dynamic colors based on the gear
        if (gear == "N") {
            lv_obj_set_style_text_color(gear_label, lv_color_hex(0x00FF33), LV_PART_MAIN); // Vibrant Green
        } else if (gear == "R") {
            lv_obj_set_style_text_color(gear_label, lv_color_hex(0xFF2222), LV_PART_MAIN); // Bright Red
        } else {
            lv_obj_set_style_text_color(gear_label, lv_color_hex(0xE0F7FA), LV_PART_MAIN); // Ice Blue/White for gears
        }
        
        lv_obj_center(gear_label);
    }
}
#else
#include <string>
void updateDisplayGear(std::string gear) {
    if (gear_label) {
        lv_label_set_text(gear_label, gear.c_str());
        
        if (gear == "N") {
            lv_obj_set_style_text_color(gear_label, lv_color_hex(0x00FF33), LV_PART_MAIN);
        } else if (gear == "R") {
            lv_obj_set_style_text_color(gear_label, lv_color_hex(0xFF2222), LV_PART_MAIN);
        } else {
            lv_obj_set_style_text_color(gear_label, lv_color_hex(0xE0F7FA), LV_PART_MAIN);
        }
        
        lv_obj_center(gear_label);
    }
}
#endif
