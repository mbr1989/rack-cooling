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
// Anzahl der Interrupts pro Umdrehung (1 oder 2)
const int ANZAHL_INTERRUPTS = 1;
              

// Variablen
unsigned int  rpm_1_out = 0;
unsigned int  rpm_2_out = 0;
unsigned long rpm_1_in  = 0;
unsigned long rpm_2_in  = 0;
unsigned long rpm_1_cnt = 0;
unsigned long rpm_2_cnt = 0;
unsigned long lastmillis_pwm = 0;
 
void setup()
{
  // Initialisieren
  TCCR1B = TCCR1B & 0b11111000 | 0x01; // Setzt Timer1 (Pin 9 und 10) auf 31300Hz
  Serial.begin(115200);
  pinMode(FAN_1_PWM, OUTPUT);
  pinMode(FAN_2_PWM, OUTPUT);
  pinMode(FAN_1_SPD, INPUT);
  pinMode(FAN_2_SPD, INPUT);
  digitalWrite(FAN_1_SPD, HIGH);
  digitalWrite(FAN_2_SPD, HIGH);
  attachInterrupt(digitalPinToInterrupt(FAN_1_SPD), rpm_1, FALLING);
  attachInterrupt(digitalPinToInterrupt(FAN_2_SPD), rpm_2, FALLING);
}
 
void loop()
{
  // control over serial
  if (Serial.available())
  {
    const int tmp_rpm_1_out = Serial.parseInt();
    const int tmp_rpm_2_out = Serial.parseInt();

    if (tmp_rpm_1_out >= 0 && tmp_rpm_2_out >= 0)
    {
      RPM2FANs(tmp_rpm_1_out, tmp_rpm_2_out);
      
      Serial.print("RPM 1 out ");
      Serial.println(rpm_1_out);
      
      Serial.print("RPM 2 out ");
      Serial.println(rpm_2_out);

    }
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
