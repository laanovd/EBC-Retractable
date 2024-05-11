/********************************************************************************
 *    TWAI.ino
 *    
 *    TWAI CAN communication channel
 * 
 ********************************************************************************/
#ifndef TWAI_HEADER_ID
#define TWAI_HEADER_ID

#include <driver/gpio.h>
#include <driver/twai.h>
#include <esp_err.h>

/*******************************************************************
 * JSON fields
 *******************************************************************/
#define JSON_TWAI_DEVICE "decice"
#define JSON_TWAI_BAUDRATE "baudrate-kb"
#define JSON_TWAI_STATE "state"
#define JSON_TWAI_RX_FRAMES "rx-frames"
#define JSON_TWAI_RX_ERRORS "rx-errors"
#define JSON_TWAI_RX_MISSED "rx-missed"
#define JSON_TWAI_RX_OVERRUN "rx-overrun"
#define JSON_TWAI_RX_QUEUED "rx-queued"
#define JSON_TWAI_TX_FRAMES "tx-frames"
#define JSON_TWAI_TX_ERRORS "tx-errors"
#define JSON_TWAI_TX_FAILED "tx-failed"
#define JSON_TWAI_TX_QUEUED "tx-queued"
#define JSON_TWAI_ARB_LOST  "arbitrage-lost"
#define JSON_TWAI_BUS_ERROR "bus-errors"

/*********************************************************************
 * Constants
 ********************************************************************/
/* CAN communication baudrate */
#ifndef CAN_250KB
#define CAN_250KB 1
#endif
#ifndef CAN_500KB
#define CAN_500KB 2
#endif

/*********************************************************************
 * Create TWAI string
 ********************************************************************/
extern String TWAI_string(void);

/********************************************************************************
 * COMCOM frame has been transmitted
 ********************************************************************************/
extern bool TWAI_rx_frames(void);
extern bool TWAI_tx_frames(void);

/********************************************************************************
 * COMCOM frame has been received
 ********************************************************************************/
extern bool TWAI_rx_frame(void);

/********************************************************************************
 *  Send a frame through CAN channel
 ********************************************************************************/
extern int TWAI_send(uint32_t id, const uint8_t *buffer, uint8_t length, bool rtr=false, bool extd=false);

/*******************************************************************
 * Set receive handler
 *******************************************************************/
extern void TWAI_rx_handler(CANReceiveHandler handler);

/********************************************************************************
 *  Setup TWAICom
 ********************************************************************************/
extern void TWAI_setup(int mode);

#endif // TWAI_HEADER_ID