/*******************************************************************
 * test_lift.cpp
 *
 * EBC Retractable lift unittest
 *
 *******************************************************************/
#include <Arduino.h>
#include <FS.h>
#include <LittleFS.h>
#include <unity.h>

/*******************************************************************
 * index.html
 *******************************************************************/
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>    
<title>...</title>
    <meta name="viewport" html="width=device-width, initial-scale=1.0">
    <link rel="icon"  href="data:,">
    <style>
    html {
      font-family: Arial; 
      text-align: center;
    }
    body {
      max-width: 400px;
      margin: 0px auto;
    }
  </style>  
  </head>
  <body>
    <h2><span style="font-size:22px;color:#090909;">RIM DRIVE <span style="color:#3498db;">TECHNOLOGY</span></h2>
    <p>&nbsp;</p>
    <p>&nbsp;</p>
    <p>&nbsp;</p>
    <p><span style="font-size:16px;color:#090909;">Program: <span id="program_name">-</span></span></p>
    <p><span style="font-size:16px;color:#090909;">Chip-id: <span id="chip_id">-</span></span></p>
    <p><span style="font-size:16px;color:#090909;">WiFi SSID: <span id="wifi_ssid">-</span></span></p>
    <p>&nbsp;</p>
    <p>&nbsp;</p>
    <p>&nbsp;</p>
    <p><a href="/update"><button>Upload Program</button></a></p>
    <p>&nbsp;</p>
    <p><a href="/webserial"><button>Program interface</button></a></p>
    <p>&nbsp;</p>
    <p><a href="./maintenance/index.html"><button>Maintenance</button></a></p>
  </body>
  <script>
    var Socket;
      function init() {
        Socket = new WebSocket("ws://" + window.location.hostname + ":81/");
        Socket.onmessage = function(event) {
          processCommand(event);
        };
      }

      function processCommand(event) {
        var obj = JSON.parse(event.data);
        document.getElementById("program_name").innerHTML = obj.program_name;
        document.getElementById("chip_id").innerHTML = obj.chip_id;
        document.getElementById("wifi_ssid").innerHTML = obj.wifi_ssid;
      }

      window.onload = function(event) {
        init();
      }  
  </script>  
)rawliteral";

/*******************************************************************
 * SUITE SetUp
 *******************************************************************/
void setUp(void) {
  // set stuff up here
}

/*******************************************************************
 * SUITE TearDown
 *******************************************************************/
void tearDown(void) {
  // clean stuff up here
}

/*******************************************************************
 * TC Initialise variables
 *******************************************************************/
void test_html_replace_title(void) {
  int start, end;
  String html = String(index_html);

  start = html.indexOf("<title>");
  TEST_ASSERT_EQUAL(35, start);

#define PROGRAM_NAME_PLACEHOLDER "<span id=\"program_name\">-</span>"
  start = html.indexOf(PROGRAM_NAME_PLACEHOLDER);
  TEST_ASSERT_EQUAL(570, start);
  end = start + sizeof(PROGRAM_NAME_PLACEHOLDER) - 1;
  html = html.substring(0, start) + String("EBC-HTML-TEST") + html.substring(end);

#define CHIP_ID_PLACEHOLDER "<span id=\"chip_id\">-</span>"
  start = html.indexOf(CHIP_ID_PLACEHOLDER);
  TEST_ASSERT_EQUAL(655, start);
  end = start + sizeof(CHIP_ID_PLACEHOLDER) - 1;
  html = html.substring(0, start) + String("CHIP-ID-123456") + html.substring(end);

#define WIFI_SSID_PLACEHOLDER "<span id=\"wifi_ssid\">-</span>"
  start = html.indexOf(WIFI_SSID_PLACEHOLDER);
  TEST_ASSERT_EQUAL(743, start);
  end = start + sizeof(WIFI_SSID_PLACEHOLDER) - 1;
  html = html.substring(0, start) + String("WiFi-SSDI") + html.substring(end);
}

/*******************************************************************
 * Setup and loop
 *******************************************************************/
void setup() {
  // NOTE!!! Wait for > 2 secs
  // if board doesn't support software reset via Serial.DTR/RTS
  delay(2000);
}

void loop() {
  UNITY_BEGIN();

  RUN_TEST(test_html_replace_title);

  UNITY_END();
}
