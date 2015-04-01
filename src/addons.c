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
#include "addons.h"
#include "common.h"
#include "basic_menu_layer.h"
#include "player.h"

static Window *s_window;
static MenuLayer *s_menu;
static Layer *s_lyr_loading;
static BasicMenuModel *s_model;
static GFont s_res_gothic_24_bold;

#define ITEMS_BUFFER_SIZE 1024
static uint8_t items_buffer[ITEMS_BUFFER_SIZE];
static DictionaryIterator items_iter;

static void lyr_loading_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);
  graphics_draw_text(ctx, "Loading...", fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), 
      GRect(0, 10, bounds.size.w, 28), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
}

static void load_menu(void) {
  Tuple *t = dict_read_first(&items_iter);
  while(t != NULL) {
    char* id = (char*) t->value->cstring;
    char* name = split_id_name_pair(id);
    basic_menu_model_add_item(s_model, name, NULL, id);
    t = dict_read_next(&items_iter);
  }
  menu_layer_reload_data(s_menu);
  layer_set_hidden(s_lyr_loading, true);
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  dict_write_begin(&items_iter, items_buffer, ITEMS_BUFFER_SIZE);
  Tuple *t = dict_read_first(iterator);
  while(t != NULL) {
    dict_write_cstring(&items_iter, (int) t->key, (char*) t->value->cstring);
    t = dict_read_next(iterator);
  }
  int items_size = dict_write_end(&items_iter);
  load_menu();
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  ERROR("Message dropped! %s", translate_app_message_result(reason));
}

static void menu_select_cb(MenuLayer *menu_layer, MenuIndex *cell_index, BasicMenuItem *item) {
  char* value = item->value;
  outbox_send_with_data("EXE_ADDON", value);
  window_stack_pop(true);
}

static void initialize_ui(void) {
  s_window = window_create();
  window_set_fullscreen(s_window, false);
  window_set_background_color(s_window, GColorBlack);
  Layer *window_layer = window_get_root_layer(s_window);
  GRect bounds = layer_get_frame(window_layer);
  s_res_gothic_24_bold = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);

  s_model = basic_menu_model_create();
  // s_menu
  s_menu = basic_menu_layer_create(bounds, s_model);
  basic_menu_model_set_select_callback(s_model, menu_select_cb);
  menu_layer_set_click_config_onto_window(s_menu, s_window);
  layer_add_child(window_layer, menu_layer_get_layer(s_menu));
  // s_lyr_loading
  s_lyr_loading = layer_create(bounds);
  layer_set_update_proc(s_lyr_loading, lyr_loading_update_proc);
  layer_add_child(window_layer, (Layer *) s_lyr_loading);

  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
}

static void destroy_ui() {
  window_destroy(s_window);
  layer_destroy(s_lyr_loading);
  basic_menu_layer_destroy(s_menu);
  basic_menu_model_destroy(s_model);
}

static void handle_window_unload(Window* window) {
  destroy_ui();
}

void show_addons(void) {
  initialize_ui();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .unload = handle_window_unload,
  });
  window_stack_push(s_window, true);
  
  outbox_send("GET_ADDONS");
}

void hide_addons(void) {
  window_stack_remove(s_window, true);
}

