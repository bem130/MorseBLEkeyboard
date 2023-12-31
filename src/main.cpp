#include <Arduino.h>
#include <BleKeyboard.h>

#define key1 18
#define key2 19

#define PRESS 0

/* one btn mode */
    /* 押す時間 */
    // short
    #define threshold1 200
    // long

    /* 離す時間 */
    // continue
    #define threshold2 300
    // confirm
    #define threshold3 800
    // confirm&space


void clearbuffer();
int searchKeyCode_forMorse();

static BleKeyboard blekb("Morse Keyboard");


bool key1_pressed;
bool key2_pressed;
ulong key1_press_t;
ulong key2_press_t;

int inputbuffer[8]; // 入力
int bufferindex;

bool morse_space_flag;

#define morseinput_len 27
int morseinput[morseinput_len][10] = {
    {65, 2, 1,2,0,0,0,0,0,0}, // A
    {66, 4, 2,1,1,1,0,0,0,0}, // B
    {67, 4, 2,1,2,1,0,0,0,0}, // C
    {68, 3, 2,1,1,0,0,0,0,0}, // D
    {69, 1, 1,0,0,0,0,0,0,0}, // E
    {70, 4, 1,1,2,1,0,0,0,0}, // F
    {71, 3, 2,2,1,0,0,0,0,0}, // G
    {72, 4, 1,1,1,1,0,0,0,0}, // H
    {73, 2, 1,1,0,0,0,0,0,0}, // I
    {74, 4, 1,2,2,2,0,0,0,0}, // J
    {75, 3, 2,1,2,0,0,0,0,0}, // K
    {76, 4, 1,2,1,1,0,0,0,0}, // L
    {77, 2, 2,2,0,0,0,0,0,0}, // M
    {78, 2, 2,1,0,0,0,0,0,0}, // N
    {79, 3, 2,2,2,0,0,0,0,0}, // O
    {80, 4, 1,2,2,1,0,0,0,0}, // P
    {81, 4, 2,2,1,2,0,0,0,0}, // Q
    {82, 3, 1,2,1,0,0,0,0,0}, // R
    {83, 3, 1,1,1,0,0,0,0,0}, // S
    {84, 1, 2,0,0,0,0,0,0,0}, // T
    {85, 3, 1,1,2,0,0,0,0,0}, // U
    {86, 4, 1,1,1,2,0,0,0,0}, // V
    {87, 3, 1,2,2,0,0,0,0,0}, // W
    {88, 4, 2,1,1,2,0,0,0,0}, // X
    {89, 4, 2,1,2,2,0,0,0,0}, // Y
    {90, 4, 2,2,1,1,0,0,0,0}, // Z
    {0x8, 8, 1,1,1,1,1,1,1,1}, // Backspace
};


void setup() {
    // put your setup code here, to run once:
    blekb.begin();
    Serial.begin(115200);
    pinMode(key1,INPUT_PULLUP);
    pinMode(key2,INPUT_PULLUP);
    key1_pressed = false;
    key2_pressed = false;
    morse_space_flag = false;
    clearbuffer();
}

void loop() {
    if(!blekb.isConnected()) {    //接続されているか
        delay(1000);
        return;
    }

    // Serial.println( String(digitalRead(key1)) + " " + String(digitalRead(key2)) );
    delay(1);

    if (key1_pressed) {
        if (digitalRead(key1)!=PRESS) {
            key1_pressed = false;
            int press_time = millis() - key1_press_t; // 押した時間
            if (press_time>threshold1) {
                inputbuffer[bufferindex] = 2;
                Serial.print("-");
            }
            else {
                inputbuffer[bufferindex] = 1;
                Serial.print(".");
            }
            bufferindex++;
            //Serial.print("    ");
            //Serial.println(press_time);
            key1_press_t = millis();
        }
    }
    else {
        {
            int release_time = millis() - key1_press_t; // 離した時間
            if ((release_time>threshold2||bufferindex>=8)&&bufferindex>=1) { // 入力を確定する条件 1.離す時間が長い 2.入力が8を超えた (3.入力がされている)
                Serial.print("\n     [");
                int res = searchKeyCode_forMorse();
                if (res==-1) {
                    Serial.print("undefined");
                }
                else {
                    if (65<=res&&res<=90) {
                        Serial.print((char)res);
                    }
                    else {
                        Serial.print(res);
                    }
                    blekb.write(res); // キーを送信
                    morse_space_flag = true;
                }
                Serial.println("]");
                clearbuffer();
                key1_press_t = millis();
            }
            release_time = millis() - key1_press_t; // 離した時間
            if (morse_space_flag&&release_time>=threshold3-threshold2) {
                Serial.println("       <space>");
                blekb.write(0x20); // スペースキーを送信
                morse_space_flag = false;
            }
        }
        if (digitalRead(key1)==PRESS) {
            key1_pressed = true;
            //Serial.print("    ");
            //Serial.println(release_time);
            key1_press_t = millis();
            morse_space_flag = false;
        }
    }
    if (key2_pressed) {
        if (digitalRead(key2)!=PRESS) { // key2を離した時
            key2_pressed = false;
        }
    }
    else {
        if (digitalRead(key2)==PRESS) { // key2を押した時
            key2_pressed = true;
            blekb.write(0x8); // バックスペースキーを送信
            morse_space_flag = false; // スペースキーを取り消し
        }
    }


    // if (digitalRead(key1)==PRESS) {
    //     blekb.write(KEY_NUM_ENTER); // キーを送信
    // }
    // if (digitalRead(key2)==PRESS) {
    //     blekb.write(65); // キーを送信
    // }

}

void clearbuffer() {
    bufferindex = 0;
    for (int i=0;i<8;i++) {
        inputbuffer[i] = 0;
    }
}

int searchKeyCode_forMorse() {
    for (int i=0;i<morseinput_len;i++) {
        if (morseinput[i][1]==bufferindex) {
            bool flag = true;
            for (int j=0;j<bufferindex;j++) {
                if (morseinput[i][j+2]!=inputbuffer[j]) {
                    flag = false;
                    break;
                }
            }
            if (flag) {
                return morseinput[i][0];
            }
        }
    }
    return -1;
}