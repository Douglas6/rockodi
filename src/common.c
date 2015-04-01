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
#include "common.h"

// AppMessage handlers
void outbox_send(char* send_cmd){
	DictionaryIterator *iter;
 	app_message_outbox_begin(&iter);

  Tuplet value = TupletCString(MSG_KEY_CMD, send_cmd);
	dict_write_tuplet(iter, &value);

  dict_write_end(iter);
	app_message_outbox_send();
}

void outbox_send_with_data(char* send_cmd, char* send_data){
	DictionaryIterator *iter;
 	app_message_outbox_begin(&iter);
  
	Tuplet code = TupletCString(MSG_KEY_CMD, send_cmd);
	dict_write_tuplet(iter, &code);
  Tuplet data = TupletCString(MSG_KEY_CMD_DATA, send_data);
	dict_write_tuplet(iter, &data);
  
	dict_write_end(iter);
	app_message_outbox_send();
}

char* translate_app_message_result(AppMessageResult app_message_error) {
  char *msg = "";
  switch(app_message_error) {
    case APP_MSG_OK:
      msg = "All good, operation was successful.";
      break;
    case APP_MSG_SEND_TIMEOUT:
      msg = "The other end did not confirm receiving the sent data with an (n)ack in time.";
      break;
    case APP_MSG_SEND_REJECTED:
      msg = "The other end rejected the sent data, with a 'nack' reply.";
      break;
    case APP_MSG_NOT_CONNECTED:
      msg = "The other end was not connected.";
      break;
    case APP_MSG_APP_NOT_RUNNING:
      msg = "The local application was not running.";
      break;
    case APP_MSG_INVALID_ARGS:
      msg = "The function was called with invalid arguments.";
      break;
    case APP_MSG_BUSY:
      msg = "There are pending (in or outbound) messages that need to be processed first.";
      break;
    case APP_MSG_BUFFER_OVERFLOW:
      msg = "The buffer was too small to contain the incoming message.";
      break;
    case APP_MSG_ALREADY_RELEASED:
      msg = "The resource had already been released.";
      break;
    case APP_MSG_CALLBACK_ALREADY_REGISTERED:
      msg = "The callback node was already registered, or its ListNode has not been initialized.";
      break;
    case APP_MSG_CALLBACK_NOT_REGISTERED:
      msg = "The callback could not be deregistered, because it had not been registered before.";
      break;
    case APP_MSG_OUT_OF_MEMORY:
      msg = "The support library did not have sufficient application memory.";
      break;
    case APP_MSG_CLOSED:
      msg = "App message was closed.";
      break;
    case APP_MSG_INTERNAL_ERROR:
      msg = "An internal OS error prevented APP_MSG from completing an operation";
      break;
  }

  return msg;
}

char* split_id_name_pair(char *pair) {
  char* pipe_pos = strchr(pair, '|');
  if (pipe_pos != NULL) {
    (*pipe_pos) = '\0';
  }
  
  return pipe_pos + 1;
}

void strip_ext(char *str) {
  char* dot_pos = strchr(str, '.');
  if (dot_pos != NULL) {
    (*dot_pos) = '\0';
  }
}

