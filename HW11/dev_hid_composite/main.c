#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "hardware/gpio.h"
#include "pico/time.h"

#include "bsp/board_api.h"
#include "tusb.h"
#include "pico/stdlib.h"

#include "usb_descriptors.h"

#define LEFT_BUTTON 18
#define UP_BUTTON 19
#define RIGHT_BUTTON 20
#define DOWN_BUTTON 21

#define MODE_BUTTON 17
#define MODE_LED 16

#define RADIUS 10
#define INCREMENT 0.1

absolute_time_t leftpress, rightpress, uppress, downpress;

enum  {
  BLINK_NOT_MOUNTED = 250,
  BLINK_MOUNTED = 1000,
  BLINK_SUSPENDED = 2500,
};

static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;

void led_blinking_task(void);
void hid_task(void);

int8_t get_speed(int64_t held_ms) {
  if (held_ms < 500) return 2;
  if (held_ms < 1500) return 5;
  return 10;
}

void button_init() { // inits the 5 buttons and the LED
  gpio_init(MODE_LED);
  gpio_set_dir(MODE_LED, GPIO_OUT);

  gpio_init(LEFT_BUTTON);
  gpio_set_dir(LEFT_BUTTON, GPIO_IN);
  gpio_pull_up(LEFT_BUTTON);

  gpio_init(UP_BUTTON);
  gpio_set_dir(UP_BUTTON, GPIO_IN);
  gpio_pull_up(UP_BUTTON);

  gpio_init(RIGHT_BUTTON);
  gpio_set_dir(RIGHT_BUTTON, GPIO_IN);
  gpio_pull_up(RIGHT_BUTTON);

  gpio_init(DOWN_BUTTON);
  gpio_set_dir(DOWN_BUTTON, GPIO_IN);
  gpio_pull_up(DOWN_BUTTON);

  gpio_init(MODE_BUTTON);
  gpio_set_dir(MODE_BUTTON, GPIO_IN);
  gpio_pull_up(MODE_BUTTON);
}

/*------------- MAIN -------------*/
int main(void)
{
  board_init();
  button_init();

  // init device stack on configured roothub port
  tud_init(BOARD_TUD_RHPORT);

  if (board_init_after_tusb) {
    board_init_after_tusb();
  }

  bool theMode = true;
  float angle = 0.0f;

  int8_t deltax = 0, deltay = 0;
  leftpress = 0;
  rightpress = 0;
  uppress = 0;
  downpress = 0;

  bool prev_butt = true;

  while (1)
  {
    tud_task();
    led_blinking_task();

    bool curr_butt = gpio_get(MODE_BUTTON);
    if (!curr_butt && prev_butt) {
      theMode = !theMode;
      sleep_ms(200);
    }

    prev_butt = curr_butt;

    // if theMode is false, this is free roam mode
    // if theMode is true, move cursor in a circle
    if (theMode) {
      gpio_put(MODE_LED, false);
      hid_task();
    }

    else {
      gpio_put(MODE_LED, true);
      deltax = (int8_t) (RADIUS*cosf(angle));
      deltay = (int8_t) (RADIUS*sinf(angle));

      angle += INCREMENT;
      if (angle >= 2*3.14159265) {
        angle -= 2*3.14159265;
      }

      if (!tud_hid_ready()) continue;

      tud_hid_mouse_report(REPORT_ID_MOUSE, 0, deltax, deltay, 0, 0);

      sleep_ms(10);
    }
  }
}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void)
{
  blink_interval_ms = BLINK_MOUNTED;
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
  blink_interval_ms = BLINK_NOT_MOUNTED;
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
  (void) remote_wakeup_en;
  blink_interval_ms = BLINK_SUSPENDED;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
  blink_interval_ms = tud_mounted() ? BLINK_MOUNTED : BLINK_NOT_MOUNTED;
}

//--------------------------------------------------------------------+
// USB HID
//--------------------------------------------------------------------+

static void send_hid_report(uint8_t report_id, uint32_t btn)
{
  // skip if hid is not ready yet
  if ( !tud_hid_ready() ) return;

  switch(report_id)
  {
    case REPORT_ID_KEYBOARD:
    {
      // use to avoid send multiple consecutive zero report for keyboard
      static bool has_keyboard_key = false;

      if ( btn )
      {
        uint8_t keycode[6] = { 0 };
        keycode[0] = HID_KEY_A;

        tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, keycode);
        has_keyboard_key = true;
      }else
      {
        // send empty key report if previously has key pressed
        if (has_keyboard_key) tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, NULL);
        has_keyboard_key = false;
      }
    }
    break;

    case REPORT_ID_MOUSE:
    {
      int8_t const delta = 5;

      // no button, right + down, no scroll, no pan
      tud_hid_mouse_report(REPORT_ID_MOUSE, 0x00, delta, delta, 0, 0);
    }
    break;

    case REPORT_ID_CONSUMER_CONTROL:
    {
      // use to avoid send multiple consecutive zero report
      static bool has_consumer_key = false;

      if ( btn )
      {
        // volume down
        uint16_t volume_down = HID_USAGE_CONSUMER_VOLUME_DECREMENT;
        tud_hid_report(REPORT_ID_CONSUMER_CONTROL, &volume_down, 2);
        has_consumer_key = true;
      }else
      {
        // send empty key report (release key) if previously has key pressed
        uint16_t empty_key = 0;
        if (has_consumer_key) tud_hid_report(REPORT_ID_CONSUMER_CONTROL, &empty_key, 2);
        has_consumer_key = false;
      }
    }
    break;

    case REPORT_ID_GAMEPAD:
    {
      // use to avoid send multiple consecutive zero report for keyboard
      static bool has_gamepad_key = false;

      hid_gamepad_report_t report =
      {
        .x   = 0, .y = 0, .z = 0, .rz = 0, .rx = 0, .ry = 0,
        .hat = 0, .buttons = 0
      };

      if ( btn )
      {
        report.hat = GAMEPAD_HAT_UP;
        report.buttons = GAMEPAD_BUTTON_A;
        tud_hid_report(REPORT_ID_GAMEPAD, &report, sizeof(report));

        has_gamepad_key = true;
      }else
      {
        report.hat = GAMEPAD_HAT_CENTERED;
        report.buttons = 0;
        if (has_gamepad_key) tud_hid_report(REPORT_ID_GAMEPAD, &report, sizeof(report));
        has_gamepad_key = false;
      }
    }
    break;

    default: break;
  }
}

// Every 10ms, we will sent 1 report for each HID profile (keyboard, mouse etc ..)
// tud_hid_report_complete_cb() is used to send the next report after previous one is complete
void hid_task(void)
{
  const uint32_t interval_ms = 10;
  static uint32_t start_ms = 0;

  if (board_millis() - start_ms < interval_ms) return;
  start_ms += interval_ms;

  if (!tud_hid_ready()) return;

  int8_t deltax = 0, deltay = 0;
  absolute_time_t currtime = get_absolute_time();

  // LEFT
  if (!gpio_get(LEFT_BUTTON)) {
    if (leftpress == 0) {
      leftpress = currtime;
    }
    int64_t held_ms = absolute_time_diff_us(leftpress, currtime) / 1000;
    deltax -= get_speed(held_ms);
  } 
  else {
    leftpress = 0;
  }

  // RIGHT
  if (!gpio_get(RIGHT_BUTTON)) {
    if (rightpress == 0) {
      rightpress = currtime;
    }
    int64_t held_ms = absolute_time_diff_us(rightpress, currtime) / 1000;
    deltax += get_speed(held_ms);
  } 
  else {
    rightpress = 0;
  }

  // UP
  if (!gpio_get(UP_BUTTON)) {
    if (uppress == 0) {
      uppress = currtime;
    }
    int64_t held_ms = absolute_time_diff_us(uppress, currtime) / 1000;
    deltay -= get_speed(held_ms);
  } 
  else {
    uppress = 0;
  }

  // DOWN
  if (!gpio_get(DOWN_BUTTON)) {
    if (downpress == 0) {
      downpress = currtime;
    }
    int64_t held_ms = absolute_time_diff_us(downpress, currtime) / 1000;
    deltay += get_speed(held_ms);
  } 
  else {
    downpress = 0;
  }

  if (deltax != 0 || deltay != 0) {
    tud_hid_mouse_report(REPORT_ID_MOUSE, 0, deltax, deltay, 0, 0);
  }
}

// Invoked when sent REPORT successfully to host
// Application can use this to send the next report
// Note: For composite reports, report[0] is report ID
void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* report, uint16_t len)
{
  (void) instance;
  (void) len;

  uint8_t next_report_id = report[0] + 1u;

  if (next_report_id < REPORT_ID_COUNT)
  {
    send_hid_report(next_report_id, board_button_read());
  }
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
  // TODO not Implemented
  (void) instance;
  (void) report_id;
  (void) report_type;
  (void) buffer;
  (void) reqlen;

  return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
  (void) instance;

  if (report_type == HID_REPORT_TYPE_OUTPUT)
  {
    // Set keyboard LED e.g Capslock, Numlock etc...
    if (report_id == REPORT_ID_KEYBOARD)
    {
      // bufsize should be (at least) 1
      if ( bufsize < 1 ) return;

      uint8_t const kbd_leds = buffer[0];

      if (kbd_leds & KEYBOARD_LED_CAPSLOCK)
      {
        // Capslock On: disable blink, turn led on
        blink_interval_ms = 0;
        board_led_write(true);
      }else
      {
        // Caplocks Off: back to normal blink
        board_led_write(false);
        blink_interval_ms = BLINK_MOUNTED;
      }
    }
  }
}

//--------------------------------------------------------------------+
// BLINKING TASK
//--------------------------------------------------------------------+
void led_blinking_task(void)
{
  static uint32_t start_ms = 0;
  static bool led_state = false;

  // blink is disabled
  if (!blink_interval_ms) return;

  // Blink every interval ms
  if ( board_millis() - start_ms < blink_interval_ms) return; // not enough time
  start_ms += blink_interval_ms;

  board_led_write(led_state);
  led_state = 1 - led_state; // toggle
}
