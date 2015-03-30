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
#include "main_menu.h"
#include "common.h"
#include "basic_menu_layer.h"
#include "player.h"
#include "remote.h"
#include "goto.h"
#include "playlists.h"
#include "addons.h"
#include "power.h"

static Window *s_window;
static MenuLayer *s_menu;
static GBitmap *s_icon_player;
static GBitmap *s_icon_remote;
static GBitmap *s_icon_goto;
static GBitmap *s_icon_list;
static GBitmap *s_icon_addons;
static GBitmap *s_icon_power;
static BasicMenuModel *s_model;

static void menu_select_cb(MenuLayer *menu_layer, MenuIndex *cell_index, BasicMenuItem *item) {
  void (*callback)(void) = item->value;
  callback();
}

static void initialize_ui(void) {
  s_icon_player = gbitmap_create_with_resource(RESOURCE_ID_ICON_PLAYER);
  s_icon_remote = gbitmap_create_with_resource(RESOURCE_ID_ICON_REMOTE);
  s_icon_goto = gbitmap_create_with_resource(RESOURCE_ID_ICON_GOTO);
  s_icon_list = gbitmap_create_with_resource(RESOURCE_ID_ICON_LIST);
  s_icon_addons = gbitmap_create_with_resource(RESOURCE_ID_ICON_ADDONS);
  s_icon_power = gbitmap_create_with_resource(RESOURCE_ID_ICON_POWER);

  s_window = window_create();
  window_set_fullscreen(s_window, false);
  window_set_background_color(s_window, GColorBlack);
  Layer *window_layer = window_get_root_layer(s_window);
  GRect bounds = layer_get_frame(window_layer);

  s_model = basic_menu_model_create();
  basic_menu_model_add_item(s_model, "Player", s_icon_player, show_player);
  basic_menu_model_add_item(s_model, "Remote", s_icon_remote, show_remote);
  basic_menu_model_add_item(s_model, "Go to", s_icon_goto, show_goto);
  basic_menu_model_add_item(s_model, "Playlists", s_icon_list, show_playlists);
  basic_menu_model_add_item(s_model, "Addons", s_icon_addons, show_addons);
  basic_menu_model_add_item(s_model, "Power", s_icon_power, show_power);
  
  s_menu = basic_menu_layer_create(bounds, s_model);
  basic_menu_model_set_select_callback(s_model, menu_select_cb);
  
  menu_layer_set_click_config_onto_window(s_menu, s_window);
  layer_add_child(window_layer, menu_layer_get_layer(s_menu));
}

static void destroy_ui() {
  window_destroy(s_window);
  gbitmap_destroy(s_icon_player);
  gbitmap_destroy(s_icon_remote);
  gbitmap_destroy(s_icon_goto);
  gbitmap_destroy(s_icon_list);
  gbitmap_destroy(s_icon_addons);
  gbitmap_destroy(s_icon_power);
  basic_menu_layer_destroy(s_menu);
  basic_menu_model_destroy(s_model);
}

static void handle_window_unload(Window* window) {
  destroy_ui();
}

void show_main_menu(void) {
  initialize_ui();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .unload = handle_window_unload,
  });
  window_stack_push(s_window, true);
}

void hide_main_menu(void) {
  window_stack_remove(s_window, true);
}