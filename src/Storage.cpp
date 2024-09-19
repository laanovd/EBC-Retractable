/********************************************************************
 *    Eeprom.ino
 *
 *    Persistent save data to 'EEPROM'
 *
 *******************************************************************/
#include <Arduino.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <LittleFS.h>
#include <SimpleCLI.h>
#include <esp_chip_info.h>
#include <esp_err.h>
#include <math.h>
#include <time.h>

#include "CLI.h"
#include "Config.h"
#include "Debug.h"
#include "WebServer.h"
#include "WiFiCom.h"
#include "Storage.h"

/********************************************************************
 * Constants
 *******************************************************************/
#undef DEBUG_STORAGE

/* Storage JSON keys */
#define JSON_CHECKSUM "checksum"

#define FORMAT_LITTLEFS_IF_FAILED true
#define APP_CONFIG_FILE "/device.cfg"

/********************************************************************
 * Definitions
 *******************************************************************/

/********************************************************************
 * Globals
 *******************************************************************/
static int file_error_status;

static JsonDocument _dta_stor;

/********************************************************************
 *  Write settings to file system
 *
 *******************************************************************/
static int STORAGE_read(fs::FS &fs, const char *path) {
  file_error_status = ESP_OK;  // Ok

#ifdef DEBUG_STORAGE
  Serial.println(F("Read storage..."));
#endif

  File file = fs.open(path, FILE_READ);
  if (!file || file.isDirectory()) {
    file_error_status = ESP_FAIL;  // Illegal file
    return file_error_status;
  }

  if (!file.available()) {
    file_error_status = ESP_FAIL;  // Error reading file
    return file_error_status;
  }

  size_t size = (file.size() < 10240) ? file.size() : 10240;  // Max 10kB

#ifdef DEBUG_STORAGE
  Serial.printf("File %s, size: %d.", path, size);
#endif

  uint8_t *buffer = (uint8_t *)malloc(size);
  file.read(buffer, size);

  DeserializationError error = deserializeJson(_dta_stor, (char *)buffer);
  if (error) {
    file_error_status = ESP_FAIL;  // Deserialisation error
  }

#ifdef DEBUG_STORAGE
  Serial.println((char *)buffer);
  Serial.printf("");
#endif

  file.close();
  free(buffer);

  return file_error_status;  // OK
}

/********************************************************************
 *  Write settings to file system
 *
 *******************************************************************/
static int STORAGE_write(fs::FS &fs, const char *path) {
  file_error_status = ESP_OK;  // Ok

#ifdef DEBUG_STORAGE
  Serial.println(F("Write storage..."));
#endif

  File file = fs.open(path, FILE_WRITE);
  if (!file || file.isDirectory()) {
    Serial.printf("STORAGE-write: file open error%s.\n\r", path);
    file_error_status = ESP_FAIL;  // Illegal file
    return file_error_status;
  }

  String str;
  serializeJson(_dta_stor, str);

#ifdef DEBUG_STORAGE
  Serial.printf("STORAGE-write: %s.\n\r", str.c_str());
#endif

  if (!file.write((uint8_t *)str.c_str(), str.length())) {
    Serial.printf("STORAGE-write: Write error.\n\r");
    file_error_status = ESP_FAIL;  // Write error
  }
  file.close();

  return file_error_status;  // OK
}

/********************************************************************
 * Return error status last file operation
 *******************************************************************/
// int errorFS(void) {
//   return file_error_status;
// }

/********************************************************************
 * Set string field
 *******************************************************************/
int STORAGE_set_string(const char *key, String value) {
  // Don't write if the same
  if (_dta_stor.containsKey(key)) {
    if (_dta_stor[key] == value)
      return ESP_OK;  // Ok
  }

  _dta_stor[key] = value;
  STORAGE_write(LittleFS, APP_CONFIG_FILE);
  return ESP_OK;  // Ok
}

/********************************************************************
 * Get string field
 *******************************************************************/
int STORAGE_get_string(const char *key, String &value) {
  // Don't write if the same
  if (!_dta_stor.containsKey(key)) {
    return ESP_ERR_NOT_FOUND;  // Not found
  }

  value = _dta_stor[key].as<String>();
  return ESP_OK;  // Ok
}

/********************************************************************
 * Set integer field
 *******************************************************************/
int STORAGE_set_int(const char *key, int value) {
#ifdef DEBUG_STORAGE
  Serial.printf("STORAGE-set-int: %s = %d.\r\n", key, value);
#endif

  // Don't write if the same
  if (_dta_stor.containsKey(key)) {
    if (_dta_stor[key] == value) {
      return ESP_FAIL;  // Ok
    }
  }

  _dta_stor[key] = value;
  if (STORAGE_write(LittleFS, APP_CONFIG_FILE)) {
    CLI_println("ERROR storing settings...");
    return ESP_FAIL;  // Ok
  }
  return ESP_OK;  // Ok
}

/********************************************************************
 * Get string field
 *******************************************************************/
int STORAGE_get_int(const char *key, int &value) {
  // Don't write if the same
  if (!_dta_stor.containsKey(key)) {
    return ESP_ERR_NOT_FOUND;  // Not found
  }

  value = _dta_stor[key].as<int>();
  return ESP_OK;  // Ok
}

/********************************************************************
 * Set float field
 *******************************************************************/
int STORAGE_set_float(const char *key, float value) {
  // Don't write if the same
  if (_dta_stor.containsKey(key)) {
    if (_dta_stor[key] == value)
      return ESP_OK;  // Ok
  }

  _dta_stor[key] = value;
  STORAGE_write(LittleFS, APP_CONFIG_FILE);
  return ESP_OK;  // Ok
}

/********************************************************************
 * Get float field
 *******************************************************************/
int STORAGE_get_float(const char *key, float &value) {
  // Don't write if the same
  if (!_dta_stor.containsKey(key)) {
    return ESP_ERR_NOT_FOUND;  // Not found
  }

  value = _dta_stor[key].as<float>();
  return ESP_OK;  // Ok
}

/********************************************************************
 *  Decode unique chip id
 *******************************************************************/
int ChipId(void) {
  uint32_t chipId = 0;

  for (int i = 0; i < 17; i = i + 8) {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }

  return (int)chipId;
}

String ChipIds(void) {
  char buffer[8];
  snprintf(buffer, sizeof(buffer), "%07X", ChipId());
  return buffer;
}

/********************************************************************
 * @brief Get cpu model info
 *
 * @param model The ESP chip model
 * @return String containing the cpu model info
 *******************************************************************/
static String model_info(esp_chip_model_t model) {
  switch (model) {
    case CHIP_ESP32:
      return "ESP32";
    case CHIP_ESP32S2:
      return "ESP32S2";
    case CHIP_ESP32S3:
      return "ESP32S3";
    case CHIP_ESP32C3:
      return "ESP32C3";
    case CHIP_ESP32H2:
      return "ESP32H2";
    // case CHIP_ESP32C2:
    //     return "ESP32C2";
    default:
      return "Unknown";
  }
}

/********************************************************************
 * @brief Get silicon revision
 *
 * @return String containing the silicon revision
 *******************************************************************/
String get_model_info(void) {
  esp_chip_info_t chip_info;
  esp_chip_info(&chip_info);

  char buffer[96];
  snprintf(buffer,
           sizeof(buffer),
           "%s with %d CPU core(s), WiFi%s%s",
           model_info(chip_info.model),
           chip_info.cores,
           (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
           (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

  return buffer;
}

/********************************************************************
 * Get silicon revision
 *******************************************************************/
String get_silicon_revision(void) {
  esp_chip_info_t chip_info;
  esp_chip_info(&chip_info);

  unsigned major_rev = chip_info.revision / 100;
  unsigned minor_rev = chip_info.revision % 100;

  char buffer[64];
  snprintf(buffer,
           sizeof(buffer),
           "revision v%d.%d",
           major_rev,
           minor_rev);

  return buffer;
}

/********************************************************************
 * Create initial JSON data
 *******************************************************************/
static void STORAGE_create(void) {
  JsonDocument doc;
  size_t total = LittleFS.totalBytes();
  size_t used = LittleFS.usedBytes();

  /* Program name and version */
  doc["name"] = ProgramName;
  doc["version"] = ProgramVersion;

  /* Add chip id */
  doc["chipid"] = ChipIds();

  /* file system statistics */
  doc["fs-total-kb"] = (total / 1024);
  doc["fs-used-kb"] = (used / 1024);

  int free = (int)(((total - used) * 100.0) / total);
  doc["fs-free-%"] = String(free);

  /* Checksum field */
  _dta_stor = doc;  // Make clean copy
}

/********************************************************************
 *  Factory reset
 *******************************************************************/
extern void all_stop(void);

void STORAGE_factory_reset(void) {
  CLI_println("Storage: factory reset...");

  STORAGE_create();
  if (STORAGE_write(LittleFS, APP_CONFIG_FILE)) {
    CLI_println("ERROR storing settings...");
  }

  all_stop();  // Stop al modules

  vTaskDelay(3000 / portTICK_PERIOD_MS);  // 3 sec.
  ESP.restart();
}

/********************************************************************
 *  CLI: List storage content
 *******************************************************************/
static void clicb_list_storage(cmd *c) {
  (void)c;

  String str;
  serializeJsonPretty(_dta_stor, str);
  CLI_println(str);
}

/********************************************************************
 * CLI: Factory reset
 *******************************************************************/
static void clicb_factory_reset(cmd *c) {
  Command cmd(c);
  Argument arg = cmd.getArgument(0);
  String argVal = arg.getValue();

  if (!argVal.equalsIgnoreCase("YES")) {
    CLI_println("Invalid command: FACTORY <yes>.");
    return;
  }

  STORAGE_factory_reset();
  CLI_println("Factory reset of the internal settings: done.");
  system_restart();
}

/********************************************************************
 *  Setup storage command handlers
 *
 *******************************************************************/
void STORAGE_cli_handlers(void) {
  cli.addCommand("storage", clicb_list_storage);
  cli.addSingleArgumentCommand("factory", clicb_factory_reset);
}

/********************************************************************
 * REST API: read handler
 *********************************************************************/
void STORAGE_api_read(AsyncWebServerRequest *request) {
  String str;
  serializeJson(_dta_stor, str);
  request->send(200, "application/json", str.c_str());
}

static rest_api_t STORAGE_api_handlers = {
    /* uri */ "/api/v1/store",
    /* comment */ "WiFi module",
    /* instances */ 1,
    /* fn_create */ nullptr,
    /* fn_read */ STORAGE_api_read,
    /* fn_update */ nullptr,
    /* fn_delete */ nullptr,
};

/********************************************************************
 * Initializes the storage system.
 * 
 * This function initializes the storage system by mounting the LittleFS file system.
 * If the mount operation fails, it prints an error message and returns.
 * 
 * It then attempts to read the storage data from the specified file.
 * If the read operation fails, it creates a new storage data and writes it to the file.
 * 
 * Finally, it prints a success message indicating that the storage has been initialized.
 *******************************************************************/
void STORAGE_init(void) {
  if (!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)) {
    Serial.println(F("LittleFS Mount Failed."));
    Serial.println(F("Storage initialisation failed..."));
    return;
  }

  /* Read or create storage data */
  if (STORAGE_read(LittleFS, APP_CONFIG_FILE)) {
    Serial.println(F("STORAGE read error..."));

    STORAGE_create();
    if (STORAGE_write(LittleFS, APP_CONFIG_FILE)) {
      Serial.println(F("ERROR storing settings..."));
    }
  }

  Serial.println(F("Storage initialised."));
}

/********************************************************************
 * @brief Starts the storage functionality.
 *
 * This function initializes the storage module and sets up the necessary command line interface (CLI) handlers.
 * After calling this function, the storage functionality is ready to be used.
 * 
 * @note Make sure to call this function before using any storage-related operations.
 *******************************************************************/
void STORAGE_start(void) {
  STORAGE_cli_handlers();

  Serial.println(F("Storage started."));
}
