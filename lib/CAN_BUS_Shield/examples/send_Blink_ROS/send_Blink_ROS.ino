// demo: CAN-BUS Shield, send data
// Hardware: TWO ArduinoMega with CAN-SHIELDS
// Run this send_Blink_ROS.ino file on one of the Arduinos
// Run receive_Blink on the other
// ***ROS commands to be followe***//
// roscore
// rosrun roial_python serial_node.py /dev/ttyACM1 _baud:=57600
// rostopic pub toggle_led std_msgs/Empty -r 100
// Jaghvi: jaghvim@andrew.cmu.edu

#include <SPI.h>
#include "mcp2515_can.h"
//ROS
#include <ros.h>
#include <std_msgs/Empty.h>

ros::NodeHandle  nh;

const int SPI_CS_PIN = 9;
const int ledHIGH = 1;
const int ledLOW = 0;
unsigned char stmp[8] = {ledHIGH, 1, 2, 3, ledLOW, 5, 6, 7};

mcp2515_can CAN(SPI_CS_PIN); // Set CS pin

void messageCb(const std_msgs::Empty& toggle_msg) {
    //digitalWrite(13, HIGH-digitalRead(13));   // blink the led
    // send data:  id = 0x00, standrad frame, data len = 8, stmp: data buf
    CAN.sendMsgBuf(0x70, 0, 8, stmp);
    delay(1000);                       // send data per 100ms
}

ros::Subscriber<std_msgs::Empty> sub("toggle_led", &messageCb);

// the cs pin of the version after v1.1 is default to D9
// v0.9b and v1.0 is default D10



void setup() {
    SERIAL_PORT_MONITOR.begin(115200);
    nh.initNode();
    nh.subscribe(sub);

    while (CAN_OK != CAN.begin(CAN_500KBPS)) {            // init can bus : baudrate = 500k
        SERIAL_PORT_MONITOR.println("CAN init fail, retry...");
        delay(100);
    }
    SERIAL_PORT_MONITOR.println("CAN init ok!");
}


void loop() {

    nh.spinOnce();
    delay(1);
}

/*******************************************************************
    END FILE
*********************************************************************************************************/
