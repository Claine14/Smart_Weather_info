/*
 * 
 * V0 - LDR Sensor Analog
 * V1 - Rain Sensor Digital (Aktif LOW Rain Detect)
 * V2 - Weather Color Button
 * V3 - Weather Info
 * V4 - Time
 * V5 - Date
 * V6 - Weather Emoji
 * V7 - Pressure BMP180 (bp)
 * V8 - Altitude BMP180 (ba)
 * V9 - Temperature BMP180 (bt)
 */


//FIRMWARE CONFIGURATION to Blynk. For connect to blynk app
#define BLYNK_TEMPLATE_ID "TMPL6TUIVFmOE"
#define BLYNK_TEMPLATE_NAME "Smart Weather Information"
#define BLYNK_AUTH_TOKEN "l7XFtoHgLFv3Z5nyRnsWw6zwAHdhUZKW"

#define BLYNK_PRINT Serial

//set wifi network
char ssid[] = "METI 2";
char pass[] = "03082015";

#include <ESP8266WiFi.h> //ibrary for the ESP8266 Wi-Fi module. To establish WIFI connection between device and network.
#include <BlynkSimpleEsp8266.h> //Blynk library for the ESP8266 Wi-Fi module, which provides a simple way to interface with the Blynk app and cloud service.
#include <TimeLib.h> //Time library, which provides functions for working with time and date values.
#include <WidgetRTC.h> //WidgetRTC library, which provides a widget for displaying and setting the real-time clock (RTC) in the Blynk app.
#include <Wire.h> //library for I2C (Inter-Integrated Circuit), which provides functions for communication over the I2C bus.
#include <Adafruit_BMP085.h> //Adafruit BMP085 library, which provides functions for reading data from the BMP085/BMP180 pressure and temperature sensor.
#include <LiquidCrystal_I2C.h> //LiquidCrystal_I2C library, which provides a simple way to interface with LCD displays that use the I2C bus.

// #include <Adafruit_BusIO_Register.h>
#include <Adafruit_I2CDevice.h>
// #include <Adafruit_I2CRegister.h>
// #include <Adafruit_SPIDevice.h>
// #include <Blynk.h>

//define color for blynk app
#define BLYNK_GREEN     "#23C48E"
#define BLYNK_BLUE      "#04C0F8"
#define BLYNK_YELLOW    "#ED9D00"
#define BLYNK_RED       "#D3435C"
#define BLYNK_DARK_BLUE "#5F7CD8"

#define I2C_SCL D1   //D1 SCL PIN OUT NodeMCU Lolin
#define I2C_SDA D2   //D2 SDA PIN OUT NodeMCU Lolin

#define rainSensor 1 //TX PIN (GPIO1)
#define ldrSensor A0 //Connect to PIN A0. Represent an analog pin of the device for LDR sensor
 
BlynkTimer timer; //BlynkTimer object called "timer". (Blynk library). To execute a function periodically with a specified interval.

WidgetRTC rtc; //WidgetRTC object called "rtc". (Blynk library). this use for access the current time and date, as well as set the RTC (real-time clock) on the Blynk server.

LiquidCrystal_I2C lcd(0x27, 16, 2); //LiquidCrystal_I2C object called "lcd" to control LCD. 
                      //"0x27": the I2C address of the LCD display
                      //"16": the number of columns in the LCD display.
                      //"2": the number of rows in the LCD display.
                      //use LiquidCrystal_I2C.h library

Adafruit_BMP085 bmp; //create object called bmp. (Adafruit_BMP085.h). To read the atmospheric pressure, altitude, and temperature data from the sensor

String weatherInfo; //declares a String variable called "weatherInfo". A String is a text variable that can store a sequence of characters. We Use this for detect weather condition
String days[]   = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"}; //define an array of String objects called "days", containing the names of the days of the week
String months[] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "Desember"}; //define an array of String objects called "months", containing the names of the months 

const int speakerPin = 14; //buzzer connected to D5 (GPIO14)


void setup() {

  lcd.begin(); //initializes the LCD display by setting the number of columns and rows     
  lcd.backlight(); // turns on the backlight of the LCD display

  Serial.begin(9600); // initializes the serial communication
  pinMode(rainSensor,INPUT); //read the value of a digital signal from a sensor (RAIN SENSOR)
  
  Wire.begin(I2C_SDA, I2C_SCL); // initializes the I2C communication protocol 
  delay(10); //delay of 10 milliseconds in the program execution
  
  //check connection of bmp sensor
  if (!bmp.begin()) {
  Serial.println("Could not find a valid BMP085/BMP180 sensor, check wiring!"); //print eror message 
  while (1) {} // infinite loop, which ensures that the program execution halts at this point and does not proceed any further.
  } 

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass); //set blynk connection 
  setSyncInterval(1 * 60); // sets the sync interval for real-time clock (RTC) synchronization to 1 minute.  RTC widget in the Blynk app will be updated with the current time every 1 minute.
  timer.setInterval(1000L, wheatherStatus); // sets an interval of 1000 milliseconds (1 second). Update Weather every 1 second (we use blynkapp)

  pinMode(speakerPin,OUTPUT); //function call that sets the speakerPin. Output for buzzer sound
}

BLYNK_CONNECTED() {
  // Synchronize time on connection
  rtc.begin();
}

void buzzer(){
  //buzzer on
  digitalWrite(speakerPin,HIGH);
  delay(700);
  //buzzer off
  digitalWrite(speakerPin,LOW);
  delay(700);
}

void wheatherStatus() //function to check weather
{
  float bp =  bmp.readPressure(); //to store the atmospheric pressure in Pascals (Pa)
  float ba =  bmp.readAltitude(); //to stores the altitude above sea level in meters (m), calculated from the atmospheric pressure
  float bt =  bmp.readTemperature(); //to stores the temperature in degrees Celsius (°C)
  float dst = bmp.readSealevelPressure(); //to stores the atmospheric pressure at sea level in Pascals (Pa), calculated based on the altitude above sea level and the atmospheric pressure.
  
  String currentTime;//String variable that will store the current time in the format "HH:MM AM/PM".
  int HR = hour(); //declare a variable to specify the hour
  char* meridian; // Meridian represent either "AM" or "PM"

  if(HR > 12) //if hour greater than or equal 12
  {
    meridian = " PM"; //set meridian to PM
    HR = HR - 12; 
    currentTime = String(HR) + ":" + minute() + ":" + second() + meridian; //constructs a string representation of the current time in the format "HH:MM:SS PM" using the String() function
  }
  else //If hour less than 12 then
  {
    meridian = " AM"; //set meridian to AM
    currentTime = String(hour()) + ":" + minute() + ":" + second() + meridian; //constructs a string representation of the current time in the format "HH:MM:SS PM" using the String() function
  }
   
  String currentWeek = String(months[month()-1]); //set to current month
  String currentDate = String(days[weekday()-1] + ", " + day()) + " " + currentWeek + " " + year(); //set to the day name, the day date, the month name and the year. 
  
  int ldrSensorAnalog = analogRead(ldrSensor); //read the ldr sensor (The analogRead() function reads the voltage on the specified pin and converts it into a value between 0 and 1023)
  int rainSensorDigital = digitalRead(rainSensor); //read the rain sensor (The digitalRead() function reads the voltage on the specified pin and returns either HIGH (1) or LOW (0))
  int colorStation = map(ldrSensorAnalog,0,562,0,255);

  //Weather Info 
  // Morning, 00:00 AM sd 09:59 AM
  if((hour() >= 0)&&(hour() < 10))
  {
    if(rainSensorDigital==true) // Not Raining
    {
      
      if(ldrSensorAnalog <=349) //if the brightness is less than or equal 349
      {
        weatherInfo = "Sunny Morning"; //set the weather info to sunny morning
        Blynk.setProperty(V2, "color", "BLYNK_YELLOW"); //set a button wheather info to yellow
        Blynk.virtualWrite(V3, weatherInfo); //send and display weather info in v3 data
        Blynk.virtualWrite(V6, "\xF0\x9F\x8C\x9E "); //Unicode (UTF-8) Character for 🌞 
      }
      if((ldrSensorAnalog >=350)&&(ldrSensorAnalog <=449)) //if the brightness is between 350 and 449
      {
        buzzer(); //set buzzer active
        weatherInfo = "Cloudy Morning"; //set the weather info to cloudy morning
        Blynk.setProperty(V2, "color", "#A4A4A4"); //set a button weather info to grey
        Blynk.virtualWrite(V3, weatherInfo); //send and display weather info in V3 data
        Blynk.virtualWrite(V6, "\xE2\x9B\x85 "); //Unicode (UTF-8) Character for ⛅
      }
      if(ldrSensorAnalog >=450) //if the brightness is greater than equal 450
      {
        buzzer(); //set buzzer active
        weatherInfo = "Dark Morning"; //set the weather info to dark morning
        Blynk.setProperty(V2, "color", "#000000"); //set a button weather info to black
        Blynk.virtualWrite(V3, weatherInfo); //send and display weather info in V3 data
        Blynk.virtualWrite(V6, "\xE2\x98\x81 "); //Unicode (UTF-8) Character for ☁
      }
    }
    else //Raining
    {
      buzzer(); //set buzzer active
      weatherInfo = "Rainy Morning"; //set the weather info to rainy morning
      Blynk.virtualWrite(V3, weatherInfo); //send and display weather info in V3 data
      Blynk.virtualWrite(V6, "\xE2\x98\x94 "); //Unicode (UTF-8) Character for☔
      if(ldrSensorAnalog <=349) //if the brightness is less than or equal 349
      {
        Blynk.setProperty(V2, "color", BLYNK_YELLOW); //set a button wheather info to yellow
      }
      if((ldrSensorAnalog >=350)&&(ldrSensorAnalog <=449)) //if the brightness between 350 and 449
      {
        Blynk.setProperty(V2, "color", "#A4A4A4"); //set a button wheather info to grey
      }
      if(ldrSensorAnalog >=450) //if the brightness is greater than equal 450
      {
        Blynk.setProperty(V2, "color", "#000000"); //set a button wheather info to black
      }
    }
  }

  // Afternoon, 10:00 AM sd 05:59 PM
  if((hour() >= 9)&&(hour() < 18))
  {
    if(rainSensorDigital==true) // Not Raining
    {
      if(ldrSensorAnalog <=249) //if the brightness is greater than equal 249
      {
        weatherInfo = "Sunny Afternoon"; //set the weather info to sunny afternoon
        Blynk.setProperty(V2, "color", BLYNK_YELLOW); //set a button wheather info to yellow
        Blynk.virtualWrite(V3, weatherInfo); //display weather info in V3 data
        Blynk.virtualWrite(V6, "\xE2\x98\x80 "); //Unicode (UTF-8) Character for ☀
      }
      if((ldrSensorAnalog >=250)&&(ldrSensorAnalog <=449)) //if the brightness between 250 and 449
      {
        buzzer(); //set buzzer active
        weatherInfo = "Cloudy Afternoon"; //set the weather info to cloudy afternoon
        Blynk.setProperty(V2, "color", "#A4A4A4"); //set a button wheather info to grey
        Blynk.virtualWrite(V3, weatherInfo); //send and display weather info in V3 data
        Blynk.virtualWrite(V6, "\xE2\x9B\x85 "); //Unicode (UTF-8) Character for ⛅
      }
      if(ldrSensorAnalog >=450) //if the brightness greater than 450
      {
        buzzer(); //set buzzer active
        weatherInfo = "Dark Afternoon"; //set the weather info to dark afternoon
        Blynk.setProperty(V2, "color", "#000000"); //set a button wheather info to black
        Blynk.virtualWrite(V3, weatherInfo); //send and display weather info in V3 data
        Blynk.virtualWrite(V6, "\xE2\x98\x81 "); //Unicode (UTF-8) Character for ☁
      }
    }
    else //Raining
    {
      buzzer();  //set buzzer active
      weatherInfo = "Rainy Afternoon"; //set the weather info to Rainy Afternoon
      Blynk.virtualWrite(V3, weatherInfo); //send and display weather info in V3 data
      Blynk.virtualWrite(V6, "\xE2\x98\x94 "); //Unicode (UTF-8) Character for ☔
      if(ldrSensorAnalog <=249) //if the brightness is greater than equal 249
      {
        Blynk.setProperty(V2, "color", BLYNK_YELLOW); //set a button wheather info to yellow
      }
      if((ldrSensorAnalog >=250)&&(ldrSensorAnalog <=449)) ////if the brightness between 250 and 449
      {
        Blynk.setProperty(V2, "color", "#A4A4A4"); //set a button wheather info to grey
      }
      if(ldrSensorAnalog >=450) //if the brightness greater than 450
      {
        Blynk.setProperty(V2, "color", "#000000"); //set a button wheather info to black
      }
    }
  }

  // Evening, 06:00 PM sd 11:59 PM
  if((hour() >= 18)&&(hour() < 24))
  {
    if(rainSensorDigital==true) // Not Raining 
    {
      weatherInfo = "Evening"; //set the weather info to Evening
      Blynk.setProperty(V2, "color", "#000000"); //set a button wheather info to black
      Blynk.virtualWrite(V3, weatherInfo); //send and display weather info in V3 data
      Blynk.virtualWrite(V6, "\xF0\x9F\x8C\x9B "); //Unicode (UTF-8) Character for 🌛 
    }
    else //Raining
    {
      buzzer(); //set buzzer active
      weatherInfo="Rainy Evening"; //set the weather info to Rainy Evening
      Blynk.virtualWrite(V3, weatherInfo); //send and display weather info in V3 data
      Blynk.virtualWrite(V6, "\xE2\x98\x94 "); //Unicode (UTF-8) Character for ☔
      if(ldrSensorAnalog >= 0) //if the brightness greater than or equal 0
      {
        Blynk.setProperty(V2, "color", "#000000"); //set a button wheather info to black
      }
    }
  } 
  
  //send and display data values to virtual pins in the blynk app
  Blynk.virtualWrite(V0, ldrSensorAnalog);  
  Blynk.virtualWrite(V1, rainSensorDigital);
  Blynk.virtualWrite(V2, colorStation);
  Blynk.virtualWrite(V4,"\xE2\x8C\x9A  ", currentTime);
  Blynk.virtualWrite(V5, currentDate); 
  Blynk.virtualWrite(V7, bp);
  Blynk.virtualWrite(V8, ba);
  Blynk.virtualWrite(V9, bt);
}

void loop(){
  Blynk.run();
  timer.run();
  lcd.setCursor(1,0);  
  lcd.print("SMART WEATHER");  
  lcd.setCursor(2,1);  
  lcd.print("INFORMATION");
}
