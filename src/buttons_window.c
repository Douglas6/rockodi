// Copyright (c) 2015 Douglas Otwell
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
#include <pebble.h>
#include "buttons_window.h"
#include "common.h"

static ButtonsWindow *window;
static ButtonsWindowData buttonsWindowData;
static Layer *s_lyr_background;

static void lyr_background_update_proc(Layer *layer, GContext *ctx) {
  GRect layer_bounds = layer_get_bounds(layer);
  window = layer_get_window(layer);

  ButtonsWindowData* data = (ButtonsWindowData*) window_get_user_data(window);

  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, GRect(0, 2, layer_bounds.size.w, 50), 12, GCornersLeft);
  graphics_fill_rect(ctx, GRect(0, 51, layer_bounds.size.w, 50), 12, GCornersLeft);
  graphics_fill_rect(ctx, GRect(0, 100, layer_bounds.size.w, 50), 12, GCornersLeft);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(1, 3, layer_bounds.size.w, 48), 12, GCornersLeft);
  graphics_fill_rect(ctx, GRect(1, 52, layer_bounds.size.w, 48), 12, GCornersLeft);
  graphics_fill_rect(ctx, GRect(1, 101, layer_bounds.size.w, 48), 12, GCornersLeft);
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, GRect(layer_bounds.size.w - 21,2, 21,layer_bounds.size.h - 4), 4, GCornersLeft);
  
  graphics_draw_text(ctx, data->buttons[0].label, fonts_get_system_font(FONT_KEY_GOTHIC_24), 
      GRect(0, 10, layer_bounds.size.w - 28, 16), GTextOverflowModeTrailingEllipsis, GTextAlignmentRight, NULL);
  graphics_draw_text(ctx, data->buttons[1].label, fonts_get_system_font(FONT_KEY_GOTHIC_24), 
      GRect(0, 59, layer_bounds.size.w - 28, 16), GTextOverflowModeTrailingEllipsis, GTextAlignmentRight, NULL);
  graphics_draw_text(ctx, data->buttons[2].label, fonts_get_system_font(FONT_KEY_GOTHIC_24), 
      GRect(0, 108, layer_bounds.size.w - 28, 16), GTextOverflowModeTrailingEllipsis, GTextAlignmentRight, NULL);
}

ButtonsWindow* buttons_window_create() {
  static struct ButtonsWindowButton button_1;
  button_1.label = "Button 1";
  static struct ButtonsWindowButton button_2;
  button_2.label = "Button 2";
  static struct ButtonsWindowButton button_3;
  button_3.label = "Button 3";
  
  buttonsWindowData.buttons[0] = button_1;
  buttonsWindowData.buttons[1] = button_2;
  buttonsWindowData.buttons[2] = button_3;

	window = window_create();
  window_set_background_color(window, GColorBlack);
  window_set_user_data(window, &buttonsWindowData);
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);

  s_lyr_background = layer_create(bounds);
  layer_set_update_proc(s_lyr_background, lyr_background_update_proc);
  layer_add_child(window_layer, (Layer *) s_lyr_background);
  
	return window;
}

void buttons_window_destroy(ButtonsWindow* window) {
	window_destroy(window);
  layer_destroy(s_lyr_background);
}

void buttons_window_set_label(int id, char* label) {
  ButtonsWindowData* data = (ButtonsWindowData*) window_get_user_data(window);
  if (id >= 1 && id <= 3) {
    data->buttons[id - 1].label = label;
  }
}

