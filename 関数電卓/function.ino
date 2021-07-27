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

int mode = -1;
int degree = false;

void dispFunction(int pos, String str) {
  int x = pos * 30 + 23;
  u8g.drawRBox(x-15, 19, 29, 12, 2);
  u8g.setColorIndex(0);
  u8g.setFont(u8g_font_6x10r);
  x -= str.length() * 3;
  u8g.setPrintPos(x, 28);
  u8g.print(str);  
  u8g.setColorIndex(1);
  u8g.setFont(u8g_font_10x20r);    
}

void update_display(String x_disp, String y_disp, boolean is_two_line) {
  u8g.firstPage();  
  do {
    if(mode < 0) {
      u8g.setPrintPos(0, 15);
      u8g.print(y_disp);

      u8g.setPrintPos(0, 31);
      if (x_disp == "") {
          x_disp = ">";
      }
      u8g.print(x_disp);
    }
    else {
      switch(mode) {
      case 0:
        dispFunction(0, "x<>y");
        dispFunction(1, "drop");
        dispFunction(2, "reset");
        break;
      case 1:
        dispFunction(0, "sin");
        dispFunction(1, "cos");
        dispFunction(2, "tan");
        break;
      case 2:
        dispFunction(0, "asin");
        dispFunction(1, "acos");
        dispFunction(2, "atan");
        break;
      case 3:
        if(degree) {
          dispFunction(0, "rad");
        }
        else {
          dispFunction(0, "deg");
        }
        dispFunction(1, "pi");
        dispFunction(2, "e");
        break;
      case 4:
        dispFunction(0, "log");
        dispFunction(1, "ln");
        dispFunction(2, "log_y");
        break;
      case 5:
        dispFunction(0, "10^x");
        dispFunction(1, "e^x");
        dispFunction(2, "y^x");
        break;
      case 6:
        dispFunction(0, "-x");
        dispFunction(1, "1/x");
        dispFunction(2, "sqrt");
        break;
      }
      dispFunction(3, "next");
      u8g.setFont(u8g_font_6x10r);
      u8g.setPrintPos(0, 28);
      u8g.print(String(mode));
      u8g.setFont(u8g_font_10x20r);    


      u8g.setPrintPos(0, 15);
      if (x_disp == "") {
          x_disp = ">";
      }
      u8g.print(x_disp);
    }
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
    CLKPR = 0x0;    // 8MHz

    // col
    DDRD  |= _BV(pin_col[0]) | _BV(pin_col[1]) | _BV(pin_col[2]) | _BV(pin_col[3]);     // OUTPUT
    PORTD |= _BV(pin_col[0]) | _BV(pin_col[1]) | _BV(pin_col[2]) | _BV(pin_col[3]);     // HIGH

    // row
    DDRC  &= ~(_BV(pin_row[0]) | _BV(pin_row[1]) | _BV(pin_row[2]) | _BV(pin_row[3]));  // INPUT
    PORTC |=   _BV(pin_row[0]) | _BV(pin_row[1]) | _BV(pin_row[2]) | _BV(pin_row[3]);   // PULLUP

    init_display();

    x = y = z = t = 0;
}

float64_t toRad(float64_t x) {
  if(degree) {
    return fp64_div(fp64_mul(x, float64_NUMBER_PI), fp64_atof("180"));
  }
  else return x;
}

float64_t fromRad(float64_t x) {
  if(degree) {
    return fp64_div(fp64_mul(x, fp64_atof("180")), float64_NUMBER_PI);
  }
  else return x;
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
            if(mode >= 0) {
              mode = (mode + 1) % 7;
            }
            // swap x and y
            else if (long_push) {
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
            float64_t tmp;
            if(long_push || mode >= 0) {
              int select = key - '0';
              if(select == 7) {
                switch(mode) {
                case 0: // x<>y (swap)
                  tmp = x;
                  x = y;
                  y = tmp;
                  y_disp = fp64_to_string_wrap(y);
                  break;
                case 1:
                  x = fp64_sin(toRad(x));
                  break;
                case 2:
                  x = fromRad(fp64_asin(x));
                  break;
                case 3: // switch degree / radian
                  degree = !degree;
                  break;
                case 4:
                  x = fp64_log10(x);
                  break;
                case 5:
                  x = fp64_exp10(x);
                  break;
                case 6: // +/- (CHS)
                  x = fp64_neg(x);
                  break;
                }
                x_disp = fp64_to_string_wrap(x);
                mode = -1;
                prev_pushed_key_type = 1;
              }
              else if(select == 8) {
                switch(mode) {
                case 0: // drop
                  x = y;
                  y = z;
                  z = t;
                  y_disp = fp64_to_string_wrap(y);
                  break;
                case 1:
                  x = fp64_cos(toRad(x));
                  break;
                case 2:
                  x = fromRad(fp64_acos(x));
                  break;
                case 3: // pi
                  x = float64_NUMBER_PI;
                  break;
                case 4:
                  x = fp64_log(x);
                  break;
                case 5:
                  x = fp64_exp(x);
                  break;
                case 6: // 1/x
                  x = fp64_inverse(x);
                  break;
                }
                x_disp = fp64_to_string_wrap(x);
                mode = -1;
                prev_pushed_key_type = 1;
              }
              else if(select == 9) {
                switch(mode) {
                case 0: // reset
                  x = y = z = t = 0;
                  x_disp = "";
                  y_disp = "";
                  prev_loop_key = 0;
                  prev_pushed_key_type = 0;
                  mode = 1;
                  degree = false;
                  break;
                case 1:
                  x = fp64_tan(toRad(x));
                  break;
                case 2:
                  x = fromRad(fp64_atan(x));
                  break;
                case 3: // e
                  x = float64_EULER_E;
                  break;
                case 4:
                  tmp = x;
                  pop();
                  x = fp64_div(fp64_log(tmp), fp64_log(x));
                  break;
                case 5:
                  tmp = x;
                  pop();
                  x = fp64_pow(x, tmp);
                  break;
                case 6:
                  x = fp64_sqrt(x);
                  break;
                }
                x_disp = fp64_to_string_wrap(x);
                mode = -1;
                prev_pushed_key_type = 1;
              }
              else if(select == mode) { //cancel function mode
                mode = -1;
              }
              else if(key == '.') { // drop
                x = y;
                y = z;
                z = t;
                y_disp = fp64_to_string_wrap(y);
                x_disp = fp64_to_string_wrap(x);
                mode = -1;
                prev_pushed_key_type = 1;
              }
              else { // select function table
                mode = select;
              }
            }
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
        }

        // display
        y_disp = fp64_to_string_wrap(y);
        update_display(x_disp, y_disp, true);
    }
    last_pushed_key_type = prev_pushed_key_type;
    prev_loop_key = key;
    delay(1);
}
