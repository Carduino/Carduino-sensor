//#include <Printers.h>
#include <XBee.h>
#include <SoftwareSerial.h>

/*
>> Pulse Sensor Amped 1.1 <<
    www.pulsesensor.com
    >>> Pulse Sensor purple wire goes to Analog Pin 0 <<<
Pulse Sensor sample aquisition happens in the background via Timer 2 interrupt. 2mS sample rate.
PWM on pins 3 and 11 will not work when using this code, because we are using Timer 2!
The following variables are automatically updated:
Signal :    int that holds the analog signal data straight from the sensor. updated every 2mS.
IBI  :      int that holds the time interval between beats. 2mS resolution.
BPM  :      int that holds the heart rate value, derived every beat, from averaging previous 10 IBI values.
QS  :       boolean that is made true whenever Pulse is found and BPM is updated. User must reset.
Pulse :     boolean that is true when a heartbeat is sensed then false in time with pin13 LED going out.
*/

//  VARIABLES
int pulsePin = 0;                 // Pulse Sensor purple wire connected to analog pin 0
int Batterie = 70;                // % of battery (it's an indication for the moment)

// these variables are volatile because they are used during the interrupt service routine!
volatile int BPM;                   // used to hold the pulse rate
volatile int Signal;                // holds the incoming raw data
volatile int IBI = 600;             // holds the time between beats, the Inter-Beat Interval
volatile boolean Pulse = false;     // true when pulse wave is high, false when it's low
volatile boolean QS = false;        // becomes true when Arduoino finds a beat.

XBee xbee = XBee(); //Create a Xbee object with classe Xbee
// XBee's DOUT (TX) is connected to pin 5 (Arduino's Software RX)
// XBee's DIN (RX) is connected to pin 6 (Arduino's Software TX)
SoftwareSerial soft(5, 6); // RX, TX

char RecBuffer[200]; 
Rx16Response rx16 = Rx16Response(); // Represents a Series 1 16-bit address RX packet
ZBRxResponse rx = ZBRxResponse();   //Represents a Series 2 RX packet

XBeeAddress64 addr64 = XBeeAddress64(0x0013a200, 0x40a1eee2); //Default address of hub
String myString = "";

void setup() {
  Serial.begin(9600);             // talk at 9600 bds with hardware serial port
  interruptSetup();               // sets up to read Pulse Sensor signal every 2mS

  //Xbee API
  soft.begin(9600);               // talk at 9600 bds with software serail port
  xbee.setSerial(soft);           // set the serial port of Xbee

}


void loop() {
  
//RECEIVE DATA
  /*xbee.readPacket();                                           // Reads all available serial bytes until a packets is parsed
  if (xbee.getResponse().isAvailable()) {                        // Determine if the packets is ready

      // Get data of the receive packet
      for (int i = 0; i < rx.getDataLength(); i++) {
          if (iscntrl(rx.getData()[i]))
            RecBuffer[i] =' ';
          else
            RecBuffer[i]=rx.getData()[i];
        }
        
        String myString = String((char *)RecBuffer);          // Conversion of RecBuffer in String
        if(myString == "HUB ADDRESS"){                        //Compare String data to String HUB ADDRESS
          addr64 = rx.getRemoteAddress64();                   //Change destination address with the broadcast packets address
          Serial.println("Address updated");                  // Notify the user of the address udapte
        }
  }*/


//SEND DATA
  if (QS == true) {                      // Quantified Self flag is true when arduino finds a heartbeat
    if (BPM < 220) {                     // Limit of real BPM

      Serial.println(BPM);
      String message = "StrapV1," + (String)Batterie + "," + (String)BPM + ","; //Message construction in String format
                                                                                // Nom + %Battery + BPM
      uint8_t payload[20] = "";         //Creation of the futur send buffer
      message.getBytes(payload, 20);    //Get the String message in the buffer playload

      ZBTxRequest zbtx = ZBTxRequest(addr64, payload, sizeof(payload)); //Represents a Series 2 TX packet that corresponds to Api Id: ZB_TX_REQUEST
      xbee.send(zbtx);                                                  // Send the packet to the hub
    }
  }
  QS = false;                      // reset the Quantified Self flag for next time

  //clear the char array, other wise data remains in the 
  //buffer and creates unwanted results.
  memset(&RecBuffer[0], 0, sizeof(RecBuffer));
  
  delay(20); //take a break!
}
