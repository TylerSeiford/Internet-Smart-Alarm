#include <Grove_LCD_RGB_Backlight.h>
#include <adafruit-led-backpack.h>

#define DELAY 10
#define ENABLEMOD true

rgb_lcd lcd;

Adafruit_8x8matrix m[3];
bool alarm = false, last = false;
int result = 0;

const int buzzerPin = D2, testerPin = D3;

//Clears and sets up matrix
void setupMatrix(void *matrix) {
    Adafruit_8x8matrix *m = (Adafruit_8x8matrix*) matrix;
    m->clear();
    m->writeDisplay();
    m->setTextSize(1);
    m->setTextWrap(false);
    m->setTextColor(LED_ON);
    m->setRotation(45);
    m->setCursor(0, 0);
}

//Displays num to matrix
void dispNum(void *matrix, int num) {
    Adafruit_8x8matrix *m = (Adafruit_8x8matrix*) matrix;

    m->clear();
    m->setCursor(1, 0);
    m->write(num + '0');
    m->writeDisplay();
}

//Generates random numbers, displays them and returns the result
int wakeUp(void *mat1, void *mat2) {
    int one = random(2,10), two = random(2,10);
    int operation, result;

    if(ENABLEMOD)
        operation = random(0,4);
    else
        operation = random(0,3);
    
    if(two > one)
    {
        int temp = one;
        one = two;
        two = temp;
    }

    dispNum(&m[0], one);
    dispNum(&m[2], two);

    if(operation == 0)
    {
        result = one-two;
        dispNum(&m[1], -3); //Display '-'
    }
    else if(operation == 1)
    {
        result = one+two;
        dispNum(&m[1], -5); //Display '+'
    }
    else if(operation == 2)
    {
        result = one * two;
        dispNum(&m[1], -6); //Display '*'
    }
    else if(operation == 3)
    {
        result = one % two;
        dispNum(&m[1], -11); //Display '%'
    }

    return result;
}

//Reads button, runs wakeUp if true
bool buttonState = false;
int tester() {
    if( digitalRead(testerPin) )
    {
        if(!buttonState)
        {
            buttonState = true;

            return wakeUp(&m[0], &m[1]);
        }
    }
    else
    {
        if(buttonState)
        {
            buttonState = false;
        }
    }
    return result;
}

//Cloud function to start alarm
int testAlarm(String ansStr) {
    alarm = true;
    return 1;
}

//Cloud function to recieve answer, disables alarm if so
int handleAnswer(String ansStr)
{
    int answer = ansStr.toInt();
    if(answer == result)
    {
        alarm = false;
        return 1;
    }
    else
        return 0;
}

//Initialize song variables
void initSong() {
    
}

//Play song, called from a loop with DELAY
void songPeriodic() {
    tone(buzzerPin, 261, 0);
}


void random_seed_from_cloud(unsigned seed) {
  srand(seed);
}

void setup() {
    m[0].begin(0x70);
    m[1].begin(0x71);
    m[2].begin(0x74);
    setupMatrix(&m[0]);
    setupMatrix(&m[1]);
    setupMatrix(&m[2]);
    
    lcd.begin(16, 2);
    lcd.setRGB(64, 64, 64);
    lcd.setCursor(2,0);
    lcd.print("Current Time");

    Time.zone(-5);
    
    pinMode(testerPin, INPUT_PULLDOWN);
    
    Particle.function("setHour", setHour);
    Particle.function("setMinute", setMinute);
    Particle.function("answer", handleAnswer);
    Particle.function("startAlarm", testAlarm);
}

void loop() {
    result = tester();
    if(alarm)
    {
        if(!last)
        {
            result = wakeUp(&m[0], &m[1]);
            last = true;
            initSong();
        }
        else
        {
            songPeriodic();
        }
    }
    else
    {
        if(last)
        {
            last = false;
            m[0].clear();
            m[0].writeDisplay();
            m[1].clear();
            m[1].writeDisplay();
            m[2].clear();
            m[2].writeDisplay();
        }
        noTone(buzzerPin);
    }
    
    //Update LCD time display
    lcd.setCursor(6, 1);
    lcd.print(Time.hourFormat12());
    lcd.print(":");
    if(Time.minute() < 10)
        lcd.print("0");
    lcd.print(Time.minute());
    
    delay(10);
}