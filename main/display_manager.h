// display_manager.h

#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <stdint.h>
#include "lvgl_port.h"

void update_display_with_data(const uint8_t *data, int length);
void display_manager_init();
void apply_screen_temperature_textbox(int sicaklik1_value, int sicaklik2_value);

void my_btnThemeWhiteFunc(void);
void my_btnBlackThemeFunc(void);
void update_regs_data(uint16_t index, uint16_t value);
void save_panel_settings();
void save_theme_settings();
void apply_theme_settings();
void color_wheel_event_cb();


#endif // DISPLAY_MANAGER_H