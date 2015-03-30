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
#include "remote.h"
#include "common.h"
#include "buttons_window.h"

static Window *s_window;
static GBitmap *s_icon_cursor_up;
static GBitmap *s_icon_check;
static GBitmap *s_icon_cursor_down;
static ActionBarLayer *s_action_bar;

static void up_click_cb(ClickRecognizerRef recognizer, void *context) {outbox_send("KEY_UP");}
static void down_click_cb(ClickRecognizerRef recognizer, void *context) {outbox_send("KEY_DOWN");}
static void select_click_cb(ClickRecognizerRef recognizer, void *context) {outbox_send("KEY_SELECT");}
static void left_click_cb(ClickRecognizerRef recognizer, void *context) {outbox_send("KEY_LEFT");}
static void right_click_cb(ClickRecognizerRef recognizer, void *context) {outbox_send("KEY_RIGHT");}
static void back_click_cb(ClickRecognizerRef recognizer, void *context) {outbox_send("KEY_BACK");}

static void click_provider(void *context)
{
    window_single_click_subscribe(BUTTON_ID_UP, up_click_cb);
    window_single_click_subscribe(BUTTON_ID_DOWN, down_click_cb);
    window_single_click_subscribe(BUTTON_ID_SELECT, select_click_cb);
    window_long_click_subscribe(BUTTON_ID_UP, 350, left_click_cb, NULL);
    window_long_click_subscribe(BUTTON_ID_DOWN, 350, right_click_cb, NULL);
    window_long_click_subscribe(BUTTON_ID_SELECT, 350, back_click_cb, NULL);
}

static void initialize_ui() {
  s_window = buttons_window_create();
  window_set_fullscreen(s_window, false);
  Layer *window_layer = window_get_root_layer(s_window);

  s_icon_cursor_up = gbitmap_create_with_resource(RESOURCE_ID_ICON_CURSOR_UP);
  s_icon_check = gbitmap_create_with_resource(RESOURCE_ID_ICON_CHECK);
  s_icon_cursor_down = gbitmap_create_with_resource(RESOURCE_ID_ICON_CURSOR_DOWN);

  // s_bar_rmt
  s_action_bar = action_bar_layer_create();
  action_bar_layer_add_to_window(s_action_bar, s_window);
  action_bar_layer_set_icon(s_action_bar, BUTTON_ID_UP, s_icon_cursor_up);
  action_bar_layer_set_icon(s_action_bar, BUTTON_ID_SELECT, s_icon_check);
  action_bar_layer_set_icon(s_action_bar, BUTTON_ID_DOWN, s_icon_cursor_down);
  action_bar_layer_set_click_config_provider(s_action_bar, click_provider);
  layer_add_child(window_layer, (Layer*) s_action_bar);
  
  buttons_window_set_label(BUTTON_ID_UP, "Up / Left");
  buttons_window_set_label(BUTTON_ID_SELECT, "Select / Back");
  buttons_window_set_label(BUTTON_ID_DOWN, "Down / Right");
}

static void destroy_ui() {
  buttons_window_destroy(s_window);
  action_bar_layer_destroy(s_action_bar);
  gbitmap_destroy(s_icon_cursor_up);
  gbitmap_destroy(s_icon_check);
  gbitmap_destroy(s_icon_cursor_down);
}

static void handle_window_unload(Window* window) {
  destroy_ui();
}

void show_remote(void) {
  initialize_ui();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .unload = handle_window_unload,
  });
  window_stack_push(s_window, true);
}

void hide_remote(void) {
  window_stack_remove(s_window, true);
}
