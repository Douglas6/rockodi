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
#include "basic_menu_layer.h"
#include "common.h"
  
BasicMenuModel *basic_menu_model_create(void) {
  BasicMenuModel *model = malloc(sizeof *model);
  model->item_count = 0;
  
  return model;
}

void basic_menu_model_destroy(BasicMenuModel *model) {
  for (int i=0; i<model->item_count; i++) {
    free(model->items[i]);
  }
  free(model);
}

void basic_menu_model_add_item(BasicMenuModel *model, char *label, GBitmap *icon, void *value) {
  BasicMenuItem *item = malloc(sizeof *item); 
  item->label = label;
  item->icon = icon;
  item->value = value;
  model->items[model->item_count++] = item;
}

void basic_menu_model_set_select_callback(BasicMenuModel *model, BasicMenuSelectCallback cb) {
  model->callback = cb;
}

// menu handlers
static uint16_t menu_get_num_sections_cb(MenuLayer *menu_layer, void *data) {
  return 1;
}

static uint16_t menu_get_num_rows_cb(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  BasicMenuModel *model = (BasicMenuModel*) data;
  switch (section_index) {
    case 0:
      return model->item_count;
    default:
      return 0;
  }
}

static void menu_draw_header_cb(GContext* ctx, const Layer *cell_layer, uint16_t section_index, void *data) {
}

static int16_t menu_get_header_height_cb(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  return 0;
}

static int16_t menu_get_separator_height_cb(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  return 0;
}

static void menu_draw_row_cb(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
  BasicMenuModel *model = (BasicMenuModel*) data;
  GRect bounds = layer_get_frame(cell_layer);
  GBitmap *icon = model->items[cell_index->row]->icon;
  
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, GRect(0, 0, bounds.size.w, bounds.size.h), 12, GCornersAll);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(1, 1, bounds.size.w - 2, bounds.size.h - 2), 12, GCornersAll);

  int offset = 4;
  if (icon != NULL) {
    offset = 44;
    graphics_draw_bitmap_in_rect(ctx, model->items[cell_index->row]->icon, GRect(4, 8, 28, 28));
  }
  graphics_draw_text(ctx, model->items[cell_index->row]->label, fonts_get_system_font(FONT_KEY_GOTHIC_24), 
      GRect(offset, 4, bounds.size.w - offset, 16), GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
}

static void menu_select_cb(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  BasicMenuModel *model = (BasicMenuModel*) data;
  BasicMenuItem *item = model->items[cell_index->row];
  model->callback(menu_layer, cell_index, item);
}

BasicMenuLayer* basic_menu_layer_create(GRect rect, BasicMenuModel* model) {
  BasicMenuLayer* layer = (BasicMenuLayer*) menu_layer_create(rect);
  menu_layer_set_callbacks(layer, model, (MenuLayerCallbacks){
    .get_num_sections = menu_get_num_sections_cb,
    .get_num_rows = menu_get_num_rows_cb,
    .get_header_height = menu_get_header_height_cb,
    .draw_header = menu_draw_header_cb,
    .draw_row = menu_draw_row_cb,
    .select_click = menu_select_cb,
    .get_separator_height = menu_get_separator_height_cb,
  });

  return layer;
}

void basic_menu_layer_destroy(BasicMenuLayer *layer) {
  menu_layer_destroy((MenuLayer*) layer);
}