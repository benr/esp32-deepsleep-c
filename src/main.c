#include "mgos.h"
#include "rom/rtc.h"
#include "driver/rtc_io.h"
#include "esp_sleep.h"


//  DEEP SLEEP HELPER FUNCTION: 
//  
static void gotosleep(int pin, void *arg){
  esp_sleep_enable_ext0_wakeup(pin, 1);		// 1== HIGH (Sensor Open)

  printf("All done.  Going to sleep in ");
  for(int i=5; i>0; --i){
    printf("%d...\n", i);
    sleep(2);
  }
  printf("Good night.\n");

  esp_deep_sleep_start();
}

//  HELPER FUNCTION: Decode the reason for resetting. 
//  Refer to:
//    https://github.com/espressif/arduino-esp32/blob/master/libraries/ESP32/examples/ResetReason/ResetReason.ino
//    https://github.com/espressif/esp-idf/blob/master/components/esp32/include/rom/rtc.h
//
void why_reset(){
  int reset_reason = rtc_get_reset_reason(0); 
  printf("Reset Reason (%d): ", reset_reason);

  switch (reset_reason) {
    case 1  : printf("Vbat power on reset");break;
    case 3  : printf("Software reset digital core");break;
    case 4  : printf("Legacy watch dog reset digital core");break;
    case 5  : printf("Deep Sleep reset digital core");break;
    case 6  : printf("Reset by SLC module, reset digital core");break;
    case 7  : printf("Timer Group0 Watch dog reset digital core");break;
    case 8  : printf("Timer Group1 Watch dog reset digital core");break;
    case 9  : printf("RTC Watch dog Reset digital core");break;
    case 10 : printf("Instrusion tested to reset CPU");break;
    case 11 : printf("Time Group reset CPU");break;
    case 12 : printf("Software reset CPU");break;
    case 13 : printf("RTC Watch dog Reset CPU");break;
    case 14 : printf("for APP CPU, reseted by PRO CPU");break;
    case 15 : printf("Reset when the vdd voltage is not stable");break;
    case 16 : printf("RTC Watch dog reset digital core and rtc module");break;
    default : printf("NO_MEAN");
  }  
  printf("\n");
}

//  HELPER FUNCTION: Decode our reason for waking.
//
void why_wake(){
  int wake_cause = esp_sleep_get_wakeup_cause();
  printf("Wake Cause (%d): ", wake_cause);
  switch (wake_cause) {
    case 1  : printf("Wakeup caused by external signal using RTC_IO");
    case 2  : printf("Wakeup caused by external signal using RTC_CNTL");
    case 3  : printf("Wakeup caused by timer");
    case 4  : printf("Wakeup caused by touchpad");
    case 5  : printf("Wakeup caused by ULP program");
    default : printf("Undefined.  In case of deep sleep, reset was not caused by exit from deep sleep.");
  } 
  printf("\n");
}

// Read the sensor via timer
//
static void sensor_timer_cb(void *arg){
  if(mgos_gpio_read(13)){
    printf("Door is Open!  Do stuff....\n");
  } else {
    printf("Door is closed, going to sleep.\n");
    gotosleep(13, NULL);
  }
}

enum mgos_app_init_result mgos_app_init(void) {
  printf("-------------- STARTING APPLICATION -------------\n");
  why_reset();
  why_wake();

  int pin = 13;
  rtc_gpio_deinit(pin);
  mgos_gpio_set_mode(pin, MGOS_GPIO_MODE_INPUT);
  mgos_gpio_set_pull(pin, MGOS_GPIO_PULL_UP);
  printf("MGOS GPIO13 read: %d\n", mgos_gpio_read(13));
  printf("RTC GPIO13 read: %d\n", rtc_gpio_get_level(13));

  mgos_set_timer(2000, MGOS_TIMER_REPEAT, sensor_timer_cb, NULL);

  return MGOS_APP_INIT_SUCCESS;
}
