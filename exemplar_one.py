import RPi.GPIO as GPIO
import socket

# Define pins 
BUTTON_PIN = 10
RED_LED_PIN = 11
BLUE_LED_PIN = 12
GREEN_LED_PIN = 13

# Pin Setup
GPIO.setup(BUTTON_PIN, GPIO.IN)
GPIO.setup(RED_LED_PIN, GPIO.OUT)
GPIO.setup(BLUE_LED_PIN, GPIO.OUT) 
GPIO.setup(GREEN_LED_PIN, GPIO.OUT) 

# Initial state for LEDs:
GPIO.output(RED_LED_PIN, GPIO.LOW)
GPIO.output(BLUE_LED_PIN, GPIO.LOW)
GPIO.output(GREEN_LED_PIN, GPIO.HIGH)

SERVICE_PORT = "5555"
SERVICE_HOST = ""

# Start our service
print("Starting!")

try:
    while 1:
        if GPIO.input(BUTTON_PIN):
            print("Button has been pushed!")

            # Set our LED to blue to show the service has started 
            GPIO.output(RED_LED_PIN, GPIO.LOW)
            GPIO.output(BLUE_LED_PIN, GPIO.HIGH)
            GPIO.output(GREEN_LED_PIN, GPIO.LOW)

            # Start the vulnerable service 
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                s.bind((SERVICE_HOST, SERVICE_PORT))
                s.listen()
                conn, addr = s.accept()
                with conn:
                    print("Connected by {0}".format(addr))
                    while True:
                        data = conn.recv(1024)
                        if not data:
                            break
                        
                        # Data should be 4 bytes size then 4 bytes data 
                        size = data[:4]
                        payload = data[4:]

                        print("Size: {0} Payload: {1}".format(size, payload))

                        # Fake a buffer overflow 
                        if size < len(payload):
                            print("UNSAFE OVERFLOW OMG")
                            GPIO.output(RED_LED_PIN, GPIO.HIGH)
                            GPIO.output(BLUE_LED_PIN, GPIO.LOW)
                            GPIO.output(GREEN_LED_PIN, GPIO.LOW)
                        else: 
                            print("Safe communication")

except KeyboardInterrupt: # If CTRL+C is pressed, exit cleanly:
    GPIO.cleanup() # cleanup all GPIO
