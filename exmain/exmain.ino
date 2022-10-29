#include <Ethernet.h>
#include<string.h>
#include<stdio.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>

// Setup our MAC address
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xFE, 0xEF, 0xEE
};

// Setup our own network information. Eventaully DHCP
IPAddress ip(192, 168 , 7, 150);
IPAddress gateway(192, 168, 7, 1);
IPAddress subnet(255, 255, 255, 0);

char packetBuffer[256];

const int BTN_1_PIN = 7;
const int BTN_2_PIN = 8;
const int BTN_3_PIN = 9;
const int BTN_4_PIN = 12;

const int LED_RED_PIN = 3;
const int LED_YELLOW_PIN = 5;
const int LED_BLUE_PIN = 6;

const int SERVER_PORT = 1234;

int button_1_state = LOW;
int button_2_state = LOW;
int button_3_state = LOW;
int button_4_state = LOW;

int button_1_prev_value = LOW;
int button_2_prev_value = LOW;
int button_3_prev_value = LOW;
int button_4_prev_value = LOW;
      
bool ready = false; 

int secret_mode = false;

bool server_active = false;
EthernetClient client;
EthernetServer server(1234);

int sequence_pos = 0;

ISR(WDT_vect)
{
  digitalWrite(LED_RED_PIN, LOW);
  digitalWrite(LED_YELLOW_PIN, LOW);
  digitalWrite(LED_BLUE_PIN, LOW);
}

void setup() {  
  // Configure the ethernet shield
  Ethernet.init(10);  // Most Arduino shields

  // Set our pins
  pinMode(BTN_1_PIN, INPUT);
  pinMode(BTN_2_PIN, INPUT);
  pinMode(BTN_3_PIN, INPUT);
  pinMode(BTN_4_PIN, INPUT);
  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(LED_YELLOW_PIN, OUTPUT);
  pinMode(LED_BLUE_PIN, OUTPUT);

  // Setup Serial
  Serial.begin(9600);
  //Serial.println("\nReset!\n");

  // Flash the LEDS for reboot and simulate a boot delay.
  digitalWrite(LED_RED_PIN, LOW);
  digitalWrite(LED_YELLOW_PIN, LOW);
  digitalWrite(LED_BLUE_PIN, LOW);
  //delay(500);

  // Start the Ethernet
  Ethernet.begin(mac, ip);
  EthernetServer server(SERVER_PORT); //server port

  // For prototype only
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield not found!");
    while (true) {
      delay(1); 
    }
  }

  server.begin();
  wdt_enable(WDTO_1S);
}

void loop() {
    // Check if we hace an ethernet link. 
    if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernet cable is not connected.");
    }
    
    // Get button values
    int button_1_value = digitalRead(BTN_1_PIN);
    int button_2_value = digitalRead(BTN_2_PIN);
    int button_3_value = digitalRead(BTN_3_PIN);
    int button_4_value = digitalRead(BTN_4_PIN);

    Serial.write("Button1 Value: ");
    Serial.print(button_1_value);
    Serial.write("\n");
    Serial.write("Button2 Value: ");
    Serial.print(button_2_value);
    Serial.write("\n");
    Serial.write("Button3 Value: ");
    Serial.print(button_3_value);
    Serial.write("\n");
    Serial.write("Button4 Value: ");
    Serial.print(button_4_value);
    Serial.write("\n");

    if ((button_1_value == HIGH) && (button_1_prev_value == LOW)){
      button_1_state = HIGH;
    }
    else {
      button_1_state = LOW;
    }

    if ((button_2_value == HIGH) && (button_2_prev_value == LOW)){
      button_2_state = HIGH;
    }
    else {
      button_2_state = LOW;
    }

    if ((button_3_value == HIGH) && (button_3_prev_value == LOW)){
      button_3_state = HIGH;
    }
    else {
      button_3_state = LOW;
    }

    if ((button_4_value == HIGH) && (button_4_prev_value == LOW)){
      button_4_state = HIGH;
    }
    else {
      button_4_state = LOW;
    }

    // If button 1 has been pushed open the tcp server and do a recv 
    if (button_1_state == HIGH){
      // Enable our server to be active. 
      server_active = true;
    }
    else if (button_2_state == HIGH){
      server_active = false;
    }

    if (server_active){
        digitalWrite(LED_RED_PIN, LOW);
        digitalWrite(LED_YELLOW_PIN, HIGH);
        digitalWrite(LED_BLUE_PIN, LOW);
    }
    else{
        digitalWrite(LED_RED_PIN, HIGH);
        digitalWrite(LED_YELLOW_PIN, LOW);
        digitalWrite(LED_BLUE_PIN, LOW);
    }

    // Hack
    //server_active = true;

    // Scenario 1 code
    // Check if any clients are trying to connect
    EthernetClient newClient = server.accept();
    if (newClient) {
      Serial.println("New client connecting");
      client = newClient;
    }
    
    if (client){
      if (server_active){
        Serial.println("Server active");
      
        // Reset our read buffer 
        memset(packetBuffer, 0, 256);
        
        // Accept clients and perform a single read
        bool need_to_read = true;
        if (client.available() > 0)
        {
          while(need_to_read){
            for(int a = 0; a < 256; a++)
            {
              packetBuffer[a] = client.read();

              if (packetBuffer[a] < 0){
                Serial.println("Packet read finished");
                need_to_read = false;
                break;
              }
            }
            Serial.println("PacketBuffer: ");
            Serial.println(packetBuffer);
          }

          // Split the data and do the memcpy.
          // Data format = XXXX:YYYYYYYYYY
          char size_string[4];
          char data_string[256];

          memset(&size_string, 0, 4);
          memset(&data_string, 0, 256);
          
          memcpy(size_string, packetBuffer, 4);
          memcpy(data_string, packetBuffer+5, strlen(packetBuffer+5)-1);

          Serial.print("Size String: ");
          Serial.println(size_string);
          Serial.print("Data String: ");
          Serial.println(data_string);

          // Convert size to int
          int data_size = atoi(size_string);

          Serial.print("User defined size: ");
          Serial.println(data_size);
          Serial.print("Actual data size: ");
          Serial.print(strlen(data_string));
          Serial.print("\n\n\n\n\n\n\n\n");

          char data2[50];
          // Do the unsafe copy. 
          char * data = (char *)malloc(data_size);
          strcpy(data2, data_string);
          
          // The communication has been consumed.
          server_active = false;
        }
      }
      else{
        // Were not active so reject the client
        client.write("REJECT");
      }

      client.stop();
    }

    // Scenario 2 code. 
    // Secret combination using button 2, 3 and 4. 2343434343. Sequence of 10 so 1024 conbinations.
    Serial.print("Current Sequence Pos: ");
    Serial.print(sequence_pos);
    Serial.print("\n");
    if (button_1_state || button_2_state || button_3_state || button_4_state){
      if (sequence_pos == 0){
        if (button_2_state == HIGH){
          sequence_pos = 1;
        }
        else{
          sequence_pos = 0;
        }
      }
      else if (sequence_pos == 1){
        if (button_3_state == HIGH){
          sequence_pos = 2;
        }
        else{
          if (button_2_state == HIGH){
            sequence_pos = 1;
          }
          else{
            sequence_pos = 0;
          }
        }
      }
      else if (sequence_pos == 2){
        if (button_4_state == HIGH){
          sequence_pos = 3;
        }
        else{
          if (button_2_state == HIGH){
            sequence_pos = 1;
          }
          else{
            sequence_pos = 0;
          }
        }
      }
      else if (sequence_pos == 3){
        if (button_3_state == HIGH){
          sequence_pos = 4;
        }
        else{
          if (button_2_state == HIGH){
            sequence_pos = 1;
          }
          else{
            sequence_pos = 0;
          }
        }
      }
      else if (sequence_pos == 4){
        if (button_4_state == HIGH){
          sequence_pos = 5;
        }
        else{
          if (button_2_state == HIGH){
            sequence_pos = 1;
          }
          else{
            sequence_pos = 0;
          }
        }
      }
      else if (sequence_pos == 5){
        if (button_3_state == HIGH){
          sequence_pos = 6;
        }
        else{
          if (button_2_state == HIGH){
            sequence_pos = 1;
          }
          else{
            sequence_pos = 0;
          }
        }
      }
      else if (sequence_pos == 6){
        if (button_4_state == HIGH){
          sequence_pos = 7;
        }
        else{
          if (button_2_state == HIGH){
            sequence_pos = 1;
          }
          else{
            sequence_pos = 0;
          }
        }
      }
      else if (sequence_pos == 7){
        if (button_3_state == HIGH){
          sequence_pos = 8;
        }
        else{
          if (button_2_state == HIGH){
            sequence_pos = 1;
          }
          else{
            sequence_pos = 0;
          }
        }
      }
      else if (sequence_pos == 8){
        if (button_4_state == HIGH){
          sequence_pos = 9;
        }
        else{
          if (button_2_state == HIGH){
            sequence_pos = 1;
          }
          else{
            sequence_pos = 0;
          }
        }
      }
      else if (sequence_pos == 9){
        if (button_3_state == HIGH){
          // Success! 
          Serial.println("Entering secret mode");
          secret_mode = true;
        }
        else{
          if (button_2_state == HIGH){
            sequence_pos = 1;
          }
          else{
            sequence_pos = 0;
          }
        }
      }
      else{
        Serial.println("Not a state");
        sequence_pos = 0;
      }
    }

    Serial.print("Secret Mode: ");
    if (secret_mode){
      Serial.println("True");
      
      // Make the LED Green 
      digitalWrite(LED_RED_PIN, LOW);
      digitalWrite(LED_YELLOW_PIN, LOW);
      digitalWrite(LED_BLUE_PIN, HIGH);
    }

    // Update the buttons
    button_1_prev_value = button_1_value;
    button_2_prev_value = button_2_value;
    button_3_prev_value = button_3_value;
    button_4_prev_value = button_4_value;

    // Feed the watchdog
    wdt_reset();
}
