// program for Rx
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "EasyLink.h"
EasyLink_RxPacket rxPacket;
EasyLink myLink;
// char c_packet[19]; // number of packet sent by transmitter(5 digits); analog value(4 digits) 
char c_packet[30];
#define HGM A5    //
#define LNA_EN A6 // low noise amplifier pin
#define PA_EN A7  // power amplifier enable pin
float t;          // stores current time in microseconds
unsigned long int num_packet_received_from_tx = 0;
unsigned long int num_packet_sent_from_tx = 0;
unsigned long int num_of_packet_loss = 0;
float percentage_of_packet_loss = 0;
unsigned long int num_packet_received_from_tx1 = 0;
unsigned long int num_packet_sent_from_tx1 = 0;
unsigned long int num_of_packet_loss1 = 0;
float percentage_of_packet_loss1 = 0;
unsigned long int num_packet_received_from_tx2 = 0;
unsigned long int num_packet_sent_from_tx2 = 0;
unsigned long int num_of_packet_loss2 = 0;
float percentage_of_packet_loss2 = 0;

void setup()
{
    Serial.begin(9600);
    // begin defaults to EasyLink_Phy_50kbps2gfsk
    // Enable c_packet1190 Module ON
    pinMode(HGM, OUTPUT);
    pinMode(LNA_EN, OUTPUT);
    pinMode(PA_EN, OUTPUT);
    digitalWrite(HGM, HIGH);
    digitalWrite(LNA_EN, HIGH);
    digitalWrite(PA_EN, LOW);
    myLink.begin();
    Serial.println(myLink.version());
    t = micros();
}

unsigned long long int getNumOfPacketSent(char data[]) {
  unsigned long long int num=0;
  int itr=0;
  for(int i=9; i>=0; i--) {
    num+=(data[i]-'0')*pow(10, itr);
    itr++;
  }
  return num;
}

void loop()
{
    for (int i = 0; i < 30; i++)
    {
        c_packet[i] = '-';
    }
    // rxTimeout is in Radio time and needs to be converted from miliseconds to RadioTime
    rxPacket.rxTimeout = EasyLink_ms_To_RadioTime(3000);
    // Turn the receiver on immediately
    rxPacket.absTime = EasyLink_ms_To_RadioTime(0);
    // check status of receiving and decoding the received packet
    EasyLink_Status status = myLink.receive(&rxPacket);
    if (status == EasyLink_Status_Success)
    {
        memcpy(&c_packet, &rxPacket.payload, sizeof(c_packet)); // copy receive payload to c_packet                   
        // if payload received is from device 170 (it is the id of device Tx (0Xaa))
        int tx_addr = rxPacket.dstAddr[0];
        Serial.print("Transmitter Address: ");
        Serial.print(tx_addr);
        Serial.println();
        
//        num_packet_sent_from_tx = 1 * (int(c_packet[9]) - 48) + 10 * (int(c_packet[8]) - 48) + 100 * (int(c_packet[7]) - 48) + 1000 * (int(c_packet[6]) - 48) + 10000 * (int(c_packet[5]) - 48) + 100000 * (int(c_packet[4]) - 48) + 1000000 * (int(c_packet[3]) - 48) + 10000000 * (int(c_packet[2]) - 48) + 100000000 * (int(c_packet[1]) - 48) + 1000000000 * (int(c_packet[0]) - 48);
        num_packet_sent_from_tx = getNumOfPacketSent(c_packet);
//Serial.print(c_packet[9]);
        Serial.print("Packets Sent: ");
        Serial.print(num_packet_sent_from_tx);
        Serial.println();

        num_packet_received_from_tx++;
        if (num_packet_received_from_tx > num_packet_sent_from_tx) // in case the tx0 is reset in between the communication to avoid negative packet loss
        {
            num_packet_received_from_tx = num_packet_sent_from_tx;
        }

        Serial.print("Packets Received: ");
        Serial.print(num_packet_received_from_tx);
        Serial.println();

        num_of_packet_loss = num_packet_sent_from_tx -
                             num_packet_received_from_tx;
        float percentage_of_packet_loss = ((float)num_of_packet_loss / (float)num_packet_sent_from_tx) * 100;
        
        Serial.print("% Packet Lost: ");
        Serial.print(percentage_of_packet_loss);
        Serial.println();

        // print sensor data
        Serial.println("Sensor Data:");
        for (int i = 11; i <= 30; i++)
        {
            Serial.print(c_packet[i]);
        }
        Serial.println();
    }
    else {
      Serial.println("Error");
    }   
}
