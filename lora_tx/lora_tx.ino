
#include "EasyLink.h"
EasyLink_TxPacket txPacket;
EasyLink myLink;

#define HGM A5
#define LNA_EN A6
#define PA_EN A7

uint16_t value;
int analog_val;

// c_packet have 30 elements first 10 elements contains digits of number of packet
// transmitted successfully from transmitter
// in unsigned long int format
char c_packet[30] = {'0','0','0','0','0','0','0','0','0','0',';','S',';','0','0','0','0','0',';','0','0','0','0','0','0',' ','R','/','H','r'};
String s = ""; // string to contain sensor data
unsigned long int counter = 1; // number of packet sent successfully, range is [0, 4294967295] 10 digits beyond it max range counter will reset to 0
float t; // stores current time
float f = 1; // frequency of running the program
float dt = 1/f; // time period in seconds of running program i.e. void loop
float loop_timer = dt*1000000; // dt in microseconds
bool string_validity = false; // a valid string is received from the sensor


// Variables for sensors
int Signal;                // Store incoming ADC data. Value can range from 0-1024
int Threshold = 550;       // Determine which Signal to "count as a beat" and which to ignore.

const int trig = 38; // D20
const int echo = 37; // D19
int distance;  // This variable will store the object's distance from the sensor.

void setup() {
  Serial.begin(9600);

  // Ultrasonic
  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);

  // ECG
  pinMode(27, INPUT); // Setup for leads off detection LO + D19
  pinMode(28, INPUT); // Setup for leads off detection LO - D20

  // Code for data transmission
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  //Enable CC1190 Module ON
  pinMode(HGM, OUTPUT);
  pinMode(LNA_EN, OUTPUT);
  pinMode(PA_EN, OUTPUT);
  digitalWrite(HGM, HIGH);
  digitalWrite(LNA_EN, LOW);
  digitalWrite(PA_EN, HIGH);
  digitalWrite(RED_LED, LOW);

  digitalWrite(GREEN_LED, LOW);
  // begin defaults to EasyLink_Phy_50kbps2gfsk
  myLink.begin();
  // Set the destination address to 0xaa
  txPacket.dstAddr[0] = 0xaa;
  t = micros();
}

void loop() {
  //  int ecgdata = -1;

  // Heart Rate Sensor
  Signal = analogRead(A2); // Read the sensor value

  // Temperature Sensor
  float temp = analogRead(A3);

  // Ultrasonic Sensor
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);

  // ECG Sensor
  //  if ((digitalRead(27) == 1) || (digitalRead(28) == 1)) {
  //    Serial.println('!');
  //  }
  //  else {
  // send the value of analog input 0:
  int ecgdata = analogRead(A0);
  //    Serial.println(analogRead(A0));
  //  }

  // Calculating the distance by multiplying the speed of sound by the time of echo
  distance = pulseIn(echo, HIGH) * 0.034 / 2;
  String ultrasonic = String(distance) + " cm";
  Serial.println(ultrasonic);


  // read analog volt from sensor and save to variable temp
//    temp = temp * 0.48828125;
//  temp = log(((10240000 / temp) - 10000));
//  temp = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * temp * temp )) * temp );
//  temp = temp - 273.15;  
   temp = 60 - (((temp/204.8) - 0.5) * 10);
  // convert the analog volt to its temperature equivalent
  Serial.print("TEMPERATURE = ");
  Serial.print(temp); // display temperature value
  Serial.print("*C");
  Serial.println();

  String heartRateData = String(Signal / 10) + " BPM";
  Serial.println(heartRateData);                // Send the signal value to serial plotter

  // Collecting All Data
  String data = String(Signal / 10) + "-" + temp + "-" + String(distance) + "-" + String(ecgdata) + "!";
  Serial.println(data);

  // Code for transmission of data

  // Copy Counter Value to c_packet;
  String counter_str = String(counter);
  int n = counter_str.length();
  if(n>=10) {
    counter = 0;
    counter_str = "0";
  }
  int j=n-1;
//  Serial.println(counter_str);
  for(int i=9; i>=0 && j>=0; i--) {
    c_packet[i] = counter_str[j];
    j--; 
  }
  String test = "";
  for(int i=9; i>=0; i--) {
    test+=c_packet[i];
  }
  Serial.println(test);
  for (int i = 0; i <= data.length(); i++) // element no 10 is ; so copy in element no 11 to last i.e 29 so maximum 19 characters can be sent
  {
    c_packet[i + 11] = data[i];
  }
  // copy data to be sent
  memcpy(&txPacket.payload, &c_packet, sizeof(c_packet));
  // Set the length of the packet
  txPacket.len = sizeof(c_packet);
  // Transmit immediately
  txPacket.absTime = EasyLink_ms_To_RadioTime(0);
  // read transmission status
  EasyLink_Status status = myLink.transmit(&txPacket);
  if (status == EasyLink_Status_Success) // if transmission is successful
  {
    counter++; // increase data sent counter by one
    // string correct and data transmission successful
    // green led blink & red led off
    digitalWrite(GREEN_LED, HIGH);
    delay(50);
    digitalWrite(GREEN_LED, LOW);
    delay(50);
    digitalWrite(RED_LED, LOW);
  }
  else
  {
    // string correct but transmission failed
    // green led blink & red led on
    digitalWrite(GREEN_LED, HIGH);
    delay(50);
    digitalWrite(GREEN_LED, LOW);
    delay(50);
    digitalWrite(RED_LED, HIGH);
  }

  delay(1000);
}
