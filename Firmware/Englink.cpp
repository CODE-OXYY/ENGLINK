#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "driver/twai.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define CAN_RX_PIN GPIO_NUM_4
#define CAN_TX_PIN GPIO_NUM_5

#define OBD_BROADCAST_ID 0x7DF
#define OBD_RESPONSE_ID  0x7E8

void setup() {
  Serial.begin(115200);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("Display failed :(");
    while(1);
  }
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 10);
  display.println("I am starting XD");
  display.display();

  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(CAN_TX_PIN, CAN_RX_PIN, TWAI_MODE_NORMAL);
  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
    if (twai_start() == ESP_OK) {
      display.println("CAN workingb :)");
      display.display();
    } else {
      display.println("CAN isnt working");
      display.display();
    }
  }
  delay(1000);
}

void loop() {
  twai_message_t tx_msg;
  tx_msg.identifier = OBD_BROADCAST_ID;
  tx_msg.extd = 0;
  tx_msg.data_length_code = 8;
  tx_msg.data[0] = 0x02; 
  tx_msg.data[1] = 0x01; 
  tx_msg.data[2] = 0x0C; 
  for(int i = 3; i < 8; i++) tx_msg.data[i] = 0; 

  twai_transmit(&tx_msg, pdMS_TO_TICKS(1000));

  twai_message_t rx_msg;
  if (twai_receive(&rx_msg, pdMS_TO_TICKS(200)) == ESP_OK) {
    if (rx_msg.identifier == OBD_RESPONSE_ID && rx_msg.data[2] == 0x0C) { 
      
      int rpm = ((rx_msg.data[3] * 256) + rx_msg.data[4]) / 4;
      
      display.clearDisplay();
      display.setCursor(0, 20);
      display.setTextSize(2);
      display.print("RPM: ");
      display.println(rpm);
      display.display();
      
      Serial.printf("RPM: %d\n", rpm);
    }
  }
  
  delay(100); 
}
