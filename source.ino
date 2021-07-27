//Comment the line below to use sensors
#define SIM

#include <M5StickC.h>
#include <vector>
#include "SmokeDetector.h"
#include "utils.h"

#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
#include "httpUtils.h"

#ifdef SIM
  #include "SmokeSimulator.h"
  SmokeSimulator* sim1;

  void generateSmokeSim()
  {
    std::vector<SmokeSession> sessions;
    sessions.push_back( {min2ms(3), min2ms(5) } ); //first smoke at 3. minute
    sessions.push_back( {min2ms(30), min2ms(6) } ); //second smoke at 30th minute
    sessions.push_back( {min2ms(60), min2ms(4) } ); //third smoke after an hour

    SmokeSimulatorSettings settings;
    settings.simSpeed = 10;
    settings.defVal = 250;
    settings.minVal = 0.62;
    settings.maxVal = 8000;
    settings.addSpeed = 80;
    settings.minRemSpeed = 10;
    settings.maxRemSpeed = 80;
    settings.speedErr = 0.2;
    settings.readErr = 0.2;
    settings.faultProb = 0;
    settings.restingHR = 83.8;
    settings.targetHR = 90.5;
    settings.incSpdHR = 0.0446;//go to target in 150 secs
    settings.decSpdHR = 0.005583;//go to resting in 1200 secs
    settings.readErrHR = 0.01;
    settings.restDist = 400;
    settings.smokeDist = 150;
    settings.readErrDist = 0.1;

    sim1 = new SmokeSimulator(settings, sessions);
  }
#else
  //Smoke detector
  int smkPin = 0;
  unsigned long lowPulseOccupancy = 0;
  
  //ToF
#include <Adafruit_VL53L0X.h>
  Adafruit_VL53L0X lox = Adafruit_VL53L0X();

  //Heart Rate
  int hrPin = 36;
#endif

//COMMON
unsigned long smokeReadPeriod = 30000;//every 30 secs
unsigned long prevSmokeRead = 0;
unsigned long hrReadPeriod = 2000;//every 2 secs
unsigned long prevHRRead = 0;
unsigned long distReadPeriod = 1000;//every sec
unsigned long prevDistRead = 0;

unsigned long curTime;//comes from SIM, or millis()
SmokeDetector* det1;

//Network related
const char *ssid = "mbtasw";
const char *password = "abdo123321";

// Your Domain name with URL path or IP address with path
const char* iftttApiKey = "dGxOYqscpZGZNdYkmY1eyn";
const char* event = "smoked";

void send2ifttt(double conc, unsigned short hr, unsigned short dist)
{
  // Send an HTTP GET request
  // Check WiFi connection status

  if(WiFi.status() != WL_CONNECTED)
  {
    Serial.println("WiFi Disconnected");
    return;
  }

  char buf[256];
  sprintf(buf, "https://maker.ifttt.com/trigger/%s/with/key/%s?value1=%.2f&value2=%hu&value3=%hu",
    event, iftttApiKey, conc, hr, dist );
  String jsonBuffer = httpGETRequest(buf);
  Serial.println(jsonBuffer);
  
  // JSONVar myObject = JSON.parse(jsonBuffer);
  // if (JSON.typeof(myObject) == "undefined") 
  // {
  //   Serial.println("Parsing input failed!");
  //   return;
  // }
  // Serial.print("JSON object = ");
  // Serial.println(myObject);
}

void myCB(unsigned long time, double conc, unsigned short hr, unsigned short dist)
{
  int ms, s, m, h;
  toPrettyTime(time, ms, s, m, h);

  Serial.printf("DETECTED : %d:%d:%d\n", h, m, s);
  Serial.printf("Concencration: %.2f, Heart Rate: %hu, Distance: %hu\n", conc, hr, dist);

  send2ifttt(conc, hr, dist);
}

void generateSmokeDetector()
{
  SmokeDetectorSettings settings;
  settings.concTH = 1500;
  settings.HRTH = 88;
  settings.distTH = 180;
  settings.waitTime = 10*60*1000;//10 minutes

  det1 = new SmokeDetector(settings, myCB);
}

void setup()
{
  M5.begin();
  //Serial.begin(9600);//ToF wants 115200?
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  generateSmokeDetector();
  
#ifdef SIM
  generateSmokeSim();
  sim1->start(millis());
#else
  pinMode(smkPin, INPUT);
  pinMode(hrPin, INPUT);
  gpio_pulldown_dis(GPIO_NUM_25); // GPIO 36 and 25 are shorted on M5STickC. so, pulldown is used to get clear data without noise

  // ToF
  // wait until serial port opens for native USB devices
  while(!Serial) delay(1);

  Serial.println("Adafruit VL53L0X test");
  if (!lox.begin(0x29, true)) {
    Serial.println("Failed to boot VL53L0X");
    while(1);
  }
  //---
#endif

}

void getConc()
{
#ifndef SIM
  auto dur = pulseIn(smkPin, LOW);
  lowPulseOccupancy += dur;
#endif

  if(curTime - prevSmokeRead >= smokeReadPeriod)
  {
    double conc = 0.0;
#ifdef SIM
    conc = sim1->getCurSmoke();
#else
    float ratio = lowPulseOccupancy/(smokeReadPeriod*10.0);  
    conc = 1.1*pow(ratio, 3) - 3.8*pow(ratio, 2) + 520 * ratio + 0.62; // using spec sheet curve
    lowPulseOccupancy = 0;
#endif
    det1->setConc(conc, curTime);
    prevSmokeRead = curTime;
  }
}

void getDist()
{
  if(curTime - prevDistRead >= distReadPeriod)
  {
    unsigned short dist = 0;
#ifdef SIM
    dist = sim1->getCurDist();
#else
    VL53L0X_RangingMeasurementData_t measure;
    Serial.print("Reading a measurement... ");
    lox.rangingTest(&measure, false); // pass in 'true' to get debug data printout!

    if(measure.RangeStatus == 4)
    {
      Serial.println(" out of range ");
      return;
    }

    Serial.print("Distance (mm): ");
    Serial.println(measure.RangeMilliMeter);
    dist = measure.RangeMilliMeter;
#endif
    //Serial.printf( "Time: %d:%d:%d, Distance: %hu\n",  h, m, s, dist );
    det1->setDist(dist, curTime);
    prevDistRead = curTime;
  }
}

void getHR()
{
  if(curTime - prevHRRead >= hrReadPeriod)
  {
    unsigned short hr = 0;
#ifdef SIM
    hr = sim1->getCurHR();
#else
    hr = analogRead(36);
#endif
    //Serial.printf( "Time: %d:%d:%d, Heart Rate: %.2f\n",  h, m, s, hr );
    det1->setHR(hr, curTime);
    prevHRRead = curTime;
  }
}

void loop()
{
#ifdef SIM
  sim1->step(millis());
  curTime = sim1->getTime();
#else
  curTime = millis();
#endif

  getConc();
  getDist();
  getHR();

#ifdef SIM
  delay(100); //needed for simulator
#endif
}
