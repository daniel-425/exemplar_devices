#include <Ethernet.h>

// Setup our MAC address
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

// Setup our own network information. Eventaully DHCP
IPAddress ip(192, 168 , 7, 150);
IPAddress gateway(192, 168, 7, 1);
IPAddress subnet(255, 255, 255, 0);

char packetBuffer[UDP_TX_PACKET_MAX_SIZE];

const int BTN_1_PIN = 10;
const int BTN_2_PIN = 11;
const int BTN_3_PIN = 12;
const int BTN_4_PIN = 13;

const int LED_RED_PIN = 14;
const int LED_GREEN_PIN = 15;
const int LED_BLUE_PIN = 16;

const int SERVER_PORT = 1234;

bool ready = false; 

bool server_active = false;
EthernetClient client;

void setup() {
  // Configure the ethernet shield
  Ethernet.init(10);  // Most Arduino shields

  // Set our pins
  pinMode(BTN_1_PIN, INPUT);
  pinMode(BTN_2_PIN, INPUT);
  pinMode(BTN_3_PIN, INPUT);
  pinMode(BTN_4_PIN, INPUT);
  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(LED_GREEN_PIN, OUTPUT);
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
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    while (true) {
      delay(1); // do nothing, no point running without Ethernet hardware
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
    
    if (true)  {
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

      // If button 1 has been pushed open the tcp server and do a recv 
      if (button_1_value = HIGH){
        // Enable our server to be active. 
        server_active = true;
      }

      // Check if any clients are trying to connect
      EthernetClient newClient = server.accept();
      if (newClient) {
        Serial.print("New client connecting");
        client = newClient;
      }
      if (client){
        if (server_active){
          // Accept clients and perform a single read
          if (client.avaiable > 0)
          {
            packetBuffer = client.read();
  
            Serial.print("Recv from client\n");
            Serial.write(packetBuffer);
  
            // Split the data and do the memcpy.
            
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
    }
    else{
      Serial.println("Ethernet not connected");
    }
}
