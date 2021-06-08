#include <stdio.h>
#include "hardware/gpio.h"
#include "../lib/keypad_4x4/keypad_4x4.h"
#include "../lib/LCDI2C/LCD_I2C.h"
#include "../lib/picoServo/picoServo.h"
#include "hardware/i2c.h"
#include "pico/time.h"

uint8_t state = 0, nxtState = 0;
char withdrawCode[50] = "\0";
char withdraw[50] = "500";
char status[50] = "\0";
alarm_id_t keypadWait;

void printMenu();
void change_state(char key);
void gpio_callback(uint gpio, uint32_t event);
int64_t alarm_callback(alarm_id_t id, void* user_data);


int main() {
    stdio_init_all();
    lcd_init();
    keypad_init();
    picoServo_init(0);
    keypad_irq_enable(true, gpio_callback);
    
    printf("Hello World");

    printMenu();
    while (1) {
      picoServo_setDegree(0, 90);
      busy_wait_us(1000000);
      picoServo_setDegree(0, 5);
      busy_wait_us(1000000);
    }
}

void printMenu() {
  lcd_clear();
  printf("Current state: %i\n", state);
  char status_[] = "$";
  char withdraw_[] = "$";
  switch (state) {
    case 0:
      lcd_print("Bienvenido", 0);
      lcd_print("Presiona #", 1);
      break;
    case 1:
      lcd_print("Con tarjeta    *", 0);
      lcd_print("Sin tarjeta    #", 1);
      break;
    case 2:
      lcd_print("Coloque tarjeta", 0);
      lcd_print("en el lector", 1);
      break;
    case 3:
      lcd_print("Leyendo tarjeta", 0);
      lcd_print("Espere", 1);
      break;
    case 4:
      lcd_print("Consultar saldo*", 0);
      lcd_print("Retirar dinero #", 1);
      break;
    case 5:
      lcd_print("Saldo disponible", 0);
      strncat(status_, status, strlen(status));
      lcd_print(status_, 1);
      break;
    case 6:
      lcd_print("Nueva operacion*", 0);
      lcd_print("Salir          #", 1);
      break;
    case 7:
      lcd_print("Ingrese monto", 0);
      strncat(withdraw_, withdraw, strlen(withdraw));
      lcd_print(withdraw_, 1);
      break;
    case 8:
      lcd_print("Confirmar retiro", 0);
      strcpy(withdraw_, "de $");
      strncat(withdraw_, withdraw, strlen(withdraw));
      lcd_print(withdraw_, 1);
      break;
    case 9:
      lcd_print("Dispensando", 0);
      lcd_print("dinero", 1);
      break;
    case 10:
      lcd_print("Fin de operacion", 0);
      lcd_print("Salir          #", 1);
      break;
    case 11:
      lcd_print("Codigo de retiro", 0);
      lcd_print(withdrawCode, 1);
      break;
    case 12:
      lcd_print("Cancelado", 0);
      lcd_print(" ", 1);
      break;
    default:
      lcd_print("ERROR", 0);
      lcd_print("ERROR", 1);
      break;
  }
}

void change_state(char key) {
    printf("Current: %i\n", state);
    switch (state) {
        case 0:
          if (key == '#') {
            nxtState = 1;
          }
          break;
        case 1:
          if (key == '*') {
            nxtState = 2;
          }
          else if (key == '#') {
            nxtState = 11;
          }
          break;
        case 2:
          // IF rfid is card
          break;
        case 3: // Connect to nodemcu and database
          //function to connect with database
          nxtState = 4;
          break;
        case 4:
            if (key == '*') {
              nxtState = 5;
            } else if (key == '#') {
              nxtState = 7;
            }
            break;
        case 5:
            if (key == '#') {
              nxtState = 6;
            }
        break;
        case 6:
            if (key == '*') {
              withdraw[0] = '\0';
              nxtState = 4;
            } else if (key == '#') {
              nxtState = 0;
            }
            break;
        case 7:
            if (key == '#') {
              nxtState = 8;
            }
            else if (key >= '0' && key <= '9' && strlen(withdraw) < 6) {
              strncat(withdraw, &key, 1);
            } else if (key == 'D' && strlen(withdraw) > 0) {
              memmove(&withdraw[strlen(withdraw)-1], &withdraw[strlen(withdraw)], 1);
            }
            printMenu();
            break;
        case 8:
            if (key == '#') {
              nxtState = 9;
            }
            break;
        case 9:
            //timer
            //servo
            nxtState = 10;
            break;
        case 10:
            if (key == '#') {
              nxtState = 0;
            }
            break;
        case 11:
            if (key == '#') {
              nxtState = 8;
            }
            else if (key >= '0' && key <= '9' && strlen(withdrawCode) < 6) {
                if (withdrawCode[0] == '\0') {
                    strcpy(withdrawCode, &key);
                } else {
                    strncat(withdrawCode, &key, 1);
                }
            } else if (key == 'D' && strlen(withdrawCode) > 0) {
                memmove(&withdrawCode[strlen(withdrawCode)-1], &withdrawCode[strlen(withdrawCode)], 1);
            }
            printf("%s tiene %i caracteres\n", withdrawCode, strlen(withdrawCode));
            printMenu();
            break;
        case 12:
            //timer 1s
            nxtState = 0;
            break;
        default:
            break;
    }
    printf("Next: %i\n", nxtState);
    if (state != nxtState) {
        state = nxtState;
        printMenu();
    }
}

void gpio_callback(uint gpio, uint32_t event) {
    if (gpio >= 18 && gpio <= 21) {
        char key = keypad_get_key();
        change_state(key);
        keypadWait = add_alarm_in_ms(1000, alarm_callback, NULL, true);
        keypad_irq_enable(false, gpio_callback);
    }
}

int64_t alarm_callback (alarm_id_t id, void *user_data) {
    if (id == keypadWait) {
        keypad_irq_enable(true, gpio_callback);
    }
}