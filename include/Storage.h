/********************************************************************
 *    Eeprom.ino
 *    
 *    Persistent save data to 'EEPROM'
 * 
*********************************************************************/
#ifndef STORAGE_HEADER_ID
#define STORAGE_HEADER_ID

/********************************************************************
* Get the ESP32 Chip id.
 *******************************************************************/
extern int ChipId(void);
extern String ChipIds(void); // Ship ID string

/********************************************************************
 * Get cpu model info
 *******************************************************************/
extern String get_model_info(void);

/********************************************************************
 * Get silicon revision
 *******************************************************************/
extern String get_silicon_revision(void);

/********************************************************************
 * Set string field
 *******************************************************************/
extern int STORAGE_set_string(const char* key, String value);

/********************************************************************
 * Get string field
 *******************************************************************/
extern int STORAGE_get_string(const char* key, String &value);

/********************************************************************
 * Set integer field
 *******************************************************************/
extern int STORAGE_set_int(const char* key, int value);

/********************************************************************
 * Get string field
 *******************************************************************/
extern int STORAGE_get_int(const char* key, int &value);

/********************************************************************
 * Set float field
 *******************************************************************/
extern int STORAGE_set_float(const char* key, float value);

/********************************************************************
 * Get float field
 *******************************************************************/
extern int STORAGE_get_float(const char* key, float &value);

/********************************************************************
 *  Factory reset
 *******************************************************************/
extern void STORAGE_factory_reset(void);

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
extern void STORAGE_init(void);

/********************************************************************
 * @brief Starts the storage functionality.
 *
 * This function initializes the storage module and sets up the necessary command line interface (CLI) handlers.
 * After calling this function, the storage functionality is ready to be used.
 * 
 * @note Make sure to call this function before using any storage-related operations.
 *******************************************************************/
extern void STORAGE_start(void);

#endif // STORAGE_HEADER_ID