#include <SoftwareSerial.h> 
#include <TinyGPS.h> 
#include "ThingSpeak.h" 
#include <DallasTemperature.h> 
#include <dht.h>
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED}; EthernetClient client;
DallasTemperature sensors(&oneWire);
dht DHT;
#define DHT11_PIN 3
TinyGPS gps;
SoftwareSerial ss(19, 18);
const int xpin = A3;  // x-axis of the accelerometer

const int ypin = A2;  // y-axis

const int zpin = A1;  // z-axis (only on 3-axis models)

const int ecgpin = A4;

int pulsePin = A0;  // Pulse Sensor purple wire connected to analog

pin 0

volatile int BPM; // int that holds raw Analog in 0. updated every

2mS

28
 
volatile int Signal;  // holds the incoming raw data
volatile int IBI = 600; // int that holds the time interval between beats!
  
volatile boolean Pulse =  false;  // "True" when User's live heartbeat is
volatile boolean QS = false;  // becomes true when Arduoino finds a beat.
// Regards Serial OutPut  -- Set This Up to your needs

static boolean serialVisual = true; 

unsigned long myChannelNumber = xxxxx;

const char * myWriteAPIKey = "xxxxxxxxxxxxxx";

void setup()

{

Serial.begin(115200); //Begin serial communication ss.begin(9600);

Serial.println("healthcare"); //Print a message

sensors.begin();

Ethernet.begin(mac);

ThingSpeak.begin(client);

interruptSetup(); // sets up to read Pulse Sensor signal every 2mS

pinMode(10, INPUT); // Setup for leads off detection LO + pinMode(11, INPUT); // Setup for leads off detection LO -
}

void loop() {

float moisture, fahrenheit, pulse, acc, results[2], x, y,z, ecg ; // Send the command to get temperatures

fahrenheit = temperature (); Serial.print(fahrenheit); Serial.print(" Fahrenheit ");

moisture = humidity();

{

if( moisture >0)

{

z = moisture; Serial.print(z); Serial.print(" moisture ");

29
 
}

else

{

z= 69; Serial.print(z);
Serial.print(" moisture ");

}

}

pulse = heart(); Serial.print(BPM); Serial.print(" BPM ");

acc = acclerometer (); Serial.print(acc); Serial.print(" Acc ");

ecg = ECG(); Serial.print(ecg); Serial.println(" ECG ");

getData(results); if ( results[0] > 0 )

{

if ( results[1]> 0 )

{

x = results[0]; Serial.print( x ); Serial.print(" lat "); y = results[1] ; Serial.print( y ); Serial.println(" lon ");
}

}

//  Write to ThingSpeak. There are up to 8 fields in a channel, allowing you to store up to 8 different

//  pieces of information in a channel. Here, we write to field 1.

ThingSpeak.setField(1, fahrenheit );

ThingSpeak.setField(2, z );

ThingSpeak.setField(3, BPM );

ThingSpeak.setField(4, acc );

30
 
ThingSpeak.setField(5, x );

ThingSpeak.setField(6, y );

ThingSpeak.setField(7, ecg );

//  Write the fields that you've set all at once. ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

//  ThingSpeak will only accept updates every 15 seconds.

}

int temperature ()

{

sensors.requestTemperatures(); float temp, fahrenheit;

temp = sensors.getTempCByIndex(0); fahrenheit = temp * 1.8 + 32.0;
return fahrenheit;

}

int humidity()

{

float chk = DHT.read11(DHT11_PIN); return DHT.humidity;

}

int heart()

{

serialOutput() ;

if (QS == true){  // A Heartbeat Was Found

//  BPM and IBI have been Determined

//  Quantified Self "QS" true when arduino finds a heartbeat

if (serialVisual == true){  //  Code to Make the Serial Monitor

return BPM;



} else{

sendDataToSerial('B',BPM);  // send heart rate with a 'B' prefix

sendDataToSerial('Q',IBI);  // send time between beats with a 'Q' prefix

}

31
 
QS = false; // reset the Quantified Self flag for next time

}

}

float acclerometer()

{

float acc,xpin,ypin,zpin; xpin= analogRead(xpin); ypin= analogRead(ypin); zpin= analogRead(zpin);

xpin = map(xpin, 0, 1023, 0, 255); ypin = map(ypin, 0, 1023, 0, 255); zpin = map(zpin, 0, 1023, 0, 255);

acc = ( xpin + ypin + zpin)/3; return acc;

}

float ECG()

{

float ecg;

if((digitalRead(10) == 1)||(digitalRead(11) == 1)){ Serial.println('!');

}

else{

ecg = analogRead(ecgpin); return ecg;
}

}

void getData(float *dest)

{

bool newData = false; float flat, flon;

// For one second we parse GPS data and report some key values for (unsigned long start = millis(); millis() - start < 1000;)

{

while (ss.available())

{

32
 
char c = ss.read();

// Serial.write(c); // uncomment this line if you want to see the GPS data flowing

if (gps.encode(c)) // Did a new valid sentence come in? newData = true;

}

}

if (newData)

{

gps.f_get_position(&flat, &flon);

flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flat, 6; flon == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flon, 6;

}

dest[0] = flat; dest[1] = flon;

}

