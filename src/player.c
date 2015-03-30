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
#include "player.h"
#include "common.h"
  
static Window *s_window;
static GBitmap *s_icon_vol_up;
static GBitmap *s_icon_pause;
static GBitmap *s_icon_play;
static GBitmap *s_icon_vol_dn;
static GBitmap *s_icon_speaker;
static GFont s_res_gothic_18;
static GFont s_res_gothic_24_bold;
static ActionBarLayer *s_action_bar;
static TextLayer *s_txt_title;
static TextLayer *s_txt_artist;
static BitmapLayer *s_bmp_speaker;
static Layer *s_lyr_volume;
static Layer *s_lyr_background;
// player data
static char s_title[64];
static char s_artist[64];

static void up_click_cb(ClickRecognizerRef recognizer, void *context) {outbox_send("VOLUME_UP");}
static void down_click_cb(ClickRecognizerRef recognizer, void *context) {outbox_send("VOLUME_DOWN");}
static void select_click_cb(ClickRecognizerRef recognizer, void *context) {outbox_send("PLAY_PAUSE");}
static void previous_click_cb(ClickRecognizerRef recognizer, void *context) {outbox_send("PREVIOUS");}
static void stop_click_cb(ClickRecognizerRef recognizer, void *context) {outbox_send("STOP");}
static void next_click_cb(ClickRecognizerRef recognizer, void *context) {outbox_send("NEXT");}

static void click_provider(void *context)
{
    window_single_click_subscribe(BUTTON_ID_UP, up_click_cb);
    window_single_click_subscribe(BUTTON_ID_SELECT, select_click_cb);
    window_single_click_subscribe(BUTTON_ID_DOWN, down_click_cb);
    window_long_click_subscribe(BUTTON_ID_UP, 500, previous_click_cb, NULL);
    window_long_click_subscribe(BUTTON_ID_SELECT, 500, stop_click_cb, NULL);
    window_long_click_subscribe(BUTTON_ID_DOWN, 500, next_click_cb, NULL);
}

static void lyr_background_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, GRect(bounds.size.w - 21,2, 21,bounds.size.h - 4), 4, GCornersLeft);
}

static void lyr_volume_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  uint32_t *len = (uint32_t *) layer_get_data(s_lyr_volume);

  graphics_context_set_fill_color(ctx, GColorWhite);
  GRect rect = GRect(0, 1, *len, bounds.size.h - 2);
  graphics_fill_rect(ctx, rect, 1, GCornersAll);
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  uint32_t *layer_data;
  
  Tuple *t = dict_read_first(iterator);
  while(t != NULL) {
    switch(t->key) {
      case MSG_KEY_TITLE: 
        DEBUG("Adjusting title: %s", t->value->cstring);
        strncpy(s_title, t->value->cstring, sizeof(s_title));
        text_layer_set_text(s_txt_title, s_title);
        break;
      case MSG_KEY_ARTIST: 
        DEBUG("Adjusting artist: %s", t->value->cstring);
        strncpy(s_artist, t->value->cstring, sizeof(s_artist));
        text_layer_set_text(s_txt_artist, s_artist);
        break;
      case MSG_KEY_VOLUME: 
        DEBUG("Adjusting volume: %d", (int) t->value->int32);
        layer_data = (uint32_t *)layer_get_data(s_lyr_volume);
    	  *layer_data = t->value->int32;
        layer_mark_dirty(s_lyr_volume);
        break;
      case MSG_KEY_PLAY_STATE: 
        DEBUG("Adjusting play state: %d", (int) t->value->int32);
        if (t->value->int32 > 0) {
          action_bar_layer_set_icon(s_action_bar, BUTTON_ID_SELECT, s_icon_pause);
        } else {
          action_bar_layer_set_icon(s_action_bar, BUTTON_ID_SELECT, s_icon_play);
        }
        break;
    }

    t = dict_read_next(iterator);
  }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  ERROR("Message dropped %s", translate_app_message_result(reason));
}

static void player_tap_cb(AccelAxisType axis, int32_t direction) {
    outbox_send("GET_STATUS");
}

static void initialize_ui() {
  s_window = window_create();
  window_set_fullscreen(s_window, false);
  window_set_background_color(s_window, GColorBlack);
  Layer *window_layer = window_get_root_layer(s_window);
  GRect bounds = layer_get_frame(window_layer);
  
  s_icon_vol_up = gbitmap_create_with_resource(RESOURCE_ID_ICON_VOL_UP);
  s_icon_pause = gbitmap_create_with_resource(RESOURCE_ID_ICON_PAUSE);
  s_icon_play = gbitmap_create_with_resource(RESOURCE_ID_ICON_PLAY);
  s_icon_vol_dn = gbitmap_create_with_resource(RESOURCE_ID_ICON_VOL_DN);
  s_icon_speaker = gbitmap_create_with_resource(RESOURCE_ID_ICON_SPEAKER);
  s_res_gothic_24_bold = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
  s_res_gothic_18 = fonts_get_system_font(FONT_KEY_GOTHIC_18);

  // s_lyr_backround
  s_lyr_background = layer_create(bounds);
  layer_set_update_proc(s_lyr_background, lyr_background_update_proc);
  layer_add_child(window_layer, (Layer *) s_lyr_background);
  // s_player_bar
  s_action_bar = action_bar_layer_create();
  action_bar_layer_add_to_window(s_action_bar, s_window);
  action_bar_layer_set_icon(s_action_bar, BUTTON_ID_UP, s_icon_vol_up);
  action_bar_layer_set_icon(s_action_bar, BUTTON_ID_SELECT, s_icon_pause);
  action_bar_layer_set_icon(s_action_bar, BUTTON_ID_DOWN, s_icon_vol_dn);
  action_bar_layer_set_click_config_provider(s_action_bar, click_provider);
  layer_add_child(window_layer, (Layer *) s_action_bar);
  // s_txt_title
  s_txt_title = text_layer_create(GRect(4, 0, 116, 76));
  text_layer_set_background_color(s_txt_title, GColorClear);
  text_layer_set_text_color(s_txt_title, GColorWhite);
  text_layer_set_text_alignment(s_txt_title, GTextAlignmentCenter);
  text_layer_set_font(s_txt_title, s_res_gothic_24_bold);
  text_layer_set_text(s_txt_title, "Loading...");
  layer_add_child(window_layer, (Layer *) s_txt_title);
  // s_txt_artist
  s_txt_artist = text_layer_create(GRect(4, 78, 116, 56));
  text_layer_set_background_color(s_txt_artist, GColorClear);
  text_layer_set_text_color(s_txt_artist, GColorWhite);
  text_layer_set_text_alignment(s_txt_artist, GTextAlignmentCenter);
  text_layer_set_font(s_txt_artist, s_res_gothic_18);
  layer_add_child(window_layer, (Layer *) s_txt_artist);
  // s_bmap_speaker
  s_bmp_speaker = bitmap_layer_create(GRect(2, 134, 8, 12));
  bitmap_layer_set_bitmap(s_bmp_speaker, s_icon_speaker);
  layer_add_child(window_layer, (Layer *) s_bmp_speaker);
  // s_lyr_volume
  s_lyr_volume = layer_create_with_data(GRect(16, 136, 100, 8), sizeof(uint32_t));
  layer_set_update_proc(s_lyr_volume, lyr_volume_update_proc);
  uint32_t *len = (uint32_t *) layer_get_data(s_lyr_volume);
	*len = 0;
  layer_add_child(window_layer, (Layer *) s_lyr_volume);
  
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);

  outbox_send("GET_STATUS");
}

static void destroy_ui() {
  window_destroy(s_window);
  gbitmap_destroy(s_icon_vol_up);
  gbitmap_destroy(s_icon_pause);
  gbitmap_destroy(s_icon_play);
  gbitmap_destroy(s_icon_vol_dn);
  gbitmap_destroy(s_icon_speaker);
  layer_destroy(s_lyr_background);
  action_bar_layer_destroy(s_action_bar);
  text_layer_destroy(s_txt_title);
  text_layer_destroy(s_txt_artist);
  bitmap_layer_destroy(s_bmp_speaker);
  layer_destroy(s_lyr_volume);
}

static void handle_window_appear(Window* window) {
  accel_tap_service_subscribe(player_tap_cb);
}

static void handle_window_disappear(Window* window) {
  accel_tap_service_unsubscribe();
}

static void handle_window_unload(Window* window) {
  destroy_ui();
}

void show_player(void) {
  initialize_ui();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .unload = handle_window_unload,
    .appear = handle_window_appear,
    .disappear = handle_window_disappear,
  });
  window_stack_push(s_window, true);
}

void hide_player(void) {
  window_stack_remove(s_window, true);
}
