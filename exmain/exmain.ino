#include <Ethernet.h>
#include<string.h>
#include<stdio.h>

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

  // Start the Ethernet
  Ethernet.begin(mac, ip);
  EthernetServer server(SERVER_PORT); //server port

  // Open Serial communications and wait for port to open:
  Serial.begin(9600);

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
}

void loop() {
    // Check if we hace an ethernet link. 
    if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernet cable is not connected.");
      ready = true;
    }
    else{
      ready = true;
    }

    // Hack 
    //ready = true;
    
    if (ready)  {
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

      // Scenario 1 code
      // Check if any clients are trying to connect
      EthernetClient newClient = server.accept();
      if (newClient) {
        Serial.print("New client connecting");
        client = newClient;
      }
      
      if (client){
        if (server_active){
          // Reset our read buffer 
          memset(packetBuffer, 0, 256);
          
          // Accept clients and perform a single read
          if (client.available() > 0)
          {
            while(true){
              for(int a = 0; a < 256; a++)
              {
                packetBuffer[a] = client.read();

                if (packetBuffer[a] < 0){
                  break;
                }
              }

              Serial.write(packetBuffer);
              Serial.write("\n");
            }
  
            // Split the data and do the memcpy.
            // Data format = XXXX:YYYYYYYYYY
            int delim_loc = strchr(packetBuffer, ':');

            char * size_string = strcpy(packetBuffer, delim_loc);
            char * data_string = strcpy(packetBuffer+delim_loc, strlen(packetBuffer));

            Serial.print("Size String: ");
            Serial.write(size_string);
            Serial.print("\nData String: ");
            Serial.write(data_string);

            // Convert size to int
            int data_size = atoi(size_string);

            // Do the unsafe copy. 
            char * data = (char *)malloc(data_size);
            strcpy(data, data_string);
            
            // The communication has been consumed.
            server_active = false;
          }
        }
        else{
          // Were not active so reject the client
          client.write("REJECT");
          client.stop();
        }
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
        analogWrite(LED_RED_PIN, 0);
        analogWrite(LED_YELLOW_PIN, 0);
        analogWrite(LED_BLUE_PIN, 10);
      }
      else{
        Serial.println("False");
        analogWrite(LED_RED_PIN, 10);
        analogWrite(LED_YELLOW_PIN, 0);
        analogWrite(LED_BLUE_PIN, 0);
      }

      // Update the buttons
      button_1_prev_value = button_1_value;
      button_2_prev_value = button_2_value;
      button_3_prev_value = button_3_value;
      button_4_prev_value = button_4_value;
      
    }
    else{
      Serial.println("Ethernet not connected");
    }
}
