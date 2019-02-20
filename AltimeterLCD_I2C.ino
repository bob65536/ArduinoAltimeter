/* Altimeter on I2C
// Credits: 
I2C LCD (lib HD44780): Bill Perry bperrybap@opensource.billsworld.billandterrie.com
BMP085: Written by Limor Fried/Ladyada for Adafruit Industries. BSD license.
BMP280: Written by orgua (https://github.com/orgua/iLib). GPL 2.0.
DS3231: Kees Bakker (Sodaq_DS3231 lib) MIT License
SD: Created by Tom Igoe (Arduino examples). Public Domain.
----------------------------------------------------------------------------  
Connections:
 > I2C LCD: 
   SDA - A4
   SCL - A5
 > DS3231:
   SDA - A4
   SCL - A5
 > BMP180 (or BMP280 - change the lib used)
   SDA - A4
   SCL - A5
   Power: 3.3V
 > SD
   MOSI - pin 11
   MISO - pin 12
   CLK - pin 13
   CS - pin 6 
   
Display example: 
133723 23° #1231
101325Pa 223.15m

1 loop = 100+200 = 300 ms
1 line = 41 bytes. 
  SD of 4 GB = 1yr or recording (should be enough)
*/

#include <Wire.h>
#include <hd44780.h>                       // main hd44780 header
#include <hd44780ioClass/hd44780_I2Cexp.h> // i2c expander i/o class header
#include "Sodaq_DS3231.h"
// #include <Adafruit_BMP085.h> // If you have a BMP085/BMP180, uncomment this
#include <SPI.h>
#include <SD.h>
#include "i2c.h" // BMP280. If you have a BMP280, uncomment this
#include "i2c_BMP280.h"  // BMP280. If you have a BMP280, uncomment this

hd44780_I2Cexp lcd; // declare lcd object: auto locate & config exapander chip
BMP280 bmp280;  // BMP280. If you have a BMP280, uncomment this

byte deltaChar[8] = {
  0b00000,
  0b00000,
  0b00100,
  0b01110,
  0b11011,
  0b10001,
  0b11111,
  0b00000
};

//Adafruit_BMP085 bmp; // Uncomment this if you have a BMP085/BMP180

// LCD geometry
const int LCD_COLS = 16;
const int LCD_ROWS = 2;
uint32_t old_ts;
const int chipSelect = 6;

uint32_t loopVar = 0; // How much points
static float a0, p0;

void setup()
{
  int status;
  status = lcd.begin(LCD_COLS, LCD_ROWS);
  Serial.begin(9600);
  Serial.println("Starting...");
  if(status) // non zero status means it was unsuccesful
  {
    Serial.println("No LCD detected!!!");
  }
  lcd.createChar(0, deltaChar); // Custom char init
  /*if (!bmp.begin()) {
    Serial.println("Could not find a valid barometer sensor, check wiring!");
    //while (1) {}
    }
  */
  Serial.print("Probe BMP280: ");
  if (bmp280.initialize()) Serial.println("Sensor found");
  else
  {
    Serial.println("Sensor missing");
    while (1) {}
  }
  // onetime-measure:
  bmp280.setEnabled(0);
  bmp280.triggerMeasurement();  
  
  Serial.print("Initializing SD card...");
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    // while (1);
  }
  Serial.println("Card initialized (or not - check above)");

  DateTime now = rtc.now(); //get the current date-time
  String title = String("Val#;Date;Temp;Pres;Alt;AltISA");
  static String nameFile = String(twoDigits(now.month())+twoDigits(now.date())+twoDigits(now.hour())+twoDigits(now.minute())) + ".CSV";
  File dataFile = SD.open(nameFile, FILE_WRITE);
  if (dataFile) {
    dataFile.println(title);
    dataFile.close();
  }
  
  if(LCD_ROWS < 2)
    delay(3000);
    
  Wire.begin();
  rtc.begin();

  // Get base pressure (other measures are necessary...)
  bmp280.awaitMeasurement();
  float t0;
  bmp280.getTemperature(t0);
  bmp280.getPressure(p0); // Base pressure
  bmp280.getAltitude(a0); // Base alt
  Serial.println("P0 = "+String(p0));
  Serial.println("Alt0 = "+String(a0));
  bmp280.triggerMeasurement();  

  // pinMode(LED_BUILTIN, OUTPUT); // To see if measures are done and recorded (conflict with SD)

}

void loop()
{
  bmp280.awaitMeasurement();
  /*
  static long p0 = bmp.readPressure(); 
  float temp = bmp.readTemperature();
  long pres = bmp.readPressure();
  float alt = bmp.readAltitude(p0);
  */
  float temp;
  bmp280.getTemperature(temp);
  float pres;
  bmp280.getPressure(pres);
  static float alt, metersOld;
  bmp280.getAltitude(alt); 
  metersOld = (metersOld * 10 + alt)/11; // Sort of smoothing
  bmp280.triggerMeasurement();
  float altISA = altitude(pres); // Altitude as given in the ISA model
                                 // (Sea level at 1013.25 hPa)
  
  DateTime now = rtc.now(); //get the current date-time
  String dateNow = String(now.year()+twoDigits(now.month())+twoDigits(now.date())+twoDigits(now.hour())+twoDigits(now.minute())+twoDigits(now.second()));
  static String nameFile = String(twoDigits(now.month())+twoDigits(now.date())+twoDigits(now.hour())+twoDigits(now.minute())) + ".CSV";
  
  // set the cursor position to column 0, row 1
  lcd.setCursor(0, 0);
  lcd.print(twoDigits(now.hour()));
  lcd.setCursor(2, 0);
  lcd.print(twoDigits(now.minute()));
  lcd.setCursor(4, 0);
  lcd.print(twoDigits(now.second()));
  lcd.setCursor(8, 0);
  lcd.print("   ");
  lcd.setCursor(9, 0);
  lcd.print((char)223); // Display the degree symbol
  lcd.setCursor(7, 0);
  lcd.print(String(temp,0)); // Don't want decimals
  lcd.setCursor(11,0);
  lcd.print("     ");
  // Where to begin the number?
  int startCol;
  if (loopVar>9999)
    startCol=10;
  else if (loopVar>999)
    startCol=11;
  else if (loopVar>99)
    startCol=12;
  else if (loopVar>9)
    startCol=13;
  else
    startCol=14;
  lcd.setCursor(startCol, 0);
  lcd.print('#');
  lcd.print(loopVar);
  lcd.setCursor(5, 1);
  lcd.print(" Pa ");
  lcd.setCursor(0, 1);
  lcd.print(String(pres,0));
  lcd.setCursor(13, 1);
  if(loopVar % 10 < 5) {
    lcd.print("  m"); 
    lcd.setCursor(9, 1);
    lcd.print(String(altISA,0));    
  }
  else {
    lcd.write(" m");
    lcd.write((uint8_t)0); // Show delta sign
    lcd.setCursor(9, 1);
    lcd.print(String(alt-a0,1)); // Show the diff since last init
  }
  
  String dataString = String(loopVar)+";"+String(dateNow)+";"+String(temp)+";"+String(long(pres))+";"+String(alt-a0)+";"+String(altISA)+";";
  
  File dataFile = SD.open(nameFile, FILE_WRITE);
  Serial.print("Writing on " + nameFile + ": ");
  Serial.println(dataString); // DEBUG
  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
  }
  else {
    Serial.println("Can not write :(");
  }
  loopVar++;

  /*
  if (loopVar % 4 < 2)
    digitalWrite(LED_BUILTIN, HIGH); // Turn LED pin 13 ON
  else
    digitalWrite(LED_BUILTIN, LOW);   // turn the LED on (HIGH is the voltage level)
  */
  
  delay(200);
}

String twoDigits(int number) {
  if (number < 10)
    return "0" + String(number);
  else
    return String(number);
}

float altitude(float pressure) {
  return 44330.0*(1-pow(float(pressure)/101325.0,1.0/5.255));
}
