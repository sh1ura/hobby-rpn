#include <Wire.h>
#include "U8glib.h"
#include <fp64lib.h>

#define MAX_DIGIT     12

#define STACK_DEPTH   8
float64_t stack[STACK_DEPTH];

void push(float64_t v) {
  for(int i = STACK_DEPTH - 1; i > 0; i--) {
    stack[i] = stack[i - 1];
  }
  stack[0] = v;
}

float64_t pop() {
  float64_t v;
  v = stack[0];
  for(int i = 0; i < STACK_DEPTH - 1; i++) {
    stack[i] = stack[i + 1];
  }
  stack[STACK_DEPTH - 1] = 0;
  return v;
}

#define UNDO_DEPTH 12
int undoTop = 0;
float64_t undoBuf[UNDO_DEPTH][STACK_DEPTH];

void backup(void) {
  if(undoTop >= UNDO_DEPTH) {
    for(int i = 0; i < UNDO_DEPTH - 1; i++) {
      for(int j = 0; j < STACK_DEPTH; j++) {
        undoBuf[i][j] = undoBuf[i + 1][j];
      }
    }
    undoTop --;
  }
  for(int j = 0; j < STACK_DEPTH; j++) {
    undoBuf[undoTop][j] = stack[j];
  }
  undoTop++;
}

void restore(void) {
  if(undoTop == 0) // no undo data
    return;
  undoTop--;
  for(int j = 0; j < STACK_DEPTH; j++) {
    stack[j] = undoBuf[undoTop][j];
  }
}

byte pin_col[] = {0, 1, 2, 3};   // PD
byte pin_row[] = {3, 2, 1, 0};   // PC

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

void setup() {
    CLKPR = 0x80; 
    CLKPR = 0x0;    // 8MHz

    // col
    DDRD  |= _BV(pin_col[0]) | _BV(pin_col[1]) | _BV(pin_col[2]) | _BV(pin_col[3]);     // OUTPUT
    PORTD |= _BV(pin_col[0]) | _BV(pin_col[1]) | _BV(pin_col[2]) | _BV(pin_col[3]);     // HIGH

    // row
    DDRC  &= ~(_BV(pin_row[0]) | _BV(pin_row[1]) | _BV(pin_row[2]) | _BV(pin_row[3]));  // INPUT
    PORTC |=   _BV(pin_row[0]) | _BV(pin_row[1]) | _BV(pin_row[2]) | _BV(pin_row[3]);   // PULLUP

    messageBox("RPN calc");

    init_display();
}

U8GLIB_SSD1306_128X32 u8g(U8G_I2C_OPT_NONE);  // I2C / TWI 

void messageBox(String str) {
  u8g.firstPage();  
  do {
    u8g.setColorIndex(0);
    u8g.drawBox(4, 4, 120, 24);
    u8g.setColorIndex(1);
    u8g.drawFrame(4, 4, 120, 24);
    int x = 64 - str.length() * 5;
    u8g.setFont(u8g_font_10x20r);
    u8g.setPrintPos(x, 22);
    u8g.print(str);  
  } while(u8g.nextPage());
  delay(1000);
}

void init_display() {
    u8g.setColorIndex(1);
    u8g.setFont(u8g_font_10x20r);
    update_display();
}

int mode = -1; // function key mode : -1 : disabled, 0-6 : function key enabled
boolean degree = false; // radian / degree
String enteringStr = "";

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

void update_display(void) {
  String x_disp = "", y_disp = "";

  if(enteringStr.length()) {
    y_disp = fp64_to_string_wrap(stack[0]);
    if(enteringStr.length() > MAX_DIGIT) { // 文字列の前方を省略表示
      x_disp = ".." + enteringStr.substring(enteringStr.length() - MAX_DIGIT + 2);
    }
    else {
      x_disp = enteringStr;
    }
  }
  else {
    y_disp = fp64_to_string_wrap(stack[1]);
    x_disp = fp64_to_string_wrap(stack[0]);
  }

  u8g.firstPage();  
  do {
    if(mode < 0) { // no function button
      u8g.setPrintPos(0, 15);
      u8g.print(y_disp);
      u8g.setPrintPos(0, 31);
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

void enter(void) {
  if(enteringStr.length()) {
    push(fp64_atof((char*)enteringStr.c_str()));
    enteringStr = "";
  }
}

void loop() {
  float64_t x, y;

  char key = key_scan();
  if (key == 0) {
    return;
  }

  // detect long push
  boolean long_push = false;
  unsigned long pushed_time = millis();
  while (key_scan() == key) {
    if (millis() - pushed_time > 1000 / 2) {
      long_push = true;
      break;
    }
  }

  switch(key) {
  case '+':
    if (long_push) { // undo
      enteringStr = "";
      restore();
    }
    else { // + addition
      backup();
      enter();
      x = pop();
      y = pop();
      push(fp64_add(y, x));
    }
    break;
  case '*':
    backup();
    enter();
    if (long_push) { // square root
      push(fp64_sqrt(pop()));
    }
    else { // - substruct
      x = pop();
      y = pop();
      push(fp64_mul(y, x));
    }
    break;
  case '-':
    backup();
    enter();
    if (long_push) { // +/- CHS
      push(fp64_neg(pop()));
    }
    else {
      x = pop();
      y = pop();
      push(fp64_sub(y, x));
    }
    break;
  case '/':
    if(mode >= 0) { // function key mode
      mode = (mode + 1) % 7; // next page
    }
    else if (long_push) {            // SWAP x and y  
      backup();
      enter();
      x = pop();
      y = pop();
      push(x);
      push(y);
    }
     else {
      backup();
      enter();
      x = pop();
      y = pop();
      push(fp64_div(y, x));
    }
    break;
  case '=':
    backup();
    if(enteringStr.length()) { // enter
      enter();
    }
    else if (long_push) { // DROP
      pop();
    }
    else { // DUP
      x = pop();
      push(x);
      push(x);    
    }
    break;
  default:
    if(mode >= 0) { // function key mode
      int select = key - '0';
      if(select == 7) {
        enter();
        switch(mode) {
        case 0: // x<>y (swap)
          backup();
          x = pop();
          y = pop();
          push(x);
          push(y);
          break;
        case 1:
          backup();
          push(fp64_sin(toRad(pop())));
          break;
        case 2:
          backup();
          push(fromRad(fp64_asin(pop())));
          break;
        case 3: // switch degree / radian
          degree = !degree;
          if(degree) {
            messageBox("degree mode");
          }
          else {
            messageBox("radian mode");
          }
          break;
        case 4:
          backup();
          push(fp64_log10(pop()));
          break;
        case 5:
          backup();
          push(fp64_exp10(pop()));
          break;
        case 6: // +/- (CHS)
          backup();
          push(fp64_neg(pop()));
          break;
        }
        mode = -1;
      }
      else if(select == 8) {
        enter();
        switch(mode) {
        case 0: // drop
          backup();
          pop();
          break;
        case 1:
          backup();
          push(fp64_cos(toRad(pop())));
          break;
        case 2:
          backup();
          push(fromRad(fp64_acos(pop())));
          break;
        case 3: // pi
          backup();
          push(float64_NUMBER_PI);
          break;
        case 4:
          backup();
          push(fp64_log(pop()));
          break;
        case 5:
          backup();
          push(fp64_exp(pop()));
          break;
        case 6: // 1/x
          backup();
          push(fp64_inverse(pop()));
          break;
        }
        mode = -1;
      }
      else if(select == 9) {
        enter();
        switch(mode) {
        case 0: // reset
          messageBox("reset");
          asm volatile ("  jmp 0");  
          break;
        case 1:
          backup();
          push(fp64_tan(toRad(pop())));
          break;
        case 2:
          backup();
          push(fromRad(fp64_atan(pop())));
          break;
        case 3: // e
          backup();
          push(float64_EULER_E);
          break;
        case 4:
          backup();
          x = pop();
          y = pop();
          push(fp64_div(fp64_log(x), fp64_log(y)));
          break;
        case 5:
          backup();
          x = pop();
          y = pop();
          push(fp64_pow(y, x));
          break;
        case 6:
          backup();
          push(fp64_sqrt(pop()));
          break;
        }
        mode = -1;
      }
      else if(select == mode) { //cancel function mode
        mode = -1;
      }
      else if(0 <= select && select <= 6) {
        mode = select;
      }
      else { // '.'
        mode = -1;
      }
    }
    else if(long_push) {
      if('0' <= key && key <= '6') {
        mode = key - '0';
      }
    }
    else { // entering number
      if (enteringStr.indexOf(".") == -1 || key != '.') {
        if(enteringStr.length() || key != '0') {
          enteringStr.concat(key);
        }
      }
    }
  }
  update_display();
  while(key_scan())
    ;
  delay(10); // avid chattering
}
