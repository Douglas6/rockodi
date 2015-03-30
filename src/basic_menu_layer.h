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
#pragma once

typedef struct {
  char *label;
  void *value;
  GBitmap *icon;
} BasicMenuItem;

typedef void (*BasicMenuSelectCallback)(MenuLayer *menu_layer, MenuIndex *cell_index, BasicMenuItem *item);

typedef struct basic_menu_model {
  int item_count;
  BasicMenuItem* items[32];
  BasicMenuSelectCallback callback;
} BasicMenuModel;

typedef MenuLayer BasicMenuLayer;

BasicMenuModel* basic_menu_model_create(void);
void basic_menu_model_destroy(BasicMenuModel *model);
void basic_menu_model_add_item(BasicMenuModel *model, char *label, GBitmap *icon, void* value);
void basic_menu_model_set_select_callback(BasicMenuModel *model, BasicMenuSelectCallback cb);
  
BasicMenuLayer* basic_menu_layer_create(GRect rect, BasicMenuModel* model);
void basic_menu_layer_destroy(BasicMenuLayer *layer);