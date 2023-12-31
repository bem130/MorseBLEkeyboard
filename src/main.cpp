#include "keysetting.h"


void setup() {
    // put your setup code here, to run once:
    Serial.begin(115200);
    blekb.begin();
    pinMode(key1,INPUT_PULLUP);
    pinMode(key2,INPUT_PULLUP);
    key1_pressed = false;
    key2_pressed = false;
    morse_space_flag = false;
    auto_space = false;
    clearbuffer1();
    clearbuffer2();
}

void loop() {
    if(!blekb.isConnected()) {    //接続されているか
        delay(1000);
        return;
    }

    // Serial.println( String(digitalRead(key1)) + " " + String(digitalRead(key2)) );
    delay(1);

    /* key1 */
    if (key1_pressed) {
        if (digitalRead(key1)!=PRESS) { // key1を離した時
            key1_pressed = false;
            int press_time = millis() - key1_press_t; // 押した時間
            if (press_time>threshold1) {
                inputbuffer1[buffer1index] = 2;
            }
            else {
                inputbuffer1[buffer1index] = 1;
            }
            buffer1index++;
            key1_press_t = millis();
        }
    }
    else {
        { // key1が押されていない時
            int release_time = millis() - key1_press_t; // 離した時間
            if ((release_time>threshold2||buffer1index>=8)&&buffer1index>=1) { // 入力を確定する条件 1.離す時間が長い 2.入力が8を超えた (3.入力がされている)
                int res = searchKeyCode_forMorse();
                if (res!=-1) {
                    if (65<=res&&res<=90) {
                        res+=0x20;
                    }
                    blekb.write(res); // キーを送信
                    morse_space_flag = true;
                }
                clearbuffer1();
                key1_press_t = millis();
            }
            if (auto_space) {
                release_time = millis() - key1_press_t; // 離した時間
                if (morse_space_flag&&release_time>=threshold3-threshold2) {
                    blekb.write(0x20); // スペースキーを送信
                    morse_space_flag = false;
                }
            }
        }
        if (digitalRead(key1)==PRESS) { // key1を押した時
            key1_pressed = true;
            key1_press_t = millis();
            morse_space_flag = false;
        }
    }

    /* key2 */
    if (key2_pressed) {
        if (digitalRead(key2)!=PRESS) { // key2を離した時
            key2_pressed = false;
            int press_time = millis() - key2_press_t; // 押した時間
            if (press_time>threshold1) {
                inputbuffer2[buffer2index] = 2;
                Serial.print("-");
            }
            else {
                inputbuffer2[buffer2index] = 1;
                Serial.print(".");
            }
            buffer2index++;
            key2_press_t = millis();
            // 一個目が短点だったら、続きを見ず終了
            if (buffer2index==1&inputbuffer2[0]==1) {
                blekb.write(KEY_BACKSPACE); // バックスペースキーを送信
                clearbuffer2();
                Serial.println(" BS");
            }
        }
    }
    else {
        { // key2が押されていない時
            int release_time = millis() - key2_press_t; // 離した時間
            if ((release_time>threshold2||buffer2index>=5)&&buffer2index>=1) { // 入力を確定する条件 1.離す時間が長い 2.入力が8を超えた (3.入力がされている)
                int res = searchKeyCode_forModifier();
                if (res!=-1) {
                    uint8_t key = modifierinput[res][0];
                    int keytype = modifierinput[res][1];
                    switch (keytype)
                    {
                        case 0:
                            blekb.write(key); // キーを送信
                        break;
                        case 1:
                            blekb.press(key); // キーを送信
                        break;
                        case 2:
                            blekb.releaseAll();
                        break;
                        case 3:
                            auto_space = key==UNABLE;
                        break;
                    }
                    Serial.print(" ");
                    Serial.print(keytype);
                    Serial.print(" ");
                    Serial.println(key);
                }
                else {
                    Serial.print(" ");
                    Serial.print(-1);
                }
                clearbuffer2();
                key2_press_t = millis();
            }
        }
        if (digitalRead(key2)==PRESS) { // key2を押した時
            morse_space_flag = false; // スペースを取り消す
            key2_pressed = true;
            key2_press_t = millis();
        }
    }

}

void clearbuffer1() {
    buffer1index = 0;
    for (int i=0;i<8;i++) {
        inputbuffer1[i] = 0;
    }
}
void clearbuffer2() {
    buffer2index = 0;
    for (int i=0;i<5;i++) {
        inputbuffer2[i] = 0;
    }
}

int searchKeyCode_forMorse() {
    for (int i=0;i<morseinput_len;i++) {
        if (morseinput[i][1]==buffer1index) {
            bool flag = true;
            for (int j=0;j<buffer1index;j++) {
                if (morseinput[i][j+2]!=inputbuffer1[j]) {
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
int searchKeyCode_forModifier() {
    for (int i=0;i<modifierinput_len;i++) {
        if (modifierinput[i][2]==buffer2index) {
            bool flag = true;
            for (int j=0;j<buffer2index;j++) {
                if (modifierinput[i][j+3]!=inputbuffer2[j]) {
                    flag = false;
                    break;
                }
            }
            if (flag) {
                return i;
            }
        }
    }
    return -1;
}