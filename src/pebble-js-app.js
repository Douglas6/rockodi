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
// MessageQueue implementation is copyright (c) 2014 by Matthew Tole, https://github.com/smallstoneapps/js-message-queue
// Distributed under the MIT license, see above. Many thanks Matthew!
//
var MessageQueue=function(){var RETRY_MAX=5;var queue=[];var sending=false;var timer=null;return{reset:reset,sendAppMessage:sendAppMessage,size:size};function reset(){queue=[];sending=false}function sendAppMessage(message,ack,nack){if(!isValidMessage(message)){return false}queue.push({message:message,ack:ack||null,nack:nack||null,attempts:0});setTimeout(function(){sendNextMessage()},1);return true}function size(){return queue.length}function isValidMessage(message){if(message!==Object(message)){return false}var keys=Object.keys(message);if(!keys.length){return false}for(var k=0;k<keys.length;k+=1){var validKey=/^[0-9a-zA-Z-_]*$/.test(keys[k]);if(!validKey){return false}var value=message[keys[k]];if(!validValue(value)){return false}}return true;function validValue(value){switch(typeof value){case"string":return true;case"number":return true;case"object":if(toString.call(value)=="[object Array]"){return true}}return false}}function sendNextMessage(){if(sending){return}var message=queue.shift();if(!message){return}message.attempts+=1;sending=true;Pebble.sendAppMessage(message.message,ack,nack);timer=setTimeout(function(){timeout()},1e3);function ack(){clearTimeout(timer);setTimeout(function(){sending=false;sendNextMessage()},200);if(message.ack){message.ack.apply(null,arguments)}}function nack(){clearTimeout(timer);if(message.attempts<RETRY_MAX){queue.unshift(message);setTimeout(function(){sending=false;sendNextMessage()},200*message.attempts)}else{if(message.nack){message.nack.apply(null,arguments)}}}function timeout(){setTimeout(function(){sending=false;sendNextMessage()},1e3);if(message.ack){message.ack.apply(null,arguments)}}}}();
// global variables
var ISDEBUG = true;
var g_server_url = "http://192.168.1.76";
var g_user;
var g_password;
var g_player_id;
var g_player_type;
var g_volume;
var g_muted;
var g_play_state;

var JsonRpcRequest = function (method, params) {
  this.jsonrpc = "2.0";
  this.id = 1;
  this.method = method;
  this.params = params;
};

function sendJsonRequest(requests, callback) {
  var response = null;
  
	var req = new XMLHttpRequest();
	req.onload = function(e) {
		if (req.readyState == 4 && req.status == 200) {
      if (ISDEBUG) console.log("Response text: "+req.responseText);
			response = JSON.parse(req.responseText);
      if (response.result == null) {
        handleJsonRpcError(response);
      } else {
        if (callback != null) {
          callback(response.result);
        }
      }
    }
	};
	req.onerror = function(e) {
		console.log("Error connecting to Kodi");
	};
  var url_string = g_server_url+"/jsonrpc?request=";
  req.open('GET', url_string+encodeURI(JSON.stringify(requests)), true);
	req.timeout = 2000;
	req.send(null);
  if (ISDEBUG) console.log("Sent request: "+url_string);
}

function handleJsonRpcError(response) {
  var err_msg = "Unknown error";
  if (response.constructor === Array) {
    for (var i=0;i<response.length;i++) {
      if (response[i].result == null) {
        if (response[i].error!= null) {err_msg = response[i].error.message;}
        console.log("Kodi responded with an error: "+err_msg);
      }
    }
  } else {
    if (response.result == null) {
      if (response.error!= null) {err_msg = response.error.message;}
      console.log("Kodi responded with an error: "+err_msg);
    }
  }
}
// player methods
function getStatus() {
  g_player_id = -1;
  sendJsonRequest(new JsonRpcRequest("Application.GetProperties", {"properties":["volume", "muted"]}), getAppPropertiesCb);
  sendJsonRequest(new JsonRpcRequest("Player.GetActivePlayers", {}), getActivePlayersCb);
}
function getAppPropertiesCb(result) {
  g_volume = result.volume;
  g_muted = (result.muted) ? 1 : 0;
  MessageQueue.sendAppMessage({"MSG_KEY_VOLUME": g_volume, "MSG_KEY_MUTED": g_muted});
}
function getActivePlayersCb(result) {
  g_player_id = -1;
  for (var i=0; i<result.length; i++) {
    var player = result[i];
    g_player_type = player.type;
    if (g_player_type == "audio" || g_player_type == "video") {
      g_player_id = player.playerid;
      break;
    }
  }
  if (g_player_id >= 0) {
    sendJsonRequest(new JsonRpcRequest("Player.GetProperties", {"playerid": g_player_id, "properties": ["speed"]}), getPlayerPropertiesCb);
    sendJsonRequest(new JsonRpcRequest("Player.GetItem", {"playerid": g_player_id, "properties": ["title","album","artist","director","duration"]}), getItemCb);
  } else {
    MessageQueue.sendAppMessage({"MSG_KEY_TITLE": "No player available", "MSG_KEY_ARTIST": ""});
  }
}
function getPlayerPropertiesCb(result) {
  g_play_state = (result.speed > 0) ? 1 : 0;
  MessageQueue.sendAppMessage({"MSG_KEY_PLAY_STATE": g_play_state});
}
function getItemCb(result) {
  var title = (result.item.title === "") ? result.item.label : result.item.title;
  var artist = "";
  if (result.item.type == "movie") {
    artist = (result.item.director.length < 1) ? "" : result.item.director[0];
  } else {
    artist = (result.item.artist.length < 1) ? "" : result.item.artist[0];
  }
  console.log("Artist:"+artist);
  MessageQueue.sendAppMessage({"MSG_KEY_TITLE":title,"MSG_KEY_ARTIST":artist});
}
function playPause() {
  if (g_player_id >= 0) {
    sendJsonRequest(new JsonRpcRequest("Player.PlayPause", {"playerid": g_player_id}), playPauseCb);
  }
}
function playPauseCb(result) {
    g_play_state = result.speed;
    MessageQueue.sendAppMessage({"MSG_KEY_PLAY_STATE": g_play_state});
}
function raiseVolume() {
  g_volume = (g_volume > 95) ? 100 : g_volume + 5;
  sendJsonRequest(new JsonRpcRequest("Application.SetVolume", {"volume": g_volume}), setVolumeCb);
}
function lowerVolume() {
  g_volume = (g_volume < 5) ? 0 : g_volume - 5;
  sendJsonRequest(new JsonRpcRequest("Application.SetVolume", {"volume": g_volume}), setVolumeCb);
}
function setVolumeCb (result) {
  g_volume = result;
  MessageQueue.sendAppMessage({"MSG_KEY_VOLUME": g_volume});
}
function previous() {
  if (g_player_id >= 0) {
    sendJsonRequest(new JsonRpcRequest("Player.GoTo", {"playerid": g_player_id, "to": "previous"}), changeTransportCb);
  }
}
function next() {
  if (g_player_id >= 0) {
    sendJsonRequest(new JsonRpcRequest("Player.GoTo", {"playerid": g_player_id, "to": "next"}), changeTransportCb);
  }
}
function stop() {
  if (g_player_id >= 0) {
    sendJsonRequest(new JsonRpcRequest("Player.Stop", {"playerid": g_player_id}), changeTransportCb);
  }
}
function changeTransportCb(result) {
  getStatus();
}
// Remote methods
function sendKey(input_key) {
  switch(input_key) {
    case "KEY_UP":
      sendJsonRequest(new JsonRpcRequest("Input.Up"));
      break;
    case "KEY_DOWN":
      sendJsonRequest(new JsonRpcRequest("Input.Down"));
      break;
    case "KEY_SELECT":
      sendJsonRequest(new JsonRpcRequest("Input.Select"));
      break;
    case "KEY_BACK":
      sendJsonRequest(new JsonRpcRequest("Input.Back"));
      break;
    case "KEY_LEFT":
      sendJsonRequest(new JsonRpcRequest("Input.Left"));
      break;
    case "KEY_RIGHT":
      sendJsonRequest(new JsonRpcRequest("Input.Right"));
      break;
  }
}
// Goto methods
function gotoFullscreen() {
  sendJsonRequest(new JsonRpcRequest("GUI.setFullScreen",{"fullscreen":true}));
}
function gotoInfo() {
  sendJsonRequest(new JsonRpcRequest("Input.Info"));
}
function gotoHome() {
  sendJsonRequest(new JsonRpcRequest("GUI.ActivateWindow",{"window":"home"}));
}
function gotoMusic() {
  sendJsonRequest(new JsonRpcRequest("GUI.ActivateWindow",{"window":"music"}));
}
function gotoVideo() {
  sendJsonRequest(new JsonRpcRequest("GUI.ActivateWindow",{"window":"video"}));
}
function gotoPictures() {
  sendJsonRequest(new JsonRpcRequest("GUI.ActivateWindow",{"window":"pictures"}));
}
function gotoWeather() {
  sendJsonRequest(new JsonRpcRequest("GUI.ActivateWindow",{"window":"weather"}));
}
// Playlist methods
function getMusicPlaylists() {
  sendJsonRequest(new JsonRpcRequest("Files.GetDirectory", {"directory":"special://musicplaylists","media":"files","properties":["title"],"limits":{"start":0,"end":10}}), getMusicPlaylistsCb);
}
function getMusicPlaylistsCb(result) {
  var dict = {};
  var key = 0;
  var files = result.files;
  for (var i=0;i<files.length;i++) {
    var filename = files[i].file;
    var label = files[i].label;
    dict[key++] = filename + "|" + label;
  }
  MessageQueue.sendAppMessage(dict);
}
function playMusicPlaylist(payload) {
  var filename = payload.MSG_KEY_BUTTON_DATA;
  var requests = [];
  requests.push(new JsonRpcRequest("PlayList.Clear", {"playlistid": 0}));
  requests.push(new JsonRpcRequest("PlayList.Add", {"playlistid":0, "item":{"directory":filename,"recursive":true}}));
  requests.push(new JsonRpcRequest("Player.Open", {"item":{"playlistid":0,"position":0}}));
  sendJsonRequest(requests);
}
// Addon methods
function getAddons() {
  sendJsonRequest(new JsonRpcRequest("Addons.GetAddons", {"type": "xbmc.python.script", "properties": ["name"],"limits":{"start":0,"end":10}}), getAddonsCb);
}
function getAddonsCb(result) {
  var dict = {};
  var key = 0;
  var addons = result.addons;
  for (var i=0;i<addons.length;i++) {
    var id = addons[i].addonid;
    var name = addons[i].name;
    dict[key++] = id + "|" + name;
  }
  MessageQueue.sendAppMessage(dict);
}
function executeAddon(payload) {
  var addonid = payload.MSG_KEY_BUTTON_DATA;
  sendJsonRequest(new JsonRpcRequest("Addons.ExecuteAddon", {"addonid": addonid}));
}
// Power methods
function systemShutdown() {
  sendJsonRequest(new JsonRpcRequest("System.Shutdown"));
}
function systemReboot() {
  sendJsonRequest(new JsonRpcRequest("System.Reboot"));
}
function systemHibernate() {
  sendJsonRequest(new JsonRpcRequest("System.Hibernate"));
}
function systemSuspend() {
  sendJsonRequest(new JsonRpcRequest("System.Suspend"));
}
// Event listeners
Pebble.addEventListener('ready',
  function(e) {
    console.log('Rockodi copyright (c) 2015, Douglas Otwell');
  }
);
Pebble.addEventListener("appmessage",
  function(e) {
    var cmd = e.payload.MSG_KEY_BUTTON_CODE;
    switch(cmd) {
      // Player messages
      case "GET_STATUS": getStatus(); break;
      case "PLAY_PAUSE": playPause(); break;
      case "VOLUME_UP": raiseVolume(); break;
      case "VOLUME_DOWN": lowerVolume(); break;
      case "NEXT": next(); break;
      case "STOP": stop(); break;
      case "PREVIOUS": previous(); break;
      // Remote messages
      case "KEY_SELECT": sendKey(cmd); break;
      case "KEY_UP": sendKey(cmd); break;
      case "KEY_DOWN": sendKey(cmd); break;
      case "KEY_BACK": sendKey(cmd); break;
      case "KEY_LEFT": sendKey(cmd); break;
      case "KEY_RIGHT": sendKey(cmd); break;
      // Goto messages
      case "GO_FULL": gotoFullscreen(); break;
      case "GO_INFO": gotoInfo(); break;
      case "GO_HOME": gotoHome(); break;
      case "GO_MUSIC": gotoMusic(); break;
      case "GO_VIDEO": gotoVideo(); break;
      case "GO_PICS": gotoPictures(); break;
      case "GO_WEATHER": gotoWeather(); break;
      // Playlist messages
      case "GET_MUSIC_PLAYLISTS": getMusicPlaylists(); break;
      case "PLAY_MUSIC_PLAYLIST": playMusicPlaylist(e.payload); break;
      // Addons messages
      case "GET_ADDONS": getAddons(); break;
      case "EXE_ADDON": executeAddon(e.payload); break;
      // Power messages
      case "PWR_SHUTDOWN": systemShutdown(); break;
      case "PWR_REBOOT": systemReboot(); break;
      case "PWR_SUSPEND": systemSuspend(); break;
      case "PWR_HIBERNATE": systemHibernate(); break;
      default: console.log("Command is not defined: "+cmd+"; payload: "+e.payload);
    }
  }
);