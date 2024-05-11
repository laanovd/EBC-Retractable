/*******************************************************************
 *    MCP.ino
 *    
 *    Handling MCP communication, and call frame handlers
 * 
 *******************************************************************/
#ifndef MCP_HEADER_ID
#define MCP_HEADER_ID

#include <stdint.h>

/*******************************************************************
 * JSON fields
 *******************************************************************/
#define JSON_MCP_DEVICE "device"
#define JSON_MCP_BAUDRATE "baudrate-kb"
#define JSON_MCP_STATE "state"
#define JSON_MCP_RX_FRAMES "rx-frames"
#define JSON_MCP_RX_ERRORS "rx-errors"
#define JSON_MCP_RX_MISSED "rx-missed"
#define JSON_MCP_RX_OVERRUN "rx-overrun"
#define JSON_MCP_RX_QUEUED "rx-queued"
#define JSON_MCP_TX_FRAMES "tx-frames"
#define JSON_MCP_TX_ERRORS "tx-errors"
#define JSON_MCP_TX_FAILED "tx-failed"
#define JSON_MCP_TX_QUEUED "tx-queued"
#define JSON_MCP_ARB_LOST  "arbitrage-lost"
#define JSON_MCP_BUS_ERROR "bus-errors"

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

/*******************************************************************
 * MCP frames received
 *******************************************************************/
extern bool MCP1_rx_frames(void);

/*******************************************************************
 * MCP frames transmitted
 *******************************************************************/
extern bool MCP1_tx_frames(void);

/*******************************************************************
 * MCP send frame
 *******************************************************************/
extern int MCP1_send(uint32_t id, uint8_t* buffer, uint8_t length, bool rtr=false, bool ext=false);

/*******************************************************************
 * Set receive handler 
 *******************************************************************/
extern void MCP1_rx_handler(CANReceiveHandler handler);

/*******************************************************************
 * Externals
 *******************************************************************/
extern void MCP1_setup(int mode);

#endif // MCP_HEADER_ID