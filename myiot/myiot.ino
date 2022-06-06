#include<ESP8266WiFi.h>                 
#include<FirebaseESP8266.h> 
#include<Servo.h>
#include <Adafruit_MLX90614.h>

#define FIREBASE_HOST "smartdoor-da7d1-default-rtdb.firebaseio.com"      
#define FIREBASE_AUTH "yYg9ICEddFCDYGb20F0ovp1WKX8oCFXFiYOuyHiy"            
#define WIFI_SSID "Nodemcu"                                  
#define WIFI_PASSWORD "smartdoor"

//Firebase
FirebaseData firebaseData;

//Buzzer
int buzzerPin=D5;
//#define buzzerPin D5

//servo
Servo myservo;
#define servoPin D4

//temperature sensor
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
double temp;

//IR sensor
#define irPinEntry D3
#define irPinExit D0

//LED
#define ledPinMask D6
#define ledPinTemp D7
#define ledPinCount D8

// count of total number of people inside
int count=0;
int totalLimit=10;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.setTimeout(2000);

  // Wait for serial to initialize.
  while(!Serial) { }

  Serial.println("Device Started");
  Serial.println("-------------------------------------");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);                                  
  Serial.print("Connecting to ");
  Serial.print(WIFI_SSID);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.begin(9600);
  Serial.println();
  Serial.print("Connected");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());    //prints local IP address
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  
  if (!mlx.begin()) {
    Serial.println("Error connecting to MLX sensor. Check wiring.");
    while (1);
  };
  Serial.println("Temperature Sensor Activated");

  myservo.attach(servoPin); //initialize digital pin for Servo Motor
  myservo.write(0);         //set position to 0 degrees
  delay(1000);

  pinMode(irPinEntry, INPUT); // initialize digital pin as an input for IR sensor at entry
  pinMode(irPinExit, INPUT); // initialize digital pin as an input for IR sensor at exit
  
  pinMode(ledPinMask, OUTPUT); //for LED as output for Mask
  pinMode(ledPinTemp, OUTPUT); //for LED as output for Temperature
  pinMode(ledPinCount, OUTPUT); //for LED as output for Total Count
  
  pinMode(buzzerPin,OUTPUT); //buzzer initialization
  Serial.println("buzzer initialization.. done");

  digitalWrite(ledPinCount,LOW); //turn count LED off
}

void loop() {
  // put your main code here, to run repeatedly:
  //if person exits
  if(digitalRead(irPinExit)==0){
    count--;
    Serial.print("Count="); Serial.println(count);
    Firebase.setInt(firebaseData,"/Person Count", count);
  }
  //if total limit reached
  if(count>=totalLimit){
    Serial.println("Total Limit Reached");
    digitalWrite(ledPinCount,HIGH); //turn count LED on
    delay(200);
  }
  digitalWrite(ledPinMask,HIGH); //turn Mask LED on 
  digitalWrite(ledPinTemp,HIGH); //turn temperature LED on
  digitalWrite(ledPinCount,LOW); //turn count LED off
  digitalWrite (buzzerPin, HIGH);  //turn buzzer off
  delay(200);
  //detect mask
  if(count<totalLimit){
    if(Firebase.getInt(firebaseData, "/test")){   //gets the status of Mask Detection
   if(firebaseData.dataType()=="int"){
      Serial.print("Value=");
      int val = firebaseData.intData();
      Serial.println(val);
      //if mask detected
      if(val==1){
        digitalWrite(ledPinMask,LOW); //turn LED off
        delay(3000);
        temp = mlx.readObjectTempC();
        temp=temp-1000.00;
        Serial.print(temp); Serial.println("*C");
        //Firebase.setInt(firebaseData,"/Temperature", temp);
        //if temperature detected
        if (temp<38.00 && temp>=37.00)
        {
          digitalWrite(ledPinTemp,LOW); //turn LED off
          Serial.println("NORMAL TEMPERATURE");
          myservo.write(180);
          delay(1000);
          while(digitalRead(irPinEntry)){
            Serial.println("Waiting for Person to Pass");
          }
          count++;
          myservo.write(0); //set position to 0 degrees
          Serial.println("Count Increased by 1");
          Firebase.setInt(firebaseData,"/Person Count", count);
          Firebase.setInt(firebaseData,"/test", 0);
        }
        else{
            //if temperature not normal
            Serial.println("TEMPERATURE WARNINGGG!!!");
            //turn on Buzzer
            digitalWrite (buzzerPin, LOW);  //turn buzzer off
            delay(500);
            digitalWrite (buzzerPin, HIGH); //turn buzzer on
            delay(500);
        }
        Firebase.setInt(firebaseData,"/Temperature", temp);
      }
      else{
          //if mask not detected
          Serial.println("Mask Not Detected");
          //turn on Buzzer
          //digitalWrite (buzzerPin, LOW);  //turn buzzer off
          //delay(500);
          //digitalWrite (buzzerPin, HIGH); //turn buzzer on
          //delay(500);
      }
    }
  }
  else{
    Serial.println("No Status at the Server End");
  }
  }
  Serial.print("Count="); Serial.println(count);
}
