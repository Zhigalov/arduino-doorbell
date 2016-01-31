/**
 * Дверной звонок
 *
 * @version v2.0
 * @author Илья Телегин <ilya.a.telegin@gmail.com>, Сергей Жигалов <sergey.zhigalov@gmail.com>
 */
const int BELL_PIN = 3;              // Пин размыкания номеронабирателя (при наборе цифры три произойдет три размыкания) [синий]
const int SIG_PIN = 4;               // Пин состояния номеронабирателя (1 - идет набор, 0 - нихера не происходит) [белый]
const int RED = 5;                   // Красный светодиод [красный]
const int GREEN = 6;                 // Красный светодиод [зеленый]
const int CLK = 8;                   // CLK для Wave shield 
const int DAT = 9;                   // DAT для Wave shield

#define WAIT 1                       // Ожидание ввода цифры
#define BELL 2                       // Состояние ввода цифр
#define VOICE 3                      // Проигрывание мелодии
#define DEFAULT_MULTIPLIER 1000
#define DELAY 10

int prevSig = LOW;
int prevBell = HIGH;

int curBell;
int curSig;

int digit = 0;                       // Последняя набранная цифра
int code = 0;                        // Набранный код (4 цифры)
int multiplier = DEFAULT_MULTIPLIER; // Множитель последней набранной цифры
int counter = 0;
int mode = WAIT;

int bellsCount = 6;                  // Количество доступных мелодий
int bells[] = {0, 1, 2, 3, 4, 5};    // Номера доступных мелодий

void setup() {
  pinMode(CLK, OUTPUT);
  pinMode(DAT, OUTPUT);
  
  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  
  pinMode(BELL_PIN, INPUT);
  pinMode(SIG_PIN, INPUT);

  Serial.begin(9600);

  play(0xfff0);                      // Убавляем громкость
}

void loop() {
  curSig = digitalRead(SIG_PIN);
  
  if (curSig == HIGH) {

    lightOff();
    mode = BELL;
    readDigit();
  } else if (prevSig == HIGH) {
    processDigit(digit);
    digit = 0;
  }
  prevSig = curSig;
  
  blink();
  delay(DELAY);
}

void readDigit() {
  curBell = digitalRead(BELL_PIN);
  digitalWrite(GREEN, curBell);
  
  if (curBell == LOW && prevBell == HIGH) {
    digit++;
  }

  prevBell = curBell;
}

void processDigit(int digit) {
  code += (digit % 10) * multiplier;
  multiplier /= 10;
  
  if (multiplier == 0) {
    Serial.println(code);
    
    if (isBellExists(code)) {
      Serial.println(":-)");
      digitalWrite(GREEN, HIGH);
      play(code);
      play(0xfff7);
    } else {
      Serial.println(":-(");
      digitalWrite(RED, HIGH);
    } 

    code = 0;
    multiplier = DEFAULT_MULTIPLIER;
    counter = 0;
    mode = VOICE;
  }
}

boolean isBellExists(int code) {
  for (int i = 0; i < bellsCount; i++) {
    if (bells[i] == code) {
      return true;
    }
  }

  return false;
}

void lightOff() {
  digitalWrite(GREEN, LOW);  
  digitalWrite(RED, LOW);  
}

void blink() {
  switch (mode) {
    case WAIT:
      if (counter == 0) {
        digitalWrite(RED, HIGH);  
      } else if (counter == 10){
        digitalWrite(RED, LOW);  
      } else if (counter == 300) {
        counter = -1;
      }
      break;
    case BELL:
      if (counter == 1500) {
        multiplier = DEFAULT_MULTIPLIER;
        code = 0;
        counter = -1;
        mode = WAIT;
      }
      break;
    case VOICE:
      if (counter == 2000) {
        lightOff();
        counter = -1;
        mode = WAIT;
        play(0xfff0);
      }
      break;
  }
  counter++;
}

/****************************************************
The following function is used to send command to wave shield.
You don't have to change it.
Send the file name to play the audio.
If you need to play file 0005.AD4, write code: send(0x0005).
For more command code, please refer to the manual
*****************************************************/
void play(int data)
{
  digitalWrite(CLK, LOW);
  delay(2);
 
  for (int i=15; i>=0; i--) {
    delayMicroseconds(50);
    if ((data>>i)&0x0001 >0) {
      digitalWrite(DAT, HIGH);
      //Serial.print(1);
    } else {
      digitalWrite(DAT, LOW);
      // Serial.print(0);
    }
 
    delayMicroseconds(50);
    digitalWrite(CLK, HIGH);
    delayMicroseconds(50);
  
    if (i>0) {
      digitalWrite(DAT, LOW);
    } else {
      digitalWrite(DAT, HIGH);
    }
    delayMicroseconds(50);
  
    if (i>0) {
      digitalWrite(CLK, LOW);
    } else {
      digitalWrite(CLK, HIGH);
    }
  }

  delay(20);
}
