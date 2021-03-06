#include <Wire.h>

// CURRENT AVAILABLE BITS
// 0X02

// Digital
#define S0 4
#define S1 5
#define S2 6
#define S3 7
#define Colour 3

// Analog
#define Flame A1
#define FlameRight A2
#define FlameLeft A0
#define Hall A3

// IMU Registers 
#define PAGE_SWAP     0x07
#define ACC_CONF      0x08
#define GYR_CONF_0    0x0A
#define GYR_CONF_1    0x0B
#define MAG_CONF      0x09
#define TEMP_SOURCE   0x40
#define UNIT_SEL      0x3B
#define PWR_MODE      0x3E

#define ADDRESS       0x28
#define HEADING       0x1A
#define ORIENTATION   0x20
#define XACCEL        0x28
#define ZGYRO         0x18
#define MODE_REG      0x3D

#define FUSION_MODE   0x0C // NDOF?

// Globally Store Detected Colours
int colours[4];

// Global byte array for all of our IMU data
// 8 for LSB and MSB of Orientation Quaternion x,y,z and w
// 2 for LSB and MSB of Linear Acceleration in x
// 2 for LSB And MSB of Gyro in Z
byte data[13];

void setup() 
{
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  
  pinMode(Colour, INPUT);
  pinMode(Flame, INPUT);
  pinMode(FlameLeft, INPUT);
  pinMode(FlameRight, INPUT);
  pinMode(Hall, INPUT);
  
  // Setting frequency-scaling
  digitalWrite(S0,HIGH);
  digitalWrite(S1,LOW);
  
  Serial.begin(9600);
  Wire.begin(0x07); //Set Arduino up as an I2C slave at address 0x07
  Wire.onRequest(requestEvent); //Prepare to send data
  Wire.onReceive(receiveEvent); //Prepare to recieve data

  // Config IMU 
  delay(20);
  config_BNO055();
  delay(20);
}

void loop()
{  
  // Request the IMU to write the orientation registers
  Wire.beginTransmission(ADDRESS);  
  Wire.write(ORIENTATION);
  Wire.endTransmission(false);

  // Read the data from orientation registers
  Wire.requestFrom(ADDRESS, 8);

  while(Wire.available() < 8);

  data[0] = Wire.read();  // Orientation W LSB
  data[1] = Wire.read();  // Orientation W MSB
  data[2] = Wire.read();  // Orientation X LSB
  data[3] = Wire.read();  // Orientation X MSB
  data[4] = Wire.read();  // Orientation Y LSB
  data[5] = Wire.read();  // Orientation Y MSB
  data[6] = Wire.read();  // Orientation Z LSB
  data[7] = Wire.read();  // Orientation Z MSB

  // Request the IMU to write the acceleration registers
  Wire.beginTransmission(ADDRESS);
  Wire.write(XACCEL);
  Wire.endTransmission(false);

  // Read the data from acceleration registers
  Wire.requestFrom(ADDRESS, 2);

  while(Wire.available() < 2);  

  data[8] = Wire.read();  // X accel LSB
  data[9] = Wire.read();  // X accel MSB

  // Request the IMU to write the gyro registers
  Wire.beginTransmission(ADDRESS);
  Wire.write(ZGYRO);
  Wire.endTransmission();

  // Read the data from gyro registers
  Wire.requestFrom(ADDRESS, 2);

  while(Wire.available() < 2);

  data[10] = Wire.read();  // Z Gyro LSB
  data[11] = Wire.read();  // Z Gyro MSB
  
  data[12] = readFlameSensor() ^ readColourSensor();

  for (int i = 0; i < 13; ++i)
  {
    Serial.print(data[i]);
    Serial.print(" ");
  }
  Serial.println();
}

void requestEvent()
{
}

void receiveEvent(int numBytes)
{
  //Set Up Vars
  int receive_int=0;
  int count=0;

  //We'll recieve one byte at a time. Stop when none left
  while(Wire.available())
  {
    char c = Wire.read(); // receive a byte as character
    //Create Int from the Byte Array
    receive_int = c << (8 * count) | receive_int;
    count++;
  }
  Serial.print("Received Number: ");
  Serial.println(receive_int);
}

byte readFlameSensor()
{
  int sensorValueFront = analogRead(Flame);
  int sensorValueLeft = analogRead(FlameLeft);
  int sensorValueRight = analogRead(FlameRight);

  byte returnVal = 0x00;
  
  /*
  Serial.print("Flame Front: ");
  Serial.print(sensorValueFront);
  Serial.print(" Flame Right: ");
  Serial.print(sensorValueRight);
  Serial.print(" Flame Left: ");
  Serial.print(sensorValueLeft);
  */
  
  if(sensorValueFront < 400)
  {
    returnVal = returnVal ^ 0x01;
  }
  if(sensorValueLeft < 600)
  {
    returnVal = returnVal ^ 0x10;
  }
  if(sensorValueRight < 600)
  {
    returnVal = returnVal ^ 0x20;
  }
  Serial.println("");
  
  return returnVal;
}

byte readColourSensor()
{
  ReadRGBC();
  
  if (buildingDetection())
  {
    return 0x04;
  }

  return 0x00;
}

void ReadRGBC()
{
  // RED
  digitalWrite(S2,LOW);
  digitalWrite(S3,LOW);
  colours[0] = pulseIn(Colour, LOW);
  
  // Green
  digitalWrite(S2,HIGH);
  digitalWrite(S3,HIGH);
  colours[1] = pulseIn(Colour, LOW);

  // Blue 
  digitalWrite(S2,LOW);
  digitalWrite(S3,HIGH);
  colours[2] = pulseIn(Colour, LOW);
  
  // Clear
  digitalWrite(S2,HIGH);
  digitalWrite(S3,LOW);
  colours[3] = pulseIn(Colour, LOW);

  /*
  Serial.print(colours[0]);
  Serial.print(", ");
  Serial.print(colours[1]);
  Serial.print(", ");
  Serial.print(colours[2]);
  Serial.print(", ");
  Serial.println(colours[3]);
  */
}

bool buildingDetection()
{
  bool returnVal = 
    colours[0] > 88 && colours[0] < 125 && // Red in range
    colours[1] > 150 && colours[1] < 221 && // Blue in range
    colours[2] > 177 && colours[2] < 122 && // Green in range
    colours[3] > 45 && colours[3] < 65;
  return returnVal;
}

/////////////////////
//                 //
//    IMU Code     //
//                 //
/////////////////////

void config_BNO055()
{
   Wire.beginTransmission(ADDRESS);
   Wire.write(PAGE_SWAP);                // Page swap
   Wire.write(1);                        // To page 1
   Wire.endTransmission();
    
   Wire.beginTransmission(ADDRESS);
   Wire.write(ACC_CONF);                // ACC Configuration
   Wire.write(0x08);                    // 0x08 acc conf| normal power, 31.25hz bandwidth, 2G range
   Wire.endTransmission();

   Wire.beginTransmission(ADDRESS);
   Wire.write(GYR_CONF_0);                // GYR Configuration
   Wire.write(0x23);                      // 0x32 250DPS scale, 23hz bandwidth
   Wire.endTransmission();

   Wire.beginTransmission(ADDRESS);
   Wire.write(GYR_CONF_1);                // GYR Configuration
   Wire.write(0x00);                      // Normal power
   Wire.endTransmission();
   
   Wire.beginTransmission(ADDRESS);
   Wire.write(MAG_CONF);                  // MAG Configuration
   Wire.write(0x1B);                      // Normal power, High accuracy, 10Hz output rate
   Wire.endTransmission();
   
   Wire.beginTransmission(ADDRESS);
   Wire.write(PAGE_SWAP);                // Page swap
   Wire.write(0);                        // To page 0
   Wire.endTransmission();
   
   Wire.beginTransmission(ADDRESS);
   Wire.write(TEMP_SOURCE);               // Temperature source
   Wire.write(0x01);                      // gyro
   Wire.endTransmission();
   
   Wire.beginTransmission(ADDRESS);
   Wire.write(UNIT_SEL);                  // Unit select
   Wire.write(0x01);                      // Temp in Degrees C, Heading in Degrees, Angular rate in DPS, Gravity in mg
   Wire.endTransmission();
   
   Wire.beginTransmission(ADDRESS);
   Wire.write(PWR_MODE);                  // Power mode normal
   Wire.write(0x00);                      // normal
   Wire.endTransmission();
   
   Wire.beginTransmission(ADDRESS);
   Wire.write(MODE_REG);                  // Operational mode NDOF
   Wire.write(FUSION_MODE);
   Wire.endTransmission();
}
