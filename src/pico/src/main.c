#include <stdio.h>
#include <stdlib.h>
#include "hardware/gpio.h"
#include "../lib/keypad_4x4/keypad_4x4.h"
#include "../lib/LCDI2C/LCD_I2C.h"
#include "../lib/pico-servo/include/pico_servo.h"
#include "hardware/i2c.h"
#include "pico/time.h"

#define SERVO  6
#define BUZZER 22
#define CANCEL 16
#define ALARM  17
#define NMCU   0x17

uint8_t _status[4];
uint8_t state, nxtState;
char withdrawCode[50];
char withdraw[50];
char status[50];
alarm_id_t keypadWait;
alarm_id_t noTouch;
uint8_t getBal;
uint8_t account[4];
bool lowPower;

void nodeMCU_init(void);
void printMenu(void);
void change_state(char key);
void gpio_callback(uint gpio, uint32_t event);
int64_t alarm_callback(alarm_id_t id, void* user_data);
void buttons_init(void);
void resetVars(void);

int main() {
    resetVars();
    stdio_init_all();
    nodeMCU_init();
    lcd_init();
    keypad_init();
    servo_init();
    buttons_init();
    servo_clock_auto();
    servo_attach(SERVO);
    
    servo_move_to(SERVO, 0);
    
    keypad_irq_enable(true, gpio_callback);
    
    noTouch = add_alarm_in_ms(15000, alarm_callback, NULL, true);
    printf("Hello World");

    printMenu();
    while (1) {
      if (state == 9) {
        itoa(atoi(status) - atoi(withdraw), status, 10);
        gpio_put(BUZZER, 1);
        servo_move_to(SERVO, 90);
        busy_wait_us(10000000);
        gpio_put(BUZZER, 0);
        servo_move_to(SERVO, 0);
        change_state(0);
      }
    }
}

void resetVars() {
    state = 0;
    nxtState = 0;
    strncpy(withdrawCode, "\0", 1);
    strncpy(withdraw, "500", 3);
    strncpy(status, "1000", 4);
    getBal = 0x2;
    lowPower = false;
}

void nodeMCU_init(void) {
  // This example will use I2C1
  i2c_init(i2c1, 100 * 1000);
  gpio_set_function(2, GPIO_FUNC_I2C); // SDA
  gpio_set_function(3, GPIO_FUNC_I2C); // SCL
  gpio_pull_up(2);
  gpio_pull_up(3);
  // Make the I2C pins available to picotool
  bi_decl(bi_2pins_with_func(2, 3, GPIO_FUNC_I2C));
}

void printMenu() {
  lcd_clear();
  printf("Current state: %i\n", state);
  char status_[] = "$";
  char withdraw_[] = "$";
  switch (state) {
    case 0:
      resetVars();
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
      change_state(0);
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
    int32_t temp;
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
          temp = 111222333;
          for (int i = 0; i < 4; i++) {
            account[i] = (temp &= 0xFF000000) >> 24;
            temp = temp << 8;
          }
          nxtState = 3;
          // IF rfid is card
          break;
        case 3: // Connect to nodemcu and database
          // printf("Write GetBal: %i \n", i2c_write_blocking(i2c1, NMCU, &getBal, 1, false));
          // printf("Write ACCOUNT: %i \n", i2c_write_blocking(i2c1, NMCU, account, 4, false));
          // printf("Read I2C: %i\n", i2c_read_blocking(i2c1, NMCU, _status, 4, false));
          //function to connect with database
          // printf("Status: %i\n", _status);
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
              if (withdraw[0] == '\0') {
                    withdraw[0] = key;
                    withdraw[1] = '\0';
                } else {
                    strncat(withdraw, &key, 1);
                }
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
                    withdrawCode[0] = key;
                    withdrawCode[1] = '\0';
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

void buttons_init() {
    gpio_init(CANCEL);
    gpio_init(ALARM);
    gpio_init(BUZZER);
    gpio_set_dir(CANCEL, 0);
    gpio_set_dir(ALARM, 0);
    gpio_set_dir(BUZZER, 1);
    gpio_pull_down(CANCEL);
    gpio_pull_down(ALARM);
    gpio_set_irq_enabled_with_callback(CANCEL, 0x8, true, gpio_callback);
    gpio_set_irq_enabled_with_callback(ALARM, 0x8, true, gpio_callback);
}

void gpio_callback(uint gpio, uint32_t event) {
    cancel_alarm(noTouch);
    if (gpio >= 18 && gpio <= 21) {
        if (lowPower) {
            lcd_send_byte(0x08 | 0x04, 0);
            lowPower = false;
        }
        char key = keypad_get_key();
        change_state(key);
        keypadWait = add_alarm_in_ms(1000, alarm_callback, NULL, true);
        keypad_irq_enable(false, gpio_callback);
    } else if (gpio == CANCEL) {
        nxtState = 0;
        state = 0;
        change_state(0);
    } else if (gpio == ALARM) {
        nxtState = 0;
        state = 0;
        change_state(0);
        for (int i = 0; i < 5; i++) {
            gpio_put(BUZZER, 1);
            busy_wait_us(500000);
            gpio_put(BUZZER, 0);
            busy_wait_us(500000);
        }
    }
    noTouch = add_alarm_in_ms(15000, alarm_callback, NULL, true);
}

int64_t alarm_callback (alarm_id_t id, void *user_data) {
    if (id == keypadWait) {
        keypad_irq_enable(true, gpio_callback);
    } else if (id == noTouch) {
        if (state == 0) {
            lcd_send_byte(0x08, 0);
            lowPower = true;
        } else {
            noTouch = add_alarm_in_ms(15000, alarm_callback, NULL, true);
            nxtState = 0;
            state = 0;
            change_state(0);
        }
    }
}