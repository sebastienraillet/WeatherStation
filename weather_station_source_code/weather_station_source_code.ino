#include <Event.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085_U.h>
#include <SPI.h>
#include <SD.h>
#include <Time.h>
 
 
 
// DECLARATION UTILE GENERAL
#define VCC 5
// PORT SERIAL 
#define  BAUD_RATE 9600
// Capteur de lumiére pont diviseur
#define R2  10000 //ohm
//SDCARD
#define CSV_FILE_NAME "test.csv"
// ENTREE DES CAPTEURS
#define chipSelect 8// SDCARD SPARKFUN
#define capteurPhoto A0 // PIN Analogique 0 pour le capteur photo
#define  DRIVE1  6   // Digital pins used to drive square wave for the EFS-10
#define  DRIVE2  7
#define  HSENSE  A3  // analog input hygrometer (EFS-10)
// EFS_10 humidity
#define  LOG_INTERVAL  8192   // Throttles logging to terminal.
#define NR_COLS 8  // Lookup table dimensions
#define NR_ROWS 5  // ;;
#define  DEBUG  false  // Send debug info over Serial.
#define  V_DIVIDER 100000       // Using a 100k Ohm resistor as voltage divider
#define  TEMP_CALIBRATION -1.1f  // Factor to adjust the temperature sensor reading ("software calibration")
#define  STEP_SIZE_TEMP 5.0f     // Temperature step size in lookup table
#define  STEP_SIZE_HUM 10.0f     // Humidity step size in lookup table
#define  PERIOD_US 1000  // for 1kHz, or use 10000 for 100Hz
#define  HALF_PERIOD_US (PERIOD_US/2)
// TIME
#define TIME_DEFAULT 1399312618
#define TIME_MSG_LEN  11   // time sync to PC is HEADER followed by unix time_t as ten ascii digits
#define TIME_HEADER  'T'   // Header tag for Serial time sync message
#define TIME_REQUEST  7    // ASCII bell character requests a time sync message


 
 
 
 
/*
   Hygrometer characteristics, impedance at different temps.
   From data-sheet at
   http://www.produktinfo.conrad.com/datenblaetter/150000-174999/156545-da-01-en-Feuchtesensor_EFS10.pdf
*/
float lookupTable[NR_ROWS][NR_COLS] = {
  /* T/H      20%            30%            40%           50%           60%             70%             80%            90%           */
  /* 15C */  {6364*1000.0f,  1803*1000.0f,  543*1000.0f,  166*1000.0f,  55.64*1000.0f,  20.94*1000.0f,  8.07*1000.0f,  3.26*1000.0f},
  /* 20C */  {4500*1000.0f,  1300*1000.0f,  398*1000.0f,  125*1000.0f,  43.0*1000.0f,   17.0*1000.0f,   6.85*1000.0f,  2.85*1000.0f},
  /* 25C */  {2890*1000.0f,   900*1000.0f,  270*1000.0f,  81*1000.0f,   33.0*1000.0f,   13.0*1000.0f,   5.30*1000.0f,  2.20*1000.0f},
  /* 30C */  {2100*1000.0f,   670*1000.0f,  210*1000.0f,  66*1000.0f,   25.50*1000.0f,  10.20*1000.0f,  4.28*1000.0f,  1.85*1000.0f},
  /* 35C */  {1652*1000.0f,   530*1000.0f,  168*1000.0f,  54*1000.0f,   21.54*1000.0f,   8.69*1000.0f,  3.71*1000.0f,  1.62*1000.0f}
};
 
// State variables
//
unsigned long prev_time;  // Used to generate square wave to drive humidity sensor
byte phase = 0;           // Used to generate square wave to drive humidity sensor
float prevVoltage = 0.0f; // Used for keping a moving average of humidity sensor readings
int printMe = 0;          // just a counter to trottle logging of values
 
//GLOBAL
//SD
char unChar;
File monFichier;
// Photo
float R1, RPHOTO,tensionPhoto;
int valeurPhoto;     // Contient la lecture analogique sur le pont diviseur Photo-résistance + R 10 KOhms
 
Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085);
 
// Valeur des capteurs
float humidite,altitude,g_pression,temperature;
long double rlum;
String date,luminosite;
 
 
String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete
 
 
 
 
void setup() 
{
	Serial.begin(BAUD_RATE); 
	pinMode (DRIVE1, OUTPUT);
	pinMode (DRIVE2, OUTPUT);
	prev_time = micros ();
	inputString.reserve(200); 
  	pinMode(chipSelect, OUTPUT);

  if (!SD.begin(chipSelect)) 
  {
      Serial.println("SD initialization failed!");
      return;
  }

  /* Initialise the BMP 085 sensor */
  if (!bmp.begin())
  {
    Serial.print("BMP085 initialization failed");
    return;
  }
       
  setSyncProvider(requestSync);  //set function to call when sync required
  setTime(TIME_DEFAULT); 
} 
 
/*
  SerialEvent occurs whenever a new data comes in the
 hardware Serial RX.  This routine is run between each
 time loop() runs, so using delay inside loop can delay
 response.  Multiple bytes of data may be available.
 */
 
void serialEvent()
{
  while (Serial.available()) 
  {
    char inChar = (char)Serial.read(); // Get the new byte
    inputString += inChar; // add it to the inputString
  }       
}

void loop() {
 
  // Generate 1kHz square wave to drive sensor. Busy wait, so blocks here.
  squareWaveSensorDrive();
 
  float voltage = readHumiditySensor();  
  float temp = temperature- 0.01;
  //Serial.println(temperature);
  humidite = voltToHumidity(voltage, temp);
 
  // la date
 
 
  if(printMe++%LOG_INTERVAL == 0) {
    bmp085read();
    // Lecture de la lumiere
                readLight();
                convertirResistanceLum();
       
 
        // la date
                if(inputString.length() )
                {
                        processSyncMessage();
inputString  = "";
                        }
                if(timeStatus()!= timeNotSet)
                {
                        digitalWrite(13,timeStatus() == timeSet); // on if synced, off if needs refresh
                        digitalClockDisplay();
                }
               
 
 
                          Serial.print(date);
                                Serial.print(';');
                                Serial.print(temperature);
                                Serial.print(';');
                                Serial.print(g_pression);
                                Serial.print(';');
                                Serial.print(luminosite);
                                Serial.print(';');
                                Serial.print(humidite);
                                Serial.print('\n');
                                inputString = "";
                // Ecriture du log sur la SDCARD
                writeSDCARD();
   
   
  }
}
 
 
/*
  Work out impedance and finally relative humidity, based on:
  - Current humidity sensor voltage reading
  - Current temp
  - Lookup table from data sheet stored in lookupTable[][]
  - Interpolation between neighbouring impedance values
  - Interpolation between neighbouring humidity values
 
  The circuit will read about 2.5V at (20C, 50% relative humidity)
*/
float voltToHumidity(float humidityVolt, float temp) {
 
  // The final result will end up in this variable.
  float relativeHumidity = 0.0f;
 
  // First: Calculate the impedance.
  // impedance = U_measured/(current through R2). See circuit
  float imp = humidityVolt/((5.0 - humidityVolt)/V_DIVIDER);
 
  // Calculate nearest temp rows and humidity columns (indices) that contain
  // the measured point (temp, imp) in the lookup table.
  //
 
  // The temperature rows
  byte tIndexL = (byte) (temp/5 - 3); // -3 is there to offset the index since the complete lookup table is not ported from the data sheet
  byte tIndexH = tIndexL + 1;
 
  // At this point we have an idea about which temperature rows to look in (tIndexL and tIndexH in lookupTable[][]).
  // Next: Find the two closest impedance cells based on r and  pick out the corresponding four
  // humidity values from the lookup table and then interpolate to approximate the humidity.
  //
  byte hIndexL = NR_COLS-1;
  byte hIndexH = 0;
 
  // Match calculated impedance to values in the lookup table
  while (lookupTable[tIndexL][hIndexL] < imp && hIndexL-- > 0);  
  while (lookupTable[tIndexH][hIndexH] > imp && hIndexH++ < NR_COLS);  
 
  // Calculate the humidity values at the corners of our "bounding box" in the lookup table.
  // This is the base for the 2d interpolation.
  float tLow = (float)(tIndexL+3) * STEP_SIZE_TEMP;
  float tHigh = (float)(tIndexH+3) * STEP_SIZE_TEMP;
  float rH_impLow_tL = 20.0f + hIndexL * STEP_SIZE_HUM; // Table starts at rH 20%
  float rH_impLow_tH = 20.0f + hIndexH * STEP_SIZE_HUM;  
  float rH_impHigh_tL = 20.0f + hIndexL * STEP_SIZE_HUM;
  float rH_impHigh_tH = 20.0f + hIndexH * STEP_SIZE_HUM;
 
  // Do interpolation along humidity and temperature axis to estimate relative humidity at (temp, imp) given the values in the table
  float rH0, rH1, tF, hFL, hFH;
  if(hIndexL > 0 && hIndexH < NR_COLS && tIndexH < NR_ROWS) 
  {
    tF = (temp - tLow)/STEP_SIZE_TEMP; // Temperature interpolation factor
   
    // Humidity sensor impedance steps are not equidistant for different temperatures, so we have to calculate
    // two interpolation factors for high and low value of relative humidity.
    hFL = (imp - lookupTable[tIndexL][hIndexL])/(lookupTable[tIndexL][hIndexL-1] - lookupTable[tIndexL][hIndexL]);
    hFH = (imp - lookupTable[tIndexL][hIndexH])/(lookupTable[tIndexH][hIndexH-1] - lookupTable[tIndexH][hIndexH]);
   
    rH0 = rH_impLow_tL + hFL*STEP_SIZE_HUM;
    rH1 = rH_impLow_tH + hFH*STEP_SIZE_HUM;
   
    relativeHumidity = rH0 + tF*((rH_impHigh_tL - rH_impLow_tL)/(rH1 - rH0));
  }
  else // Edge case.
    relativeHumidity = 0.0f; // TODO: linear iterpolation here
 
  if(DEBUG && printMe%LOG_INTERVAL == 0) {
    Serial.print("DEBUG: humidityVolt: ");
    Serial.print(humidityVolt);
    Serial.print(" temp: ");
    Serial.print(temp);
    Serial.print(" impedance ");
    Serial.print(imp);
 
    Serial.print(" tIndexL: ");
    Serial.print(tIndexL);
    Serial.print(" tIndexH: ");
    Serial.print(tIndexH);
    Serial.print(" hIndexL: ");
    Serial.print(hIndexL);
    Serial.print(" hIndexH: ");
    Serial.print(hIndexH);
 
    Serial.print(" rH_impLow_tL: ");
    Serial.print(rH_impLow_tL);
    Serial.print(" rH_impLow_tH: ");
    Serial.print(rH_impLow_tH);
    Serial.print(" rH_impHigh_tL: ");
    Serial.print(rH_impHigh_tL);
    Serial.print(" rH_impHigh_tH: ");
    Serial.print(rH_impHigh_tH);
   
    Serial.print(" tF: ");
    Serial.print(tF);
    Serial.print(" hFL: ");
    Serial.print(hFL);
    Serial.print(" hFH: ");
    Serial.print(hFH);
   
    Serial.print(" rH0: ");
    Serial.print( rH0);
    Serial.print(" rH1: ");
    Serial.println( rH1);
  }
  return relativeHumidity;
}
 
 
/*
  Read sensor, convert to volts and apply a basic moving average to minimize jitter.
  Returns the reading in Volts.
*/
float readHumiditySensor() {
  int val = analogRead (HSENSE);
  val = analogRead (HSENSE); // second read allows much longer for high impedance reading to settle
  if (phase == 0)
    val = 1023 - val;  // reverse sense on alternate half cycles
  float v = 5.0 * val / 1024; // Convert to volts
  float voltage = (v + prevVoltage) / 2; // Moving average
  prevVoltage = voltage;
  phase = 1 - phase;
 
  return voltage;
}
 
 
 
 
 
/*
   Generates a square wave to drive the humidity sensor..
   Blocks for half the period.
*/
void squareWaveSensorDrive() {
  while (micros () - prev_time < HALF_PERIOD_US) {}  //wait for next 0.5ms time segment
  prev_time += HALF_PERIOD_US;   // set up for the next loop
  digitalWrite (DRIVE1, phase == 0);   // antiphase drive
  digitalWrite (DRIVE2, phase != 0);
}
 
 
/*
  Output read values at regular intervals.
  Prints to Serial.
*/
void outputResult(float realtiveHumidity, float temp) {
  Serial.print(realtiveHumidity);
  Serial.print(",");
  Serial.println(temp);
}
 
/* Cette fonction permet de lire le capteur photo
*
*/
int readLight()
{
        valeurPhoto = analogRead(capteurPhoto);
        tensionPhoto = valeurPhoto * 4.88;
        // On utilise un pont diviseur de tension entre la photoresistance, la resistance, 5V et le GND.
        tensionPhoto = tensionPhoto / 1000;
        // Calcule de photoresistance
        RPHOTO = (R2 * (VCC - tensionPhoto)) / tensionPhoto ;
        return RPHOTO;
        }
 
 
void convertirResistanceLum()
{
        if (rlum  > 600000)
        {
                luminosite = "Noir" ;
                }
        else if (rlum > 23000)
        {
                luminosite = "Piece tres sombre";
                }
        else if (rlum > 6000)
        {
                luminosite = " Piece sombre";
                }
        else if (rlum > 2000) // Photo resistance sup&#8730;©rieur a 6mOhm
        {
                luminosite = "Piece clair" ;
                }
       else if(rlum<0)
       {
                       luminosite = "Nuit" ;

       }
        else
        {
                luminosite = "Jour" ;
                }
}
 
void digitalClockDisplay(){
        // digital clock display of the time
       
        date = String(day());
        date +='/';
        date +=month();
        date +='/';
        date +=year();
       
        date +=' ';
        date += String(hour()) ;
        printDigits(minute());
        }
 
void printDigits(int digits){
        // utility function for digital clock display: prints preceding colon and leading 0
        date += ':';
        if(digits < 10)
        date += '0';
        date += digits;
        }
 
void processSyncMessage() 
{
// if time sync available from Serial port, update time and return true
int g = 0;
while(inputString.length()-g)
{  // time message consists of a header and ten ascii digits
        char c = inputString[g];
        g = 1+g;
             
                if( c == TIME_HEADER ) {
                time_t pctime = 0;
                for(int i=1; i < TIME_MSG_LEN; i++){
                     
                        c = inputString[i];
                        Serial.print(c);  
                        if( c >= '0' && c <= '9'){
                                pctime = (10 * pctime) + (c - '0') ; // convert digits to a number
                                }
                        }
                setTime(pctime);   // Sync Arduino clock to the time received on the Serial port
                }
                else if(c == 'C')
              {
                readSDCARD();
                inputString = "";
                break;
              }
               
                else if(c == 'A')
                {
                        Serial.print(date);
                        Serial.print(';');
                        Serial.print(temperature);
                        Serial.print(';');
                        Serial.print(g_pression);
                        Serial.print(';');
                        Serial.print(luminosite);
                        Serial.print(';');
                        Serial.print(humidite);
                        Serial.print('\n');
                        inputString = "";
                       
                        break;
  }
}


inputString = "";
}
 
time_t requestSync()
{
  //Serial.print(TIME_REQUEST);
  return 0; // the time will be sent later in response to Serial mesg
}
 
 
void readSDCARD()
{
  File l_fichier = SD.open(CSV_FILE_NAME, FILE_READ);

  if(l_fichier)
  {      
    // Lire tous le fichier
    while (l_fichier.available())
    {
      Serial.write(l_fichier.read());
    }
    l_fichier.close();
  }
}
 
void writeSDCARD()
{
  File l_fichier = SD.open(CSV_FILE_NAME, FILE_WRITE);

  if(l_fichier)
  {
    l_fichier.print(date);
    l_fichier.print(';');
    l_fichier.print(temperature);
    l_fichier.print(';');
    l_fichier.print(g_pression);
    l_fichier.print(';');
    l_fichier.print(luminosite);
    l_fichier.print(';');
    l_fichier.print(humidite);
    l_fichier.print('\r\n');
    l_fichier.close();
  }
} 
 
void bmp085read()
{
  /* Get a new sensor event */
  sensors_event_t l_sensor_event;
  bmp.getEvent(&l_sensor_event);

  /* Display the results (barometric pressure is measure in hPa) */
  if (l_sensor_event.pressure)
  {
    g_pression = l_sensor_event.pressure;
   
    /* First we get the current temperature from the BMP085 */
    bmp.getTemperature(&temperature);
   
    /* Then convert the atmospheric pressure, SLP and temp to altitude    */
    /* Update this next line with the current SLP for better results      */
    float seaLevelPressure = SENSORS_PRESSURE_SEALEVELHPA;
   
    altitude = bmp.pressureToAltitude(seaLevelPressure, l_sensor_event.pressure, temperature);  
  }
  else
  {
    Serial.println("BMP085 sensor error");
  }
}
