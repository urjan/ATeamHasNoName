/*
MarvinBase
Basic controls of IoT Academy Marvin LoRa Development board.
This version supports:
  - Sending LoRa uplink messages using ABP 
  - Blink three times when sending data
  - Power control to RN2483 module
Instructions:
  - Get the latest version of the Arduino software
  - In Arduino IDE select Arduino Leonardo and com port of your device
  - Please adjust ABP adresses and key below to match yours
  - The loop() is where the actual stuff happens. Adjust input of send_lora_data() in void loop() to send your own data.
*/
// Port to assign the type of lora data (any port can be used between 1 and 223)
int     set_port  = 1;

// Some standard ports that depend on the layout of the Marvin
long    defaultBaudRate = 57600;
int     reset_port = 5;
int     RN2483_power_port = 6; //Note that an earlier version of the Marvin doesn't support seperate power to RN2483NA
int     led_port = 13;
float loudness_value = 0.0;
int dust_pin = 8;
unsigned long duration;
unsigned long starttime_dust;
unsigned long sampletime_ms = 2000;//sampe 30s&nbsp;;
unsigned long lowpulseoccupancy = 0;
float ratio = 0;
float concentration = 0;


//*** Set parameters here BEGIN ---->
String  set_nwkskey = "14f8bc7262d957928f41bd3a71d98002";
String  set_appskey = "f9bccebfad1511f9613df05752b7b4de";
String  set_devaddr = "04001E46";
//*** <---- END Set parameters here

//** Set thigs right for the Grove temperature / humidity sensor
//#include "DHT.h"      //download it here: https://github.com/Seeed-Studio/Grove_Temperature_And_Humidity_Sensor
                      // press clone/download and then download as .zip

#include"AirQuality.h"
#include "math.h"  //SG
#include "SoftwareSerial.h"
//#define DHTPIN A3     // A3 is closes to the usb port of Marvin

// define the type of sensor used (there are others)
//#define DHTTYPE DHT11   // DHT 11 
#define LIGHT_SENSOR A0//Grove - Light Sensor is connected to A0 of Arduino
const int light_thresholdvalue=10;         //The treshold for which the LED should turn on. Setting it lower will make it go on at more light, higher for more darkness
AirQuality airqualitysensor;
int current_quality =-1;

SoftwareSerial SoftSerial(2, 3);
unsigned char buffer[64];                   // buffer array for data receive over serial port
int count=0;                                // counter for buffer array

//DHT dht(DHTPIN, DHTTYPE);

/*
 * Setup() function is called when board is started. Marvin uses a serial connection to talk to your pc and a serial
 * connection to talk to the RN2483, these are both initialized in seperate functions. Also some Arduino port are 
 * initialized and a LED is called to blink when everything is done. 
 */

 //SG
const int B = 4275;               // B value of the thermistor
const int R0 = 100000;            // R0 = 100k
const int pinTempSensor = A5;     // Grove - Temperature Sensor connect to A5
//SG

void setup() {

  Serial.begin(defaultBaudRate);
  Serial1.begin(defaultBaudRate);
  InitializeSerials(defaultBaudRate);
  initializeRN2483(RN2483_power_port, reset_port);
  pinMode(led_port, OUTPUT); // Initialize LED port  
  airqualitysensor.init(14);
  SoftSerial.begin(9600);    
  //dht.begin();
  blinky();
}

void loop() {
/*    float h = dht.readHumidity();
    float t = dht.readTemperature();
    int a = analogRead(pinTempSensor); //SG

    float R = 1023.0/a-1.0;
    R = R0*R;

    float temperature = 1.0/(log(R/R0)/B+1/298.15)-273.15; // convert to temperature via datasheet

    Serial.print("temperature = ");
    Serial.println(temperature);//SG*/

/*GPS*/
    if (SoftSerial.available())                     // if date is coming from software serial port ==> data is coming from SoftSerial shield
    {
        while(SoftSerial.available())               // reading data into char array
        {
            buffer[count++]=SoftSerial.read();      // writing data into array
            if(count == 64)break;
        }
       
        Serial.write(buffer,count);                 // if no data transmission ends, write buffer to hardware serial port
        clearBufferArray();                         // call clearBufferArray function to clear the stored data from the array
        count = 0;                                  // set counter of while loop to zero 
    }
    if (Serial.available())                 // if data is available on hardware serial port ==> data is coming from PC or notebook
    SoftSerial.write(Serial.read());        // write it to the SoftSerial shield

   // Serial.write(buffer,count);                 // if no data transmission ends, write buffer to hardware serial port
        
/*GPS*/
    
/* are quality sensor*/
    current_quality=airqualitysensor.slope();
    if (current_quality >= 0)// if a valid data returned.
    {
        if (current_quality==0)
            Serial.println("High pollution! Force signal active");
        else if (current_quality==1)
            Serial.println("High pollution!");
        else if (current_quality==2)
            Serial.println("Low pollution!");
        else if (current_quality ==3)
            Serial.println("Fresh air");
    }
/* air quality sensor*/
/* Loudness sensor*/
    loudness_value = analogRead(0);
    Serial.print("Loudness_value is :");
    Serial.println(loudness_value);
/* Loudness sensor*/
    
/* Dust sensor*/  
  duration = pulseIn(dust_pin, LOW);
  lowpulseoccupancy = lowpulseoccupancy+duration;

  if ((millis()-starttime_dust) >= sampletime_ms)//if the sample time = = 30s
  {
    ratio = lowpulseoccupancy/(sampletime_ms*10.0);  // Integer percentage 0=&gt;100
    concentration = 1.1*pow(ratio,3)-3.8*pow(ratio,2)+520*ratio+0.62; // using spec sheet curve
    Serial.print("concentration = ");
    Serial.print(concentration);
    Serial.println(" pcs/0.01cf");
    Serial.println("\n");
    lowpulseoccupancy = 0;
    starttime_dust = millis();
  }
  
/* Dust Sensor*/

/* light sensor*/
    int sensorValue = analogRead(LIGHT_SENSOR); 
    Serial.println("the analog read light sensor data is ");
    Serial.println(sensorValue);
/* light sensor*/

    // check if returns are valid, if they are NaN (not a number) then something went wrong!
    /*if (isnan(t) || isnan(h)) 
    {
        Serial.println("Failed to read from DHT");
    } 
    else 
    {
        Serial.print("Humidity: "); 
        Serial.print(h);
        Serial.print(" %\t");
        Serial.print("Temperature: "); 
        Serial.print(t);
        Serial.println(" *C");
    }
    
  int temp = (int) h;
  int hum = (int) t;  
  int tempdec = t * 100;
  int humdec = h * 100;
  */
  //send_LoRa_data(set_port, String(temp) + "F" + String(hum) + String(temperature));      //send temp / hum as rounded int over lora
  send_LoRa_data(set_port, String(loudness_value) + String(concentration) + String(sensorValue));      //send temp / hum as rounded int over lora
  
  blinky();
  delay(1000);
  read_data_from_LoRa_Mod();
  delay(30000);
}

ISR(TIMER1_OVF_vect)
{
  if(airqualitysensor.counter==61)//set 2 seconds as a detected duty
  {

      airqualitysensor.last_vol=airqualitysensor.first_vol;
      airqualitysensor.first_vol=analogRead(A0);
      airqualitysensor.counter=0;
      airqualitysensor.timer_index=1;
      PORTB=PORTB^0x20;
  }
  else
  {
    airqualitysensor.counter++;
  }
}
void clearBufferArray()                     // function to clear buffer array
{
    for (int i=0; i<count;i++)
    {
        buffer[i]=NULL;
    }                      // clear all index of array with command NULL
}
void InitializeSerials(int baudrate)
{
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

  send_LoRa_Command("radio set crc off");
  delay(1000);
  read_data_from_LoRa_Mod();

  send_LoRa_Command("mac set nwkskey " + set_nwkskey);
  read_data_from_LoRa_Mod();

  send_LoRa_Command("mac set appskey " + set_appskey);
  read_data_from_LoRa_Mod();

  send_LoRa_Command("mac set devaddr " + set_devaddr);
  read_data_from_LoRa_Mod();

  //For this commands some extra delay is needed.
  send_LoRa_Command("mac set adr on");
  delay(1000);
  read_data_from_LoRa_Mod();

  send_LoRa_Command("mac save");
  delay(1000);
  read_data_from_LoRa_Mod();

  send_LoRa_Command("mac join abp");
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
  //Serial.print ("raw_data" +rawdata); //SG
  //{item_id = "a13c1ad9-0d6a-4869-98b3-968846bd8802", raw_payload = rawdata)
  // send_LoRa_Command("mac tx uncnf " + String(tx_port) + String(" ") + rawdata);
  send_LoRa_Command("mac tx uncnf " + String(tx_port) + String(" ") + rawdata);
}


void blinky()
{
  digitalWrite(led_port, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(500);                     // wait for a second
  digitalWrite(led_port, LOW);    // turn the LED off by making the voltage LOW
  delay(500);                     // wait for a second
  digitalWrite(led_port, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(500);                     // wait for a second
  digitalWrite(led_port, LOW);    // turn the LED off by making the voltage LOW
  delay(500);                     // wait for a second

}
