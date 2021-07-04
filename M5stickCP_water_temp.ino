/*
    Description: insert CAN Unit to PORT-B,Press buttonA or buttonB to Send message
    if received message, screen will display
*/

//#include <M5Stack.h>
#include <M5StickCPlus.h>
#include <AXP192.h>
#include "ESP32CAN.h"
#include "CAN_config.h"
#include "freed.h"
#include "wtemp.h"
// #include "esp_pm.h"

#define TX GPIO_NUM_32
#define RX GPIO_NUM_33

CAN_device_t CAN_cfg;               // CAN Config
TFT_eSprite Disbuff = TFT_eSprite(&M5.Lcd);
CAN_frame_t rx_frame;


int i = 0;
unsigned long prev_time;
byte loop_counter;
int last_rpm;


void header(const char *string, uint16_t color)
{
    M5.Lcd.setRotation(1);
    M5.Lcd.fillScreen(color);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Lcd.fillRect(0, 0, 239, 134,0x0000);
    M5.Lcd.setTextDatum(TC_DATUM);
    M5.Lcd.startWrite();
    M5.Lcd.pushImage(0, 2, imgWidth_w, imgHeight_w, img_w);
    M5.Lcd.endWrite();

    M5.Lcd.setCursor(70, 5);
    M5.Lcd.setTextFont(4);
    M5.Lcd.setTextSize(2);
    M5.Lcd.print(0);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextColor(TFT_BLUE, TFT_BLACK);
    M5.Lcd.drawString("Engine RPM", 10, 62, 4); 
    M5.Lcd.setTextFont(0);
    M5.Lcd.setTextSize(0);
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Lcd.drawString("0    1    2    3    4    5    6    7", 4, 90, 1); 
    
}

void setup() {
  M5.begin(true, true, true);
  M5.Axp.begin();
//  M5.Power.begin();
  Serial.println("CAN Unit Send&Received");
  header("CAN-Bus Send&Received", BLACK);
  M5.Lcd.setCursor(10, 60, 4);
  
  CAN_cfg.speed = CAN_SPEED_500KBPS;
  CAN_cfg.tx_pin_id = TX;
  CAN_cfg.rx_pin_id = RX;

  CAN_cfg.rx_queue = xQueueCreate(10,sizeof(CAN_frame_t));
  
  // Init CAN Module
  ESP32Can.CANInit();



  M5.Lcd.startWrite();
  M5.Lcd.pushImage(0, 2, imgWidth, imgHeight, img);
  // 描画終了
  M5.Lcd.endWrite();
  // Wait
  delay(2000);

  header("CAN-Bus Send&Received", BLACK);
  
}

void loop() {
  static int current_rpm;
  static int current_temp;
  int i;

    
        
      M5.Lcd.setCursor(150, 10);
      M5.Lcd.setTextSize(2);
      M5.Lcd.print(M5.Axp.GetVBusVoltage());
      Serial.print(M5.Axp.GetVBusVoltage());

    

    /*
    CAN_frame_t rx_frame;
    //receive next CAN frame from queue
    if(xQueueReceive(CAN_cfg.rx_queue,&rx_frame, 3*portTICK_PERIOD_MS)==pdTRUE){
      if ((rx_frame.MsgID & 0xFFFFFF00) == 0x18daf100) {
        if ((rx_frame.data.u8[2])== 0x05) {
          current_temp = (int)((rx_frame.data.u8[3]) - 40);
          DisplayOut(current_temp,current_rpm);
        }else if ((rx_frame.data.u8[2])== 0x0c) {
          current_rpm=(int)(((int)rx_frame.data.u8[3])*256 + (int)rx_frame.data.u8[4])/4;
          DisplayOut(current_temp,current_rpm);
        }
      }
    }
    */

    if ((prev_time-millis())> 50) {
      prev_time=millis();


/*
      if (M5.Axp.GetVBusVoltage() == 0) {
        M5.Axp.PowerOff();
      }
*/
      if (loop_counter == ((loop_counter/4)*4)) { // if counter value is even
        rx_frame.FIR.B.FF = CAN_frame_ext;
        rx_frame.MsgID = 0x18db33f1 ;
        rx_frame.FIR.B.DLC = 3;
        rx_frame.data.u8[0] = 0x02;
        rx_frame.data.u8[1] = 0x01;
        rx_frame.data.u8[2] = 0x0C; // Request engine speed
        ESP32Can.CANWriteFrame(&rx_frame);
      } else if (loop_counter == 19) { // 1sec expired
        rx_frame.FIR.B.FF = CAN_frame_ext;
        rx_frame.MsgID = 0x18db33f1 ;
        rx_frame.FIR.B.DLC = 3;
        rx_frame.data.u8[0] = 0x02;
        rx_frame.data.u8[1] = 0x01;
        rx_frame.data.u8[2] = 0x05; // Request coolant temperature
        ESP32Can.CANWriteFrame(&rx_frame);
      }
      if (loop_counter >= 19) {
        loop_counter = 0;
      } else {
        loop_counter++;
      }
    }
//  if (M5.Axp.GetVBusVoltage() < 1) {
//    M5.Axp.PowerOff();
//  }


      M5.update();


}

void DisplayOut(int temp,int rpm) {
  char Str[3];
  static int PrevTemp;
    M5.Lcd.setTextFont(4);
    M5.Lcd.setCursor(70, 5);
    M5.Lcd.setTextSize(2);
    if (PrevTemp!=temp) {
      if (temp<50){
        M5.Lcd.setTextColor(TFT_BLUE, TFT_BLACK);
      } else if (temp<60) {
        M5.Lcd.setTextColor(TFT_GREEN, TFT_BLACK);
      } else if (temp<70) {
        M5.Lcd.setTextColor(TFT_YELLOW, TFT_BLACK);
      } else if (temp<80) {
        M5.Lcd.setTextColor(TFT_RED, TFT_BLACK);
      }
      M5.Lcd.print(temp);
      PrevTemp = temp;
    }
    M5.Lcd.fillRect(5,100,rpm*3/100,30,0xFFFF);
    M5.Lcd.fillRect((5+rpm*3/100),100,240,30,0x0000);

//    M5.Lcd.setCursor(150, 60);
//    M5.Lcd.print(rpm);
}
