/*
  AirQuality Demo V1.0.
  connect to A0 to start testing. it will needs about 20s to start 
* By: http://www.seeedstudio.com
*/
#include"AirQuality.h"
#include"Arduino.h"
AirQuality airqualitysensor;
int current_quality =-1;
void setup()
{
    Serial.begin(9600);
    airqualitysensor.init(14);
}
void loop()
{
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


/*To adjust the thresholds and indicating messages
 int AirQuality::slope(void)
{
    while(timer_index)
    {
        if(first_vol-last_vol>400||first_vol>700)
        {
            Serial.println("High pollution! Force signal active.");
            timer_index=0;
            avg_voltage();
            return 0;

        }
        else if((first_vol-last_vol>400&&first_vol<700)||first_vol-vol_standard>150)
        {
            Serial.print("sensor_value:");
            Serial.print(first_vol);
            Serial.println("\t High pollution!");
            timer_index=0;
            avg_voltage();
            return 1;

        }
        else if((first_vol-last_vol>200&&first_vol<700)||first_vol-vol_standard>50)
        {
            //Serial.println(first_vol-last_vol);
            Serial.print("sensor_value:");
            Serial.print(first_vol);
            Serial.println("\t Low pollution!");
            timer_index=0;
            avg_voltage();
            return 2;
        }
        else
        {
            avg_voltage();
            Serial.print("sensor_value:");
            Serial.print(first_vol);
            Serial.println("\t Air fresh");
            timer_index=0;
            return 3;
        }
    }
    return -1;
}

*/
 */
