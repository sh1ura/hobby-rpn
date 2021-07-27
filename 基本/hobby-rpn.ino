#include <Wire.h>
#include "U8glib.h"
#include <fp64lib.h>

U8GLIB_SSD1306_128X32 u8g(U8G_I2C_OPT_NONE);  // I2C / TWI 

#define MAX_DIGIT     12

byte pin_col[] = {0, 1, 2, 3};   // PD
byte pin_row[] = {3, 2, 1, 0};   // PC

float64_t x, y, z, t;
byte last_pushed_key_type = 0;    // 0:numeral 1:operator 2:enter

void push() {
    t = z;
    z = y;
    y = x;
}

void pop() {
    x = y;
    y = z;
    z = t;
}

char key_scan() {
    PORTD |= _BV(pin_col[0]) | _BV(pin_col[1]) | _BV(pin_col[2]) | _BV(pin_col[3]); // set high

    for (byte i = 0; i < 4; i++) {
        PORTD &= ~_BV(pin_col[i]);  // set low

        for (byte j = 0; j < 4; j++) {
            if ((PINC & _BV(pin_row[j])) == LOW) {
                switch (4 * j + i) {
                    case  0: return '7';
                    case  1: return '8';
                    case  2: return '9';
                    case  3: return '/';

                    case  4: return '4';
                    case  5: return '5';
                    case  6: return '6';
                    case  7: return '*';

                    case  8: return '1';
                    case  9: return '2';
                    case 10: return '3';
                    case 11: return '-';

                    case 12: return '0';
                    case 13: return '.';
                    case 14: return '=';
                    case 15: return '+';
                }
            }
        }
        PORTD |= _BV(pin_col[i]);   // set high
    }
    return 0;
}

void blink_display() {
  u8g.firstPage();  
  do {
  } while(u8g.nextPage());
  delay(10);
}

void init_display() {
    u8g.setColorIndex(1);
    u8g.setFont(u8g_font_10x20r);
    update_display("", fp64_to_string_wrap(0), true);
}

void update_display(String x_disp, String y_disp, boolean is_two_line) {
  u8g.firstPage();  
  do {
    u8g.setPrintPos(0, 15);
    u8g.print(y_disp);

    u8g.setPrintPos(0, 31);
    if (x_disp == "") {
        x_disp = ">";
    }
    u8g.print(x_disp);
  } while(u8g.nextPage());
}
    

String fp64_to_string_wrap(float64_t n) {
  String s;
    if (fp64_signbit(n)) {  // minus
        s = fp64_to_string(n, MAX_DIGIT, MAX_DIGIT - 3);
    }
    else {
        s = fp64_to_string(n, MAX_DIGIT, MAX_DIGIT - 2);
    }
    while(s.length() < 12) {
        s = " " + s; // align right
    }
    return s;
}


void setup() {
    CLKPR = 0x80; 
    CLKPR = 0x1;    // 8MHz

    // col
    DDRD  |= _BV(pin_col[0]) | _BV(pin_col[1]) | _BV(pin_col[2]) | _BV(pin_col[3]);     // OUTPUT
    PORTD |= _BV(pin_col[0]) | _BV(pin_col[1]) | _BV(pin_col[2]) | _BV(pin_col[3]);     // HIGH

    // row
    DDRC  &= ~(_BV(pin_row[0]) | _BV(pin_row[1]) | _BV(pin_row[2]) | _BV(pin_row[3]));  // INPUT
    PORTC |=   _BV(pin_row[0]) | _BV(pin_row[1]) | _BV(pin_row[2]) | _BV(pin_row[3]);   // PULLUP

    init_display();

    x = y = z = t = 0;
}

void loop() {
    static String x_disp = "";
    static String y_disp = "";
    static char prev_loop_key = 0;
    static byte prev_pushed_key_type = 0;    // 0:numeral 1:operator 2:enter

    // key scan
    char key = key_scan();
    if (key != 0 && key != prev_loop_key) {

        // detect long push
        boolean long_push = false;
        unsigned long pushed_time = millis();
        while (key_scan() == key) {
            if (millis() - pushed_time > 1000 / 2) {    // 1sec
                long_push = true;
                break;
            }
        }

        if (key == '+') {
            // no long press
            float64_t acc1 = x;
            pop();
            float64_t acc2 = x;

            x = fp64_add(acc2, acc1);

            x_disp = fp64_to_string_wrap(x);
            prev_pushed_key_type = 1;
            blink_display();
        }
        // sqrt or multiply
        else if (key == '*') {
            // square root
            if (long_push) {
                x = fp64_sqrt(x);
                prev_pushed_key_type = 1;
            }
            // - substruct
            else {
                float64_t acc1 = x;
                pop();
                float64_t acc2 = x;

                x = fp64_mul(acc2, acc1);
                prev_pushed_key_type = 1;
            }
            x_disp = fp64_to_string_wrap(x);
        }
        // chsgn or substruct
        else if (key == '-') {
            // -/+ change sign
            if (long_push) {
                x = fp64_neg(x);
//                prev_pushed_key_type = last_pushed_key_type;
                prev_pushed_key_type = 1;
            }
            // - substruct
            else {
                float64_t acc1 = x;
                pop();
                float64_t acc2 = x;

                x = fp64_sub(acc2, acc1);
                prev_pushed_key_type = 1;
            }
            x_disp = fp64_to_string_wrap(x);
        }
        // '/' or x<>y
        else if (key == '/') {
            // swap x and y
            if (long_push) {
                float64_t tmp = x;
                x = y;
                y = tmp;
            }
            // '/' key
            else {
                float64_t acc1 = x;
                pop();
                float64_t acc2 = x;
    
                x = fp64_div(acc2, acc1);
            }
            x_disp = fp64_to_string_wrap(x);            
            prev_pushed_key_type = 1;            
            blink_display();
        }
        // clear or enter
        else if (key == '=') {
            // clear
            if (long_push) {
                x = 0;
                x_disp = "";
                prev_pushed_key_type = 0;
            }
            // enter
            else {
                push();
                
                x_disp = fp64_to_string_wrap(x);
                prev_pushed_key_type = 2;
            }
            blink_display();
        }
        // numeral or .
        else {
            if (prev_pushed_key_type == 1) push();          // operator
            if (prev_pushed_key_type > 0) x_disp = "";      // operator or enter
            
            if (x_disp.length() < MAX_DIGIT) {
                if (x_disp.indexOf(".") == -1 || key != '.') {
                    x_disp.concat(key);
                    x = fp64_atof((char*)x_disp.c_str());
                }
            }
            prev_pushed_key_type = 0;
        }

        // display
        y_disp = fp64_to_string_wrap(y);
        update_display(x_disp, y_disp, true);
    }
    last_pushed_key_type = prev_pushed_key_type;
    prev_loop_key = key;
    delay(1);
}
