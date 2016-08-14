#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 4

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

// arrays to hold device address
DeviceAddress insideThermometer;

// Input:
#define FAN_1_SPD 2
#define FAN_2_SPD 3
// Output:
#define FAN_1_PWM 9
#define FAN_2_PWM 10
#define SW_FAN_1_PWM 5
#define SW_FAN_2_PWM 6
#define ledPin 13
// Konstanten
#define UPDATE_ZYKLUS_PWM 1000
#define UPDATE_ZYKLUS_TEMP 1000
// Anzahl der Interrupts pro Umdrehung (1 oder 2)
const int ANZAHL_INTERRUPTS = 1;
// max. Temperatur
#define Tmax 40.0 
// min. Temperatur                          
#define Tmin 30.0
// min. RPM for alarm                         
#define rpm_in_min 800
// FAN RPM deadband                         
#define deadband 70                            

// Variablen
unsigned int  rpm_1_out = 0;
unsigned int  rpm_2_out = 0;
unsigned long rpm_1_in  = 0;
unsigned long rpm_2_in  = 0;
unsigned long rpm_1_cnt = 0;
unsigned long rpm_2_cnt = 0;
unsigned long lastmillis_pwm = 0;
unsigned long lastmillis_temp = 0;
float temperature = 0;
 
void setup()
{
  // Initialisieren
  TCCR1B = TCCR1B & 0b11111000 | 0x01; // Setzt Timer1 (Pin 9 und 10) auf 31300Hz
  Serial.begin(115200);
  pinMode(FAN_1_PWM, OUTPUT);
  pinMode(FAN_2_PWM, OUTPUT);
  pinMode(SW_FAN_1_PWM, OUTPUT);
  pinMode(SW_FAN_2_PWM, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(FAN_1_SPD, INPUT);
  pinMode(FAN_2_SPD, INPUT);
  digitalWrite(FAN_1_SPD, HIGH);
  digitalWrite(FAN_2_SPD, HIGH);
  sensors.begin();
  sensors.getAddress(insideThermometer, 0);
  sensors.setResolution(insideThermometer, 9);
  attachInterrupt(digitalPinToInterrupt(FAN_1_SPD), rpm_1, FALLING);
  attachInterrupt(digitalPinToInterrupt(FAN_2_SPD), rpm_2, FALLING);
}
 
void loop()
{
  if ( (millis() - lastmillis_temp) >= UPDATE_ZYKLUS_TEMP )
  {
    sensors.requestTemperatures(); 
    temperature = sensors.getTempC(insideThermometer);

    if ( temperature == -127.00 )
    {
      RPM2FANs(255, 255);
      SWFANstat(false);
      Serial.println("no temperature sensor found");
    }

    else if ( temperature >= Tmin )
    {
      // Tmin->0% // Tmax->100%
      const unsigned int FanSpeed = map(temperature, Tmin, Tmax, deadband, 255);
      RPM2FANs(FanSpeed, FanSpeed);
    }

    else if ( temperature < Tmin )
    {
      RPM2FANs(0, 0);
    }

    Serial.print("Temperature is ");
    Serial.print(temperature);
    Serial.print(" ");
    Serial.write(0xB0);
    Serial.println("C");
    
    Serial.print("RPM 1 out ");
    Serial.println(rpm_1_out);
    Serial.print("RPM 2 out ");
    Serial.println(rpm_2_out);

    // Zeitpunkt setzen
    lastmillis_temp = millis();
  }

  // Tachosignal berechnen
  if (millis() - lastmillis_pwm >= UPDATE_ZYKLUS_PWM)
  {
    // Interrupt deaktivieren um das rechnen nicht zu unterbrechen.
    detachInterrupt(FAN_1_SPD);
    detachInterrupt(FAN_2_SPD);

    // RPM errechnen und ausgeben:
    rpm_1_in = rpm_1_cnt * (60 / ANZAHL_INTERRUPTS);
    rpm_2_in = rpm_2_cnt * (60 / ANZAHL_INTERRUPTS);

    if ( rpm_1_in <= rpm_in_min )
    {
      SWFANstat(false); 
      Serial.println("FAN 1 speed limit undershot");
    }

    if ( rpm_2_in <= rpm_in_min )
    {
      SWFANstat(false);
      Serial.println("FAN 2 speed limit undershot");
    }

    Serial.print("RPM 1 in ");
    Serial.println(rpm_1_in);
    Serial.print("RPM 2 in ");
    Serial.println(rpm_2_in);
 
    // Counter zuruecksetzen
    rpm_1_cnt = 0;
    rpm_2_cnt = 0;
 
    // Zeitpunkt setzen
    lastmillis_pwm = millis();
 
    // Interrupt wieder aktivieren
    attachInterrupt(digitalPinToInterrupt(FAN_1_SPD), rpm_1, FALLING);
    attachInterrupt(digitalPinToInterrupt(FAN_2_SPD), rpm_2, FALLING);
  }
  // Tachosignal berechnen beendet

  if ( rpm_1_in >= rpm_in_min && rpm_2_in >= rpm_in_min && temperature != -127.00 )
  {
      SWFANstat(true); 
  }
}

// Cisco SG300 Switch - FAN status
void SWFANstat (bool SWFANstat)
{
  // FAN status is operational
  if ( SWFANstat == true )
  {
      digitalWrite(ledPin, HIGH);
      analogWrite(SW_FAN_1_PWM, 255);
      analogWrite(SW_FAN_2_PWM, 255);
  }

  // FAN status is NOT operational
  else if ( SWFANstat == false )
  {
      digitalWrite(ledPin, LOW);
      analogWrite(SW_FAN_1_PWM, 0);
      analogWrite(SW_FAN_2_PWM, 0);
  }
}

// Write RPM to FANs
void RPM2FANs(unsigned int rpm_1_out_new, unsigned int rpm_2_out_new)
{
  if (rpm_1_out_new != rpm_1_out)
  {
      rpm_1_out = rpm_1_out_new;
      analogWrite(FAN_1_PWM, rpm_1_out);
  }

  else
  {
    
  }

  if (rpm_2_out_new != rpm_2_out)
  {
      rpm_2_out = rpm_2_out_new;
      analogWrite(FAN_2_PWM, rpm_2_out);
  }

    else
  {
    
  }
}
 
// Interrupt zaehlt den RPM-1-Counter hoch
void rpm_1()
{
  rpm_1_cnt++;
}

// Interrupt zaehlt den RPM-2-Counter hoch
void rpm_2()
{
  rpm_2_cnt++;
}
