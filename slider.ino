#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define pin_CLK 4
#define pin_DT  5
#define pin_Btn 6
#define stp_Dir 7
#define stp_Step 8
#define cam_pin 9

LiquidCrystal_I2C lcd(0x27, 16, 2);

unsigned long CurrentTime, LastTime;
enum eEncoderState {eNone, eLeft, eRight, eButton};
uint8_t EncoderA, EncoderB, EncoderAPrev;
int8_t counter, brightness1, brightness2;
bool ButtonPrev;
byte cMode = 0; // 0 - start, 1 - установка направления, 2 - установка шагов, 3 - установка выдержки, 4 - примерное время, 5 - пуск
byte cKey = 0; // 1 - left, 2 - right, 3 - button
byte cSliderDirection = 0; // 0 - left, 1 - right
int cSliderSteps = 50; // количество шагов слайдера
int cSliderExposure = 30; // выдержка фотоаппарата
int nExp;
int nSteps;

eEncoderState GetEncoderState() {
  // Считываем состояние энкодера
  eEncoderState Result = eNone;
  CurrentTime = millis();
  if (CurrentTime - LastTime >= 5) {
    // Считываем не чаще 1 раза в 5 мс для уменьшения ложных срабатываний
    LastTime = CurrentTime;
    if (digitalRead(pin_Btn) == LOW ) {
      if (ButtonPrev) {
        Result = eButton; // Нажата кнопка
        ButtonPrev = 0;
      }
    }
    else {
      ButtonPrev = 1;
      EncoderA = digitalRead(pin_CLK);
      EncoderB = digitalRead(pin_DT);
      if ((!EncoderA) && (EncoderAPrev)) { // Сигнал A изменился с 1 на 0
        if (EncoderB) Result = eRight;     // B=1 => энкодер вращается по часовой
        else          Result = eLeft;      // B=0 => энкодер вращается против часовой
      }
      EncoderAPrev = EncoderA; // запомним текущее состояние
    }
  }
  return Result;
}

void calcTime()
{
  double nTime = (6500 / cSliderSteps * (1.307 + cSliderExposure) + 65);
  int nHour = nTime / 3600;
  int nMinute = (nTime - nHour * 3600) / 60;
  int nSeconds = nTime - nHour * 3600 - nMinute * 60;
  String cTime = String(nHour) + "h " + String(nMinute) + "m " + String(nSeconds) + "s";
  lcd.setCursor(0, 0);
  lcd.print("Aproximate time ");
  lcd.setCursor(0, 1); 
  lcd.print(cTime); 
}

void setup() {
  lcd.init();
  pinMode(pin_DT,  INPUT);
  pinMode(pin_CLK, INPUT);
  pinMode(pin_Btn, INPUT_PULLUP);
  pinMode(stp_Dir, OUTPUT);
  pinMode(stp_Step, OUTPUT);
  pinMode(cam_pin, OUTPUT); 
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Slider v2.0     ");  
  lcd.setCursor(0, 1);
  lcd.print("Push Button     ");
}

void loop() {
  cKey = 0;
  
  // опрос кнопок
  switch (GetEncoderState()) {
    case eNone: return;
    case eLeft: {   // Энкодер вращается влево
        cKey = 1;
        break;
      }
    case eRight: {  // Энкодер вращается вправо
        cKey = 2;
        break;
      }
    case eButton: { // Нажали кнопку
        cKey = 3;
        break;
      }
  }
  
  // вход в меню после запуска
  if (cMode == 0) {
    lcd.setCursor(0, 0);
    lcd.print("Slider v2.0     ");  
    lcd.setCursor(0, 1);
    lcd.print("Push Button     ");
    if (cKey == 3) {
     cMode = 1;
     cKey = 0;
     delay(500);
    }  
  }

  // установка направления вращения
  if (cMode == 1) {
    if (cKey == 1) {
      cSliderDirection = 0;
    }
    if (cKey == 2) {
      cSliderDirection = 1;
    }
    lcd.setCursor(0, 0);
    lcd.print("Slider direction");  
    lcd.setCursor(0, 1);
    if (cSliderDirection == 0) {
      lcd.print("Left            ");
    } else {
      lcd.print("Right           ");
    }
    if (cKey == 3) {
      cMode = 2;
      delay(500);
    }
    cKey = 0;
  }
  
  // установка количество шагов
  if (cMode == 2) {
    if ((cKey == 1) && (cSliderSteps > 1)) {
      cSliderSteps--;
    }
    if ((cKey == 2) && (cSliderSteps < 5000)) {
      cSliderSteps++;
    }
    lcd.setCursor(0, 0);
    lcd.print("Slider steps    ");  
    lcd.setCursor(0, 1);
    lcd.print("                ");
    lcd.setCursor(0, 1);
    lcd.print(cSliderSteps);
    if (cKey == 3) {
      cMode = 3;
      delay(500);
    }
    cKey = 0;
  }

  // установка выдержки
  if (cMode == 3) {
    if ((cKey == 1) && (cSliderExposure > 1)) {
      cSliderExposure--;
    }
    if ((cKey == 2) && (cSliderExposure < 300)) {
      cSliderExposure++;
    }
    lcd.setCursor(0, 0);
    lcd.print("DSLR exposure(s)");  
    lcd.setCursor(0, 1);
    lcd.print("                ");
    lcd.setCursor(0, 1);
    lcd.print(cSliderExposure);
    if (cKey == 3) {
      cMode = 4;
      delay(500);
    }
    cKey = 0;
  }

  // примерное время съёмки
  if (cMode == 4) {
    calcTime();
    if (cKey == 3) {
      cMode = 5;
      delay(500); 
    }
    cKey = 0; 
  }

  // последнее меню перед стартом
  if (cMode == 5) {
    lcd.setCursor(0, 0);
    lcd.print("Button - start  ");  
    lcd.setCursor(0, 1);
    lcd.print("LR - return     ");
    // вернёмся в меню
    if ((cKey == 1) || (cKey == 2)) {
      cMode = 0;
      cSliderDirection = 0;
      cSliderSteps = 50;
      cSliderExposure = 30;
    }
    // старт
    if (cKey == 3) {
      cMode = 6;
      lcd.setCursor(0, 1);
      lcd.print("                ");
      delay(500);
    }
    cKey = 0;    
  }

  // рабочий цикл
  if ((cMode == 6) || (cMode == 7)) {
    lcd.setCursor(0, 0);
    lcd.print("Don't panic!    ");
    // установим направление движения
    if (cSliderDirection == 0) {
      digitalWrite(stp_Dir, LOW);
    } else {
      digitalWrite(stp_Dir, HIGH);
    }
    // цикл съёмки    
    while (cMode == 6) {
      // делаем снимок
      digitalWrite(cam_pin, HIGH);
      nExp = 1;
      while (nExp <= cSliderExposure) {
        delay(1000);
        nExp++;
      }
      digitalWrite(cam_pin, LOW);
      
      // двигаем фотоаппарат
      nSteps = 1;
      while (nSteps <= cSliderSteps){
        digitalWrite(stp_Step, HIGH);
        delayMicroseconds(10);
        digitalWrite(stp_Step, LOW);
        delay(10);
        nSteps++;
      }
    }
  }
}
