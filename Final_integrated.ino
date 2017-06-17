/*
  MarvinLaptop

  Basic controls of IoT Academy Marvin LoRa Development board through your Laptop

  This version supports:
  - Sending LoRa uplink messages using ABP that are given as input from the serial port on your laptop
  - Blink three times when sending data
  - Power control to RN2483 module

  Instructions:
  - Get the latest version of the Arduino software
  - In Arduino IDE select Arduino Leonardo and com port of your device
  - Please adjust ABP adresses and key below to match yours
*/
#include"WString.h"
#include"String.h"
#include"AirQuality.h"
#include"Arduino.h"
#include <math.h>

// Port to assign the type of lora data (any port can be used between 1 and 223)
int     set_port  = 1;

// Some standard ports that depend on the layout of the Marvin
long    defaultBaudRate = 57600;
int     reset_port = 5;
int     RN2483_power_port = 6;
int     led_port = 13;
long loudness_value = 0;

const int pinAdc = A4; //Sound sensor

/* Dust sensor*/
int dust_pin = 4;
unsigned long duration;
unsigned long starttime;
unsigned long sampletime_ms = 2000;//sampe 30s&nbsp;;
unsigned long lowpulseoccupancy = 0;
float ratio = 0;
float concentration = 0;
/* dust sensor*/

//*** Set parameters here BEGIN ---->
String  set_nwkskey = "14f8bc7262d957928f41bd3a71d98002";
String  set_appskey = "f9bccebfad1511f9613df05752b7b4de";
String  set_devaddr = "04001E47";
//*** <---- END Set parameters here


//******* Light sensor stuff--------
#define LIGHT_SENSOR A0               //Grove - Light Sensor is connected to A0 of Arduino
const int thresholdvalue=10;          //The treshold for which the LED should turn on. Setting it lower will make it go on at more light, higher for more darkness                       //Resistance of sensor in K
//------------------

//********** Air quality sensor stuff...........

AirQuality airqualitysensor;
int current_quality =-1;


/*
   Setup() function is called when board is started. Marvin uses a serial connection to talk to your pc and a serial
   connection to talk to the RN2483, these are both initialized in seperate functions. Also some Arduino port are
   initialized and a LED is called to blink when everything is done.
*/
void setup() {
  InitializeSerials(defaultBaudRate);
  initializeRN2483(RN2483_power_port, reset_port);
  pinMode(led_port, OUTPUT); // Initialize LED port
  blinky();
  pinMode(dust_pin,INPUT);
  starttime = millis();//get the current time;

//Air quality:
    airqualitysensor.init(14);

  
}

void loop() {

//----------- Light sensor stuff---------
    int sensorValue = analogRead(LIGHT_SENSOR); 
    print_to_console("Light sensor value read: ");
    print_to_console(String(sensorValue));

//------------ Air quality:
    current_quality=airqualitysensor.slope();
    if (current_quality >= 0)// if a valid data returned.
    {
        if (current_quality==0) 
            print_to_console("High pollution! Force signal active");
        else if (current_quality==1)
            print_to_console("High pollution!");
        else if (current_quality==2)
            print_to_console("Low pollution!");
        else if (current_quality ==3)
            print_to_console("Fresh air");
    }

/* Loudness sensor*/
    long Loudness_value = 0;
    for(int i=0; i<32; i++)
    {
        loudness_value += analogRead(pinAdc);
    }

    loudness_value >>= 5;

    Serial.println("Loudness value ");
    Serial.println(loudness_value);
    delay(10);
    
   /* loudness sensor*/

 //Dust sensor.....
  duration = pulseIn(dust_pin, LOW);
  lowpulseoccupancy = lowpulseoccupancy+duration;

  if ((millis()-starttime) >= sampletime_ms)//if the sampel time = = 30s
  {
    ratio = lowpulseoccupancy/(sampletime_ms*10.0);  // Integer percentage 0=&gt;100
    concentration = 1.1*pow(ratio,3)-3.8*pow(ratio,2)+520*ratio+0.62; // using spec sheet curve
    Serial.print("concentration = ");
    Serial.print(concentration);
    Serial.println(" pcs/0.01cf");
    Serial.println("\n");
    lowpulseoccupancy = 0;
    starttime = millis();
  }
   
    print_to_console("Sending data to LoRa");
    
    send_LoRa_data(set_port, String(sensorValue));
    send_LoRa_data(set_port, String(current_quality));
    send_LoRa_data(set_port, String(loudness_value));
     send_LoRa_data(set_port, String(concentration));
    
    print_to_console("Sent to LoRa");
    read_data_from_LoRa_Mod();

}


//ISR for Air quality sensor
ISR(TIMER1_OVF_vect)
{
  if(airqualitysensor.counter==61)//set 2 seconds as a detected duty
  {
      airqualitysensor.last_vol=airqualitysensor.first_vol;
      airqualitysensor.first_vol=analogRead(A2);
      airqualitysensor.counter=0;
      airqualitysensor.timer_index=1;
      PORTB=PORTB^0x20;
  }
  else
  {
    airqualitysensor.counter++;
  }
}


void InitializeSerials(long baudrate)
{
  Serial.begin(baudrate);
  Serial1.begin(baudrate);
  delay(1000);
  print_to_console("Serial ports initialised");
}

void initializeRN2483(int pwr_port, int rst_port)
{
  //Enable power to the RN2483
  pinMode(pwr_port, OUTPUT);
  digitalWrite(pwr_port, HIGH);
  print_to_console("RN2483 Powered up");
  delay(1000);

  //Disable reset pin
  pinMode(rst_port, OUTPUT);
  digitalWrite(rst_port, HIGH);

  //Configure LoRa module
  send_LoRa_Command("sys reset");
  read_data_from_LoRa_Mod();

  send_LoRa_Command("mac set nwkskey " + set_nwkskey);
  read_data_from_LoRa_Mod();

  send_LoRa_Command("mac set appskey " + set_appskey);
  read_data_from_LoRa_Mod();

  send_LoRa_Command("mac set devaddr " + set_devaddr);
  read_data_from_LoRa_Mod();

  //For this commands some extra delay is needed.
  send_LoRa_Command("mac set adr on");
  //send_LoRa_Command("mac set dr 0"); //uncomment this line to fix the RN2483 on SF12 (dr=DataRate)
  delay(1000);
  read_data_from_LoRa_Mod();

  send_LoRa_Command("mac save");
  delay(1000);
  read_data_from_LoRa_Mod();

  send_LoRa_Command("mac join abp");
  delay(1000);
  read_data_from_LoRa_Mod();

  send_LoRa_Command("radio set crc off");
  delay(1000);
  read_data_from_LoRa_Mod();

}

void print_to_console(String message)
{
  Serial.println(message);
}

void read_data_from_LoRa_Mod()
{
  if (Serial1.available()) {
    String inByte = Serial1.readString();
    print_to_console("Got this from Lora");
    Serial.println(inByte);

  }

}

void send_LoRa_Command(String cmd)
{
  print_to_console("Now sending: " + cmd);
  Serial1.println(cmd);
  delay(500);
}

void send_LoRa_data(int tx_port, String rawdata)
{
  send_LoRa_Command("mac tx uncnf " + String(tx_port) + String(" ") + rawdata);
}


void blinky()
{
  digitalWrite(led_port, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(500);              // wait for a second
  digitalWrite(led_port, LOW);    // turn the LED off by making the voltage LOW
  delay(500);              // wait for a second
  digitalWrite(led_port, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(500);              // wait for a second
  digitalWrite(led_port, LOW);    // turn the LED off by making the voltage LOW
  delay(500);              // wait for a second

}


