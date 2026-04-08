/* Copyright 2023 splitkb.com <support@splitkb.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include QMK_KEYBOARD_H
#include "gpio.h"

#ifdef OLED_ENABLE
void render_logo(void);
void render_logo_text(void);
void render_space(void);
void render_layer_state(void);
void render_mod_status_gui_alt(uint8_t modifiers);
void render_mod_status_ctrl_shift(uint8_t modifiers);
void render_kb_LED_state(void);
#endif

enum layers {
  _DEFAULT = 0,
  _FUNCTION,
  _NAVIGATION,
  _ADJUST,
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    // clang-format off
    [_DEFAULT] = LAYOUT(
        KC_GRV,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,                             KC_6,            KC_7,    KC_8,    KC_9,   KC_0,    KC_GRV,
        KC_ESC,  KC_Q,    KC_W,    KC_F,    KC_P,    KC_B,                             KC_J,            KC_L,    KC_U,    KC_Y,   KC_SCLN, KC_BSPC,
        KC_TAB,  KC_A,    KC_R,    KC_S,    KC_T,    KC_G,                             KC_M,            KC_N,    KC_E,    KC_I,   KC_O,    KC_QUOT,
        KC_LSFT, KC_X,    KC_C,    KC_D,    KC_V,    KC_Z,          KC_NO,     KC_NO,  KC_K,            KC_H,    KC_COMM, KC_DOT, KC_SLSH, KC_RSFT,
                          KC_LGUI, KC_LALT, KC_LCTL, TT(_FUNCTION), KC_ENT,    KC_SPC, TT(_NAVIGATION), KC_RCTL, KC_RALT, KC_RGUI
    ),
    [_FUNCTION] = LAYOUT(
        KC_TRNS, KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,                        KC_F6,         KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,
        KC_GRV,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,                         KC_6,          KC_7,    KC_8,    KC_9,    KC_0,    KC_F12,
        KC_TRNS, KC_EXLM, KC_AT,   KC_HASH, KC_DLR,  KC_PERC,                      KC_CIRC,       KC_AMPR, KC_ASTR, KC_LPRN, KC_RPRN, KC_PIPE,
        KC_TRNS, KC_EQL,  KC_MINS, KC_PLUS, KC_LCBR, KC_RCBR, KC_NO,      KC_NO,   KC_LBRC,       KC_RBRC, KC_SCLN, KC_COLN, KC_BSLS, KC_TRNS,
                          KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,    KC_TRNS, MO(_ADJUST),   KC_TRNS, KC_TRNS, KC_TRNS
    ),
    [_NAVIGATION] = LAYOUT(
        KC_TRNS, KC_TRNS,    KC_TRNS,    KC_TRNS,    KC_TRNS,    KC_TRNS,                          KC_TRNS, KC_TRNS,       KC_TRNS, KC_TRNS,       KC_TRNS,       KC_TRNS,
        KC_TRNS, KC_INS,     KC_PSCR,    KC_APP,     KC_NO,      KC_NO,                            KC_PGUP, LCTL(KC_LEFT), KC_UP,   LCTL(KC_RGHT), LCTL(KC_BSPC), KC_BSPC,
        KC_TRNS, KC_LALT,    KC_LCTL,    KC_LSFT,    KC_NO,      KC_CAPS,                          KC_PGDN, KC_LEFT,       KC_DOWN, KC_RGHT,       KC_DEL,        KC_BSPC,
        KC_TRNS, LCTL(KC_Z), LCTL(KC_X), LCTL(KC_C), LCTL(KC_V), KC_NO,       KC_NO,      KC_NO,   KC_HOME, KC_NO,         KC_END,  KC_NO,         KC_TRNS,       KC_TRNS,
                             KC_TRNS,    KC_TRNS,    KC_TRNS,    MO(_ADJUST), KC_TRNS,    KC_TRNS, KC_TRNS, KC_TRNS,       KC_TRNS, KC_TRNS
    ),
    [_ADJUST] = LAYOUT(
        QK_BOOT, KC_NO,   KC_NO,  KC_NO,   KC_NO,   KC_NO,                          KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,
        KC_NO,   KC_NO,   KC_NO,  KC_NO,   KC_NO,   KC_NO,                          KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,
        KC_NO,   KC_NO ,  KC_NO,  KC_NO,   KC_NO,   KC_NO,                          KC_NO,   KC_VOLD, KC_MUTE, KC_VOLU, KC_NO,   KC_NO,
        KC_NO,   KC_NO,   KC_NO,  KC_NO,   KC_NO,   KC_NO,   KC_TRNS,      KC_TRNS, KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_MPRV, KC_MPLY,
                          KC_NO,  KC_MNXT, KC_TRNS, KC_TRNS, KC_TRNS,      KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS
    )
    // clang-format on
};

void keyboard_pre_init_user(void) {
  gpio_set_pin_output(LED_POWER_PIN);
  gpio_write_pin_high(LED_POWER_PIN);
}

const char NEW_FIRMWARE[] = "Awaiting\nNew\nFirmware\0";
const char REBOOTING[] = "Rebooting\0";

void oled_render_boot(bool bootloader) {
  oled_clear();

  const char *msg = bootloader ? NEW_FIRMWARE : REBOOTING;
  oled_write_P(PSTR(msg), false);

  oled_render_dirty(true);
}

bool shutdown_user(bool jump_to_bootloader) {
  oled_render_boot(jump_to_bootloader);

  return false;
}

bool oled_task_user(void) {
  // Renders the current keyboard state (layers and mods)
  render_logo();
  render_logo_text();
  render_space();
  render_layer_state();
  render_space();
  render_mod_status_gui_alt(get_mods() | get_oneshot_mods());
  render_mod_status_ctrl_shift(get_mods() | get_oneshot_mods());
  render_kb_LED_state();

  return false;
}
