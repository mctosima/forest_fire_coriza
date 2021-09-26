#include "TinyGPS++.h"
#include <SoftwareSerial.h>
#include <Wire.h>
#include <Adafruit_BMP280.h>

int FC = A0;
int fcValue;
SoftwareSerial gpsModul(11, 10); 
TinyGPSPlus gps;
SoftwareSerial sim(2, 3);
Adafruit_BMP280 bmp;
char Received_SMS;              
short CEK_OK=-1;     
String number = "+6282282630281"; 
String Pesan = "", Lat = "", Long = "", Humid = "", Pressure = "", Ketinggian ="", Zona ="", Keterangan = "";


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  if (!bmp.begin()) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring!"));
    while (1);
  }
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     
                  Adafruit_BMP280::SAMPLING_X2,    
                  Adafruit_BMP280::SAMPLING_X16,   
                  Adafruit_BMP280::FILTER_X16,     
                  Adafruit_BMP280::STANDBY_MS_500); 
  gpsModul.begin(9600);
  sim.begin(9600);
  delay(7000); 
  ReceiveMode(); 
}

void loop() {
  // put your main code here, to run repeatedly:
  String RSMS;
  fcValue = analogRead(FC);
  if (fcValue > 870) {
    Peringatan(fcValue);
  }
  
  while(sim.available()>0){      
        Received_SMS=sim.read();  
        Serial.print(Received_SMS);  
        RSMS.concat(Received_SMS);    
        CEK_OK=RSMS.indexOf("CEK");   
    }
    
    if(CEK_OK!=-1){                         
    Serial.println("found SMS");
    sim.end();     
    Pengecekan(fcValue);
    ReceiveMode();                  
    CEK_OK=-1;                      
  }
}

void Peringatan(int fcValue) {
  gpsModul.listen();
  Serial.println("Reading GPS");
  while (gpsModul.available()) {
    gps.encode(gpsModul.read());
  }
  if (gps.location.isUpdated()) 
  {
    Serial.println("GPS found");
    Pesan = "Peringatan Potensi Munculnya Api!";
    String Lat = String (gps.location.lat(), 6);
    String Long = String (gps.location.lng(), 6);
    String Humid = String(fcValue);
    String Pressure = String (bmp.readPressure());
    String Ketinggian = String (bmp.readAltitude(1013.25));
    if(bmp.readAltitude(1013.25) < 1500 ){
      Zona = "zona panas";
      String Keterangan = "pada ketinggian : " + Ketinggian + " mdpl (" + Zona + ") diharapkannya melakukan tindakan cepat terhadap peringatan ini.";
    }else{
      if(bmp.readAltitude(1013.25) > 2400){
        Zona = "zona sejuk";
      }else{
        Zona = "zona beriklim sedang";
      }
      String Keterangan = "pada ketinggian : " + Ketinggian + " mdpl (" + Zona + ").";
    }
    Sms(Pesan, Lat, Long, Humid, Pressure, Keteragan);
    delay(3000000); 
  }
}

void Pengecekan(int fcValue) {
  gpsModul.listen();
  delay(3000);
  Keterangan = "";
  while (gpsModul.available()) {
    gps.encode(gpsModul.read());
  }
  if (gps.location.isUpdated()) 
  {
    Pesan = "Pengecekan. ";
    String Lat = String (gps.location.lat(), 6);
    String Long = String (gps.location.lng(), 6);
    String Humid = String(fcValue);
    String Pressure = String (bmp.readPressure());
    Sms(Pesan, Lat, Long, Humid, Pressure);
    delay(3000); 
  }else{
    Serial.println("GPS not found");
    Pesan = "Pengecekan. ";
    String Humid = String(fcValue);
    String Pressure = String (bmp.readPressure());
    Sms(Pesan, Lat, Long, Humid, Pressure, Keterangan);
    delay(3000); 
  }
}

void Sms(String Pesan, String Lat, String Long, String Humid, String Pressure, String Keterangan) {
  sim.begin(9600);
  Serial.println("Sending SMS...");
  sim.print("AT+CMGF=1\r");    
  delay(1000);
  sim.print("AT+CMGS=\"" + number + "\"\r"); 
  delay(1000);
  String SMS = Pesan + "\nPada Latitude : " + Lat + " Longitude : " + Long + 
  "\ntingkat Humidity sebesar : " + Humid + " dan tekanan udara sebesar : " + Pressure + " Pa.\n"+ Keterangan;
  Serial.println(SMS);
  sim.print(SMS);
  delay(100);
  sim.print((char)26);
  delay(1000);
  Serial.println("SMS Sent.");
  delay(500);
}

void Serialcom() 
{
  delay(500);
  while(Serial.available())                                                                      
  {
    sim.write(Serial.read());
  }
  while(sim.available())                                                                      
  {
    Serial.write(sim.read());
  }
}

void ReceiveMode(){       
  sim.println("AT"); 
  Serialcom();
  sim.println("AT+CMGF=1"); 
  Serialcom();
  sim.println("AT+CNMI=2,2,0,0,0"); 
  Serialcom();
}
