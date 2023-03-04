// -------------------- БИБЛИОТЕКИ ---------------------
#include <string.h>             // библиотека расширенной работы со строками
#include <LiquidCrystal_I2C.h>  // библтотека дисплея
#include <TimerOne.h>           // библиотека таймера
#include <TimeLib.h>
// -------------------- БИБЛИОТЕКИ ---------------------


#define printByte(args) write(args);
LiquidCrystal_I2C lcd(0x27, 20, 4);
boolean Start_logo, Connection_flag, Start_logo1 = 1, updateDisplay_flag, updateTemp_flag, timeOut_flag = 1, restoreConnectToPC = 0, reDraw_flag = 1, timeOutLCDClear = 1;
boolean display_flag = 1, change_flag = true;
unsigned long timeout, plot_timer;
char inData[82];      // массив входных значений (СИМВОЛЫ)
int PCdata[13];       // массив численных значений показаний с компьютера
byte PLOTmem[6][16];  // массив для хранения данных для построения графика (16 значений для 6 параметров)
byte lines[] = { 4, 5, 7, 6 };
byte plotLines[] = { 0, 1, 4, 5, 6, 7 };  // 0-CPU temp, 1-GPU temp, 2-CPU load, 3-GPU load, 4-RAM load, 5-GPU memory
byte index = 0;
String string_convert;
byte blocks, halfs;
String perc;
uint32_t myTimer1, myTimer2, myTimer3, myTimer_clear;
// блоки для построения графиков
byte row8[8] = { 0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111 };
byte row7[8] = { 0b00000, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111 };
byte row6[8] = { 0b00000, 0b00000, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111 };
byte row5[8] = { 0b00000, 0b00000, 0b00000, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111 };
byte row4[8] = { 0b00000, 0b00000, 0b00000, 0b00000, 0b11111, 0b11111, 0b11111, 0b11111 };
byte row3[8] = { 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b11111, 0b11111, 0b11111 };
byte row2[8] = { 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b11111, 0b11111 };
byte row1[8] = { 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b11111 };
// значок градуса!!!! lcd.write(223);
byte degree[8] = { 0b11100, 0b10100, 0b11100, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000 };
// правый край полосы загрузки
byte right_empty[8] = { 0b11111, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b11111 };
// левый край полосы загрузки
byte left_empty[8] = { 0b11111, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b11111 };
// центр полосы загрузки
byte center_empty[8] = { 0b11111, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b11111 };
// Названия для легенды графиков
const char plot_0[] = "CPU";
const char plot_1[] = "GPU";
const char plot_2[] = "RAM";

const char plot_3[] = "temp";
const char plot_4[] = "load";
const char plot_5[] = "mem";
// названия ниже должны совпадать с массивами сверху и идти по порядку!
static const char *plotNames0[] = {
  plot_0, plot_1, plot_0, plot_1, plot_2, plot_1
};
static const char *plotNames1[] = {
  plot_3, plot_3, plot_4, plot_4, plot_4, plot_5
};
//PCdata[0] = temp CPU; PCdata[1] = CPU usage; PCdata[2] = clock #1; PCdata[3] = clock #2; PCdata[4] = clock #3; PCdata[5] = clock #4
//PCdata[6] = CPU Power; PCdata[7] = GPU temp; ; PCdata[8] = GPU usage; PCdata[9] = Mem GPU Usage; PCdata[10] = SSD temp; PCdata[11] = RAM use


void setup() {
  Serial.begin(9600);
  Start_logo = true;
  lcd.init();
  lcd.backlight();
  initBar3();
  lcd.clear();
  //MGTU_logo();
  //delay(3500);
  //lcd.clear();
  //Developer_logo();
  //delay(3500);
  lcd.clear();
}

void loop() {
  parsing();
  updateDisplay();
}


void MGTU_logo() {
  lcd.setCursor(3, 0);
  lcd.print("Moscow");
  lcd.setCursor(12, 0);
  lcd.print("State");
  lcd.setCursor(5, 1);
  lcd.print("Technical");
  lcd.setCursor(5, 2);
  lcd.print("University");
  lcd.setCursor(3, 3);
  lcd.print("Kaluga");
  lcd.setCursor(11, 3);
  lcd.print("Branch");
}


void Developer_logo() {
  lcd.setCursor(5, 0);
  lcd.print("Project");
  lcd.setCursor(14, 0);
  lcd.print("of");
  lcd.setCursor(3, 1);
  lcd.print("Kirill");
  lcd.setCursor(11, 1);
  lcd.print("Gorlov");
  lcd.setCursor(6, 2);
  lcd.print("HardWare");
  lcd.setCursor(6, 3);
  lcd.print("Monitor");
  lcd.setCursor(14, 3);
  lcd.print(char(252));
}


void parsing() {
  while (Serial.available() > 0) {
    char aChar = Serial.read();
    if (aChar != 'E') {
      inData[index] = aChar;
      index++;
      inData[index] = '\0';
    } else {
      char *p = inData;
      char *str;
      index = 0;
      String value = "";
      while ((str = strtok_r(p, ";", &p)) != NULL) {
        string_convert = str;
        PCdata[index] = string_convert.toInt();
        index++;
      }
      index = 0;
      updateDisplay_flag = 1;
      updateTemp_flag = 1;
    }
    timeout = millis();
    timeOut_flag = 1;
    restoreConnectToPC = 1;
    lcd.backlight();
  }
}



void updateDisplay() {
  display_switch();
  if (display_flag) {
    draw_names1();
    draw_stats1();
    int perc1 = PCdata[1];
    fillBar3(7, 0, 10, perc1);
    int perc2 = PCdata[8];
    fillBar3(7, 1, 10, perc2);
    int perc3 = PCdata[9];
    fillBar3(7, 2, 10, perc3);
    int perc4 = PCdata[11];
    fillBar3(7, 3, 10, perc4);
  } else {
    draw_names_stats2();
  }
}

void draw_names1() {
  lcd.setCursor(0, 0);
  lcd.print("CPU:");
  lcd.setCursor(0, 1);
  lcd.print("GPU:");
  lcd.setCursor(0, 2);
  lcd.print("GPUmem:");
  lcd.setCursor(0, 3);
  lcd.print("RAMuse:");
}


void draw_stats1() {
  lcd.setCursor(4, 0);
  lcd.print(PCdata[0]);
  lcd.write(223);
  lcd.setCursor(17, 0);
  lcd.print(PCdata[1]);
  if (PCdata[1] < 10) perc = "% ";
  else if (PCdata[1] < 100) perc = "%";
  else perc = "";
  lcd.print(perc);
  lcd.setCursor(4, 1);
  lcd.print(PCdata[7]);
  lcd.write(223);
  lcd.setCursor(17, 1);
  lcd.print(PCdata[8]);
  if (PCdata[8] < 10) perc = "% ";
  else if (PCdata[8] < 100) perc = "%";
  else perc = "";
  lcd.print(perc);
  lcd.setCursor(17, 2);
  lcd.print(PCdata[9]);
  if (PCdata[9] < 10) perc = "% ";
  else if (PCdata[9] < 100) perc = "%";
  else perc = "";
  lcd.print(perc);
  lcd.setCursor(17, 3);
  lcd.print(PCdata[11]);
  if (PCdata[11] < 10) perc = "% ";
  else if (PCdata[11] < 100) perc = "%";
  else perc = "";
  lcd.print(perc);
}


void draw_names_stats2() {
  //Частота первого ядра
  lcd.setCursor(0, 0);
  lcd.print("Clock#1:");
  if (PCdata[2] < 1000) {
    lcd.setCursor(9, 0);
    lcd.print(PCdata[2]);
    lcd.setCursor(14, 0);
    lcd.print("MHz");
    lcd.setCursor(12, 0);
    lcd.print(" ");
  } else {
    lcd.setCursor(9, 0);
    lcd.print(PCdata[2]);
    lcd.setCursor(14, 0);
    lcd.print("MHz");
  }
  //Частота первого ядра

  //Частота второго ядра
  lcd.setCursor(0, 1);
  lcd.print("Clock#2:");
  if (PCdata[2] < 1000) {
    lcd.setCursor(9, 1);
    lcd.print(PCdata[3]);
    lcd.setCursor(14, 1);
    lcd.print("MHz");
    lcd.setCursor(12, 1);
    lcd.print(" ");
  } else {
    lcd.setCursor(9, 1);
    lcd.print(PCdata[3]);
    lcd.setCursor(14, 1);
    lcd.print("MHz");
  }
  //Частота второго ядра

  //Частота третьего ядра
  lcd.setCursor(0, 2);
  lcd.print("Clock#3:");
  if (PCdata[2] < 1000) {
    lcd.setCursor(9, 2);
    lcd.print(PCdata[4]);
    lcd.setCursor(14, 2);
    lcd.print("MHz");
    lcd.setCursor(12, 2);
    lcd.print(" ");
  } else {
    lcd.setCursor(9, 2);
    lcd.print(PCdata[4]);
    lcd.setCursor(14, 2);
    lcd.print("MHz");
  }
  //Частота третьего ядра

  //Частота четвертого ядра
  lcd.setCursor(0, 3);
  lcd.print("Clock#4:");
  if (PCdata[2] < 1000) {
    lcd.setCursor(9, 3);
    lcd.print(PCdata[5]);
    lcd.setCursor(14, 3);
    lcd.print("MHz");
    lcd.setCursor(12, 3);
    lcd.print(" ");
  } else {
    lcd.setCursor(9, 3);
    lcd.print(PCdata[5]);
    lcd.setCursor(14, 3);
    lcd.print("MHz");
  }
  //Частота четвертого ядра
}


void draw_names_stats3() {
  //Количетсво потребляемых процессором ватт
  lcd.setCursor(0, 0);
  lcd.print("CPU Power:");
  if (PCdata[6] < 10) {
    lcd.setCursor(11, 0);
    lcd.print(PCdata[6]);
    lcd.setCursor(14, 0);
    lcd.print("Watt");
    lcd.setCursor(12, 0);
    lcd.print(" ");
  } else {
    lcd.setCursor(11, 0);
    lcd.print(PCdata[6]);
    lcd.setCursor(14, 0);
    lcd.print("Watt");
  }
  //Количетсво потребляемых процессором ватт


  //Температура SSD
  lcd.setCursor(0, 1);
  lcd.print("SSD temp:");
  lcd.setCursor(10, 1);
  lcd.print(PCdata[10]);
  lcd.setCursor(12, 1);
  lcd.write(223);
  //Температура SSD

  //Дата и время
  lcd.setCursor(5, 2);
  lcd.print("WORK TIME:");
  time_t t = now();
  lcd.setCursor(3, 3);
  lcd.print(hour(t));
  lcd.setCursor(5, 3);
  lcd.print("h");
  lcd.setCursor(7, 3);
  lcd.print(minute(t));
  lcd.setCursor(9, 3);
  lcd.print("min");
  lcd.setCursor(13, 3);
  lcd.print(second(t));
  lcd.setCursor(15, 3);
  lcd.print("sec");
  //Дата и время
}


void display_switch() {
  if (millis() - myTimer1 >= 10000) {  // ищем разницу
    myTimer1 = millis();               // сброс таймера
    display_flag = !display_flag;
    lcd.clear();
  }
}


void initBar3() {
  byte right_empty[8] = { 0b11111, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b11111 };
  byte left_empty[8] = { 0b11111, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b11111 };
  byte center_empty[8] = { 0b11111, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b11111 };
  byte bar2[] = { 0b11111, 0b11000, 0b11000, 0b11000, 0b11000, 0b11000, 0b11000, 0b11111 };
  byte bar3[] = { B11111, B11100, B11100, B11100, B11100, B11100, B11100, B11111 };
  byte bar4[] = { B11111, B11110, B11110, B11110, B11110, B11110, B11110, B11111 };
  lcd.createChar(0, left_empty);
  lcd.createChar(1, center_empty);
  lcd.createChar(2, right_empty);
  lcd.createChar(3, bar2);
  lcd.createChar(4, bar3);
  lcd.createChar(5, bar4);
}
void fillBar3(byte start_pos, byte row, byte bar_length, byte fill_percent) {
  byte infill = bar_length * fill_percent / 10;
  byte fract = infill % 10;
  infill = infill / 10;
  // change_flag - true слева, false справа
  if (infill < bar_length - 1) {
    if (!change_flag) {
      change_flag = true;
      byte bar2[] = { 0b11111, 0b11000, 0b11000, 0b11000, 0b11000, 0b11000, 0b11000, 0b11111 };
      byte bar3[] = { B11111, B11100, B11100, B11100, B11100, B11100, B11100, B11111 };
      byte bar4[] = { B11111, B11110, B11110, B11110, B11110, B11110, B11110, B11111 };
      lcd.createChar(3, bar2);
      lcd.createChar(4, bar3);
      lcd.createChar(5, bar4);
    }
  } else {
    if (change_flag) {
      change_flag = false;
      byte leftbar1[] = { B11111, B10001, B10001, B10001, B10001, B10001, B10001, B11111 };
      byte leftbar2[] = { B11111, B11001, B11001, B11001, B11001, B11001, B11001, B11111 };
      byte leftbar3[] = { B11111, B11101, B11101, B11101, B11101, B11101, B11101, B11111 };
      lcd.createChar(3, leftbar1);
      lcd.createChar(4, leftbar2);
      lcd.createChar(5, leftbar3);
    }
  }
  lcd.setCursor(start_pos, row);
  if (infill == 0) {
    if (fract >= 0 && fract < 2) lcd.write(0);
    else if (fract >= 2 && fract < 4) lcd.write(0);
    else if (fract >= 4 && fract < 6) lcd.write(3);
    else if (fract >= 6 && fract < 8) lcd.write(4);
    else if (fract >= 8) lcd.write(5);
  } else lcd.write(255);
  for (int n = 1; n < bar_length - 1; n++) {
    if (n < infill) lcd.write(255);
    if (n == infill) {
      if (fract >= 0 && fract < 2) lcd.write(1);
      else if (fract >= 2 && fract < 4) lcd.write(0);
      else if (fract >= 4 && fract < 6) lcd.write(3);
      else if (fract >= 6 && fract < 8) lcd.write(4);
      else if (fract >= 8) lcd.write(5);
    }
    if (n > infill) lcd.write(1);
  }
  if (infill == bar_length - 1) {
    if (fract >= 0 && fract < 2) lcd.write(2);
    else if (fract >= 2 && fract < 4) lcd.write(3);
    else if (fract >= 4 && fract < 6) lcd.write(4);
    else if (fract >= 6 && fract < 8) lcd.write(5);
    else if (fract >= 8) lcd.write(255);
  } else if (infill == bar_length) lcd.write(255);
  else lcd.write(2);
}
