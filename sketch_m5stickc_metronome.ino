#include <M5StickC.h>

//default.csvの書き換えが必要
//eeprom,   data, 0x99,  0x290000,0x1000,
//spiffs,   data, spiffs,  0x291000,0x16F000,
#include <EEPROM.h>

// メトロノーム音を設定
#define clinkTone 1046 //C6

#define GPIO_SPEAKER_PIN 26

// テンポ変数 デフォルト120
int bpm = 120;
unsigned long playProgress = 0;

int volume = 100;

// 音符変数の宣言
unsigned long noteBaseMs; // 1テンポあたりのms

bool clickLEDOn = false;

// ステート変数の宣言
int runstate = 0;
int bpmCursol = 0;

int vibePinCtrl = 33;
int vivePinData = 32;

double vbat = 0.0;
int battery_p100 = 0;

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
  //for(;;){
    if( 20 <= battery_p100 ) backGroundColor = BLACK;
    else backGroundColor = RED;

    if( prevBackGroundColor != backGroundColor ) M5.Lcd.fillScreen(backGroundColor);
    prevBackGroundColor = backGroundColor;
    
    if( runstate == 0) M5.Lcd.setTextColor(GREEN, backGroundColor);
    else M5.Lcd.setTextColor(PINK, backGroundColor);
    
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.print("Tempo:");
    M5.Lcd.print(bpm);
    M5.Lcd.println("  ");
  
    M5.Lcd.print("Batt :");
    M5.Lcd.print(battery_p100);
    M5.Lcd.println("%  ");
    if( runstate == 0 ){
      //M5.Lcd.println("           ");
      M5.Lcd.println("Change tempo");
      M5.Lcd.print(" Up  :+"); M5.Lcd.printf("%3d",(int)pow(10, bpmCursol)); M5.Lcd.println("  ");
      M5.Lcd.print(" Down:-"); M5.Lcd.printf("%3d",(int)pow(10, bpmCursol)); M5.Lcd.println("  ");
    }else{
      M5.Lcd.println("Fight! Mai!  ");
      M5.Lcd.println("Up  :Vol Up  ");
      M5.Lcd.println("Down:Vol Down");
    }

    while( clickLEDOn ){
      dacWrite(GPIO_SPEAKER_PIN, 0);
      delayMicroseconds(500);
      dacWrite(GPIO_SPEAKER_PIN, volume - 50);
      delayMicroseconds(500);
    }
    delay(1UL);
  //}
}

bool isCursolChange = false;
void SelectBPM()
{
  if( isCursolChange && M5.BtnB.isReleased() )
  {
    isCursolChange = false;
  }else if (M5.BtnB.pressedFor(1000UL) && isCursolChange == false)
  {
    bpmCursol++;
    if( 2 < bpmCursol ) bpmCursol = 0;

    isCursolChange = true;
  }else if (M5.BtnB.wasReleasefor(100UL))
  {
    bpm -= pow(10, bpmCursol);
    if( bpm < 30 ) bpm = 30;
    AdjustBPM();
    
    //PrintInfos();
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
  // 音の長さをBPMに合わせる
  const unsigned long bpmBaseMs = 60000UL;
  
  noteBaseMs = (unsigned long)(bpmBaseMs / (unsigned long)bpm);

  playProgress = 0;
}

void SelectVolume()
{
  if (M5.BtnB.wasPressed())
  {
    volume -= 50;
    if( volume < 0 ) volume = 0;
    //M5.Speaker.setVolume(volume);
    
    //PrintInfos();
    writeConfig();
  }
  else if (M5.Axp.GetBtnPress() == 2)
  {
    volume += 50;
    if( 300 < volume ) volume = 300;
    //M5.Speaker.setVolume(volume);
    
    //PrintInfos();
    writeConfig();
  }
}


void MetronomePlay()
{
  if( playProgress <= 0 ) playProgress = noteBaseMs;

  if( (noteBaseMs - playProgress) == 0 ){
    if( 0 < volume ){
      //M5.Speaker.tone( clinkTone );
      digitalWrite(M5_LED, LOW);
      clickLEDOn = true;
    }
    digitalWrite( vibePinCtrl, HIGH );
  }else if( (noteBaseMs - playProgress) >= 50UL ){
    //M5.Speaker.mute();
    digitalWrite(M5_LED, HIGH);
    clickLEDOn = false;
    digitalWrite( vibePinCtrl, LOW );
  }
  //M5.Speaker.update();

  if( 50UL < playProgress){
    delay(50UL);
    playProgress -= 50UL;
  }else{
    delay(playProgress);
    playProgress = 0;
  }
}

void CheckRunState()
{
  if (M5.BtnA.wasPressed())
  {
    switch (runstate)
    {
    case 0:
      AdjustBPM();
      runstate = 1;
      break;

    case 1:
      runstate = 0;
      break;
    }
    
    //PrintInfos();
  }
}

int lcd_rewrite_counter = 0;
int lcd_rewrite_pbat = 0;
void loopClickTask(void *pvParameters)
{
  for(;;){
    M5.update();
  
    vbat = M5.Axp.GetVbatData() * 1.1 / 1000;
    battery_p100 = (int)((vbat - 3.2) / 1.0 * 100);
    if( battery_p100 > 100 ) battery_p100 = 100;
    if( battery_p100 <   0 ) battery_p100 = 0;
  
    CheckRunState();
    if (runstate == 1)
    {
      MetronomePlay();
      SelectVolume();
  
      if( battery_p100 % 10 == 0 ){
        if( lcd_rewrite_pbat != battery_p100 ){
          //PrintInfos();
          lcd_rewrite_pbat = battery_p100;
        }else{
          lcd_rewrite_pbat = battery_p100;
        }
      }
      
    }else{
      SelectBPM();
  
      //M5.Speaker.mute();
      digitalWrite( M5_LED, HIGH );
      clickLEDOn = false;
      
      digitalWrite( vibePinCtrl, LOW );
  
      //PrintInfos();
      lcd_rewrite_pbat = 0;
      
      delay(100UL);
    }
  }
}

void setup()
{
  M5.begin();

  pinMode( vibePinCtrl, OUTPUT );
  digitalWrite( vibePinCtrl, LOW );
  
  pinMode( vivePinData, OUTPUT );
  digitalWrite( vivePinData, LOW );

  pinMode(M5_LED, OUTPUT);
  digitalWrite(M5_LED, HIGH);

  pinMode(GPIO_SPEAKER_PIN, OUTPUT);
  dacWrite( GPIO_SPEAKER_PIN, 0 );

  // デフォルトは240MHzで、240, 160, 80, 40, 20, 10から選択可
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

  vbat = M5.Axp.GetVbatData() * 1.1 / 1000;
  Serial.print("M5.Axp.GetVbatData:");
  Serial.println( vbat );

  M5.BtnA.read();  //ボタン状況更新

  EEPROM.begin(8);
  if (M5.BtnA.isPressed()) writeConfig();
  ReadConfig();

  AdjustBPM();
  //PrintInfos();

  //fname, fnameStr, StackSize, NULL, priority, taskHandle, CoreID
  xTaskCreateUniversal(loopClickTask, "loopClickTask", 4096, NULL, 1, NULL, 0);
  //xTaskCreateUniversal(PrintInfos, "PrintInfos", 4096, NULL, 1, NULL, 1);
}


void loop()
{
  PrintInfos();
}
