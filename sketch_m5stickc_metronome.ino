//M5StickC viveration metronome.
// By Takuhiro Mizuno
//https://twitter.com/mizunon

//Usage:
// BtnA('M5'): Start/Stop metronome.
// PowerBtn(upper side): Tempo Up or Volume Up.
// BtnB(lower side): Tempo down or Volume Down.
// lond press BtnB: Change tempo cursol.

#include <M5StickC.h>

//You may need change 'default.csv' for use EEPROM.
//eeprom,   data, 0x99,  0x290000,0x1000,
//spiffs,   data, spiffs,  0x291000,0x16F000,
#include <EEPROM.h>

//Metronome tone. default:C6
#define clinkTone 1046
//M5StickC Speaker Hat.
#define GPIO_SPEAKER_PIN 26
#define GPIO_SPEAKER_SD_PIN 0
#define GPIO_SPEAKER_NC_PIN 36

//Grove - Vibration Motor SKU:105020003
//Grove port. I2C clock -> DOUT
#define GPIO_VIBE_PIN  33
//Grove port. I2C data -> DOUT(NC)
#define GPIO_VIBE_NC_PIN 32


//Global variables.//

// Tempo(bpm). default:120
int bpm = 120;
//Click volume. default:100
int volume = 100;

//bpmBase in microsec
unsigned long bpmBaseMs;
unsigned long playProgressMs = 0;
bool isClickLEDOn = false;
bool isPlay = false;
int bpmCursol = 0;

double vBattery = 0.0;
int batteryP100 = 0;


// Functions //

void ReadConfig(){
  if( EEPROM.read(0) == 'M' && EEPROM.read(1) == 'T' ){
    bpm = (int)EEPROM.read(2);
    volume = (int)EEPROM.read(3);
  }

  if( bpm < 30 || 300 < bpm ) bpm = 120;
  if( volume < 0 || 300 < volume ) volume = 100;
}

void writeConfig(){
  EEPROM.write( 0, (byte)'M' );
  EEPROM.write( 1, (byte)'T' );

  EEPROM.write( 2, (byte)bpm );
  EEPROM.write( 3, (byte)volume );

  EEPROM.commit();
}


int backGroundColor = BLACK;
int prevBackGroundColor = BLACK;
void PrintInfos()
{
  if( 20 <= batteryP100 ) backGroundColor = BLACK;
  else backGroundColor = RED;

  if( prevBackGroundColor != backGroundColor ) M5.Lcd.fillScreen(backGroundColor);
  prevBackGroundColor = backGroundColor;
  
  if( !isPlay ) M5.Lcd.setTextColor(GREEN, backGroundColor);
  else M5.Lcd.setTextColor(PINK, backGroundColor);
  
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.print("Tempo:");
  M5.Lcd.print(bpm);
  M5.Lcd.println("  ");

  M5.Lcd.print("Batt :");
  M5.Lcd.print(batteryP100);
  M5.Lcd.println("%  ");
  
  if( !isPlay ){
    M5.Lcd.println("Change tempo ");
    //M5.Lcd.print(" Up  :+"); M5.Lcd.printf("%3d",(int)pow(10, bpmCursol)); M5.Lcd.println("   ");
    //M5.Lcd.print(" Down:-"); M5.Lcd.printf("%3d",(int)pow(10, bpmCursol)); M5.Lcd.println("   ");
    M5.Lcd.print  (" UpDn:+-"); M5.Lcd.printf("%3d",(int)pow(10, bpmCursol)); M5.Lcd.println("  ");
    M5.Lcd.println(" DnLp:Chg cur");
  }else{
    M5.Lcd.println("Fight! Mai!  ");
    M5.Lcd.println("Up  :Vol Up  ");
    M5.Lcd.println("Down:Vol Down");
  }

  while( isClickLEDOn ){
    dacWrite(GPIO_SPEAKER_PIN, volume - 50);
    delayMicroseconds(500);
    dacWrite(GPIO_SPEAKER_PIN, 0);
    delayMicroseconds(500);
  }
  
  delay(10UL);

}

bool isCursolChanged = false;
void SelectBPM()
{
  if( isCursolChanged && M5.BtnB.isReleased() )
  {
    isCursolChanged = false;
  }else if (M5.BtnB.pressedFor(1000UL) && isCursolChanged == false)
  {
    bpmCursol++;
    if( 2 < bpmCursol ) bpmCursol = 0;

    isCursolChanged = true;
  }else if (M5.BtnB.wasReleasefor(100UL))
  {
    bpm -= pow(10, bpmCursol);
    if( bpm < 30 ) bpm = 30;
    AdjustBPM();
    
    writeConfig();
  }
  else if (M5.Axp.GetBtnPress() == 2)
  {
    bpm += pow(10, bpmCursol);
    if( 300 < bpm ) bpm = 300;
    AdjustBPM();
    
    //PrintInfos();
    writeConfig();
  }
}

void AdjustBPM()
{
  // 60sec * 1000miillis * 1000micros
  const unsigned long bpm1BaseMs = 60000000UL;

  if( bpm < 30 ) bpm = 120;
  bpmBaseMs = (unsigned long)(bpm1BaseMs / (unsigned long)bpm);

  playProgressMs = 0;
}

void SelectVolume()
{
  if (M5.BtnB.wasPressed())
  {
    volume -= 50;
    if( volume < 0 ) volume = 0;
    
    writeConfig();
  }
  else if (M5.Axp.GetBtnPress() == 2)
  {
    volume += 50;
    if( 300 < volume ) volume = 300;
    
    writeConfig();
  }
}

unsigned long surplusMillis;
unsigned long surplusMicros;
void MetronomePlay()
{
  if( playProgressMs <= 0 ) playProgressMs = bpmBaseMs;

  if( (bpmBaseMs - playProgressMs) == 0 ){
    if( 0 < volume ){
      digitalWrite(M5_LED, LOW);
      isClickLEDOn = true;
    }
    digitalWrite( GPIO_VIBE_PIN, HIGH );
  }else if( (bpmBaseMs - playProgressMs) >= 50000UL ){
    digitalWrite(M5_LED, HIGH);
    isClickLEDOn = false;
    digitalWrite( GPIO_VIBE_PIN, LOW );
  }

  if( 50000UL < playProgressMs){
    delay(50UL);
    playProgressMs -= 50000UL;
  }else{
    surplusMillis = playProgressMs / 1000UL;
    surplusMicros = playProgressMs - (1000UL * surplusMillis );
    
    if( 0 < surplusMillis ) delay(surplusMillis);
    if( 0 < surplusMicros ) delayMicroseconds(surplusMicros);
    
    playProgressMs = 0;
  }
}

unsigned long prevTime = 0;
void loopClickTask(void *pvParameters)
{
  for(;;){
    M5.update();
    
    if( (millis() - prevTime) > 1000UL ){
      vBattery = M5.Axp.GetVbatData() * 1.1 / 1000;
      batteryP100 = (int)((vBattery - 3.2) / 1.0 * 100);
      if( batteryP100 > 100 ) batteryP100 = 100;
      if( batteryP100 <   0 ) batteryP100 = 0;
  
      prevTime = millis();
    }
    
    if (M5.BtnA.wasPressed()){
      isPlay = !isPlay;
      
      if( isPlay ){
        AdjustBPM();
        
        digitalWrite( GPIO_SPEAKER_SD_PIN, HIGH );
      }else{
        isClickLEDOn = false;

        //LED OFF.
        digitalWrite( M5_LED, HIGH );
        //Viberation OFF.
        digitalWrite( GPIO_VIBE_PIN, LOW );
        //Speaker noiseproof.
        digitalWrite( GPIO_SPEAKER_SD_PIN, LOW );
      }
    }
    
    if ( isPlay )
    {
      SelectVolume();
      
      MetronomePlay();
      //delay in MetronomePlay().
    }else{
      SelectBPM();
      
      delay(100UL);
    }
  }
}

void setup()
{
  M5.begin();

  pinMode( GPIO_VIBE_PIN, OUTPUT );
  digitalWrite( GPIO_VIBE_PIN, LOW );
  
  pinMode( GPIO_VIBE_NC_PIN, OUTPUT );
  digitalWrite( GPIO_VIBE_NC_PIN, LOW );

  pinMode(M5_LED, OUTPUT);
  digitalWrite(M5_LED, HIGH);

  pinMode(GPIO_SPEAKER_PIN, OUTPUT);
  dacWrite( GPIO_SPEAKER_PIN, 0 );

  pinMode( GPIO_SPEAKER_SD_PIN, OUTPUT );
  digitalWrite( GPIO_SPEAKER_SD_PIN, LOW );

  pinMode( GPIO_SPEAKER_NC_PIN, OUTPUT );
  digitalWrite( GPIO_SPEAKER_NC_PIN, LOW );

  // defalut:240MHz 240, 160, 80, 40, 20, 10
  while(!setCpuFrequencyMhz(240)){;}
  Serial.print("getCpuFrequencyMhz:");
  Serial.println(getCpuFrequencyMhz());

  Serial.print("portNUM_PROCESSORS:");
  Serial.println(portNUM_PROCESSORS);

  Serial.print("xPortGetCoreID():");
  Serial.println(xPortGetCoreID());

  M5.Axp.ScreenBreath(9); // 7 - 15

  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setRotation(3);
  M5.Lcd.setTextColor(GREEN, BLACK);
  M5.Lcd.setTextSize(2);

  vBattery = M5.Axp.GetVbatData() * 1.1 / 1000;
  Serial.print("M5.Axp.GetVbatData:");
  Serial.println( vBattery );

  M5.BtnA.read();

  EEPROM.begin(8);
  if (M5.BtnA.isPressed()) writeConfig();
  ReadConfig();

  AdjustBPM();

  //fname, fnameStr, StackSize, NULL, priority, taskHandle, CoreID
  xTaskCreateUniversal(loopClickTask, "loopClickTask", 4096, NULL, 1, NULL, 0);
  //xTaskCreateUniversal(PrintInfos, "PrintInfos", 4096, NULL, 1, NULL, 1);
}


void loop()
{
  PrintInfos();
}
