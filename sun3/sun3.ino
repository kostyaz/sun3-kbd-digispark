#include <SoftSerial_INT0.h>
#include <DigiKeyboard.h>

typedef struct {
  uint8_t key;
  uint8_t value;
} table_item_t;

#define LED 1
SoftSerial keyboard(2, 0, true);

#define MAX_KEYS 6
uint8_t pressed_keys[MAX_KEYS];
int nr_pressed_keys = 0;

const table_item_t modifiers_table[] = {
  { 0x4C, MOD_CONTROL_LEFT },
  { 0x63, MOD_SHIFT_LEFT },
  { 0x6E, MOD_SHIFT_RIGHT },
  { 0x13, MOD_ALT_LEFT },
  { 0x0D, MOD_ALT_RIGHT },
  { 0x78, MOD_GUI_LEFT },
  { 0x7A, MOD_GUI_RIGHT }
};

const table_item_t keys_table[] = {
  { 0x05, KEY_F1 },
  { 0x06, KEY_F2 },
  { 0x08, KEY_F3 },
  { 0x0A, KEY_F4 },
  { 0x0C, KEY_F5 },
  { 0x0E, KEY_F6 },
  { 0x10, KEY_F7 },
  { 0x11, KEY_F8 },
  { 0x12, KEY_F9 },
  { 0x07, KEY_F10 },
  { 0x09, KEY_F11 },
  { 0x0B, KEY_F12 },
  { 0x58, 49 },   // \|
  { 0x42, 76 },   // Delete
  { 0x01, 120 },  // Stop
  { 0x03, 121 },  // Again
  { 0x15, 72 },   // Pause
  { 0x16, 70 },   // Print Screen
  { 0x17, 71 },   // Scroll Lock
  { 0x62, 83 },   // Num Lock
  { 0x19, 163 },  // Props
  { 0x1A, 122 },  // Undo
  { 0x1D, 41 },   // Esc
  { 0x1E, KEY_1 },
  { 0x1F, KEY_2 },
  { 0x20, KEY_3 },
  { 0x21, KEY_4 },
  { 0x22, KEY_5 },
  { 0x23, KEY_6 },
  { 0x24, KEY_7 },
  { 0x25, KEY_8 },
  { 0x26, KEY_9 },
  { 0x27, KEY_0 },
  { 0x28, 45 },   // -_
  { 0x29, 46 },   // =+
  { 0x2B, 42 },   // Backspace
  { 0x2D, 103 },  // Num =
  { 0x2E, 84 },   // Num /
  { 0x2F, 85 },   // Num *
  { 0x47, 86 },   // Num -
  { 0x31, 119 },  // Front (Select)
  { 0x33, 124 },  // Copy
  { 0x35, 43 },   // Tab
  { 0x36, KEY_Q },
  { 0x37, KEY_W },
  { 0x38, KEY_E },
  { 0x39, KEY_R },
  { 0x3A, KEY_T },
  { 0x3B, KEY_Y },
  { 0x3C, KEY_U },
  { 0x3D, KEY_I },
  { 0x3E, KEY_O },
  { 0x3F, KEY_P },
  { 0x40, 47 },   // [{
  { 0x41, 48 },   // ]}
  { 0x59, KEY_ENTER },
  { 0x44, 95 },   // Num Home
  { 0x45, 96 },   // Num Up
  { 0x46, 97 },   // Num PgUp
  { 0x7D, 87 },   // Num +
  { 0x48, 116 },  // Open (Execute)
  { 0x49, 125 },  // Paste
  { 0x4D, KEY_A },
  { 0x4E, KEY_S },
  { 0x4F, KEY_D },
  { 0x50, KEY_F },
  { 0x51, KEY_G },
  { 0x52, KEY_H },
  { 0x53, KEY_J },
  { 0x54, KEY_K },
  { 0x55, KEY_L },
  { 0x56, 51 },   // ;:
  { 0x57, 52 },   // '"
  { 0x2A, 53 },   // `~
  { 0x5B, 92 },   // Num Left
  { 0x5C, 93 },   // Num 5
  { 0x5D, 94 },   // Num Right
  { 0x5F, 126 },  // Find
  { 0x61, 123 },  // Cut
  { 0x64, KEY_Z },
  { 0x65, KEY_X },
  { 0x66, KEY_C },
  { 0x67, KEY_V },
  { 0x68, KEY_B },
  { 0x69, KEY_N },
  { 0x6A, KEY_M },
  { 0x6B, 54 },   // ,<
  { 0x6C, 55 },   // .>
  { 0x6D, 56 },   // /?
  { 0x6F, 83 },   // Line Feed -> remapping to Num Lock
  { 0x70, 89 },   // Num End
  { 0x71, 90 },   // Num Down
  { 0x72, 91 },   // Num PgDn
  { 0x5A, 88 },   // Num Enter
  { 0x76, 117 },  // Help
  { 0x77, 57 },   // Caps Lock
  { 0x79, KEY_SPACE },
  { 0x43, 101 },  // Compose (App)
  { 0x5E, 73 },   // Ins
  { 0x32, 76 }    // Del
};

void process_code(uint8_t code) {
  if (code < 0x7F) {
    // Key pressed
    if (nr_pressed_keys < MAX_KEYS) {
      pressed_keys[nr_pressed_keys++] = code;
    }
  }
  else if (code == 0x7F) {
    // All keys released
    nr_pressed_keys = 0;
  }
  else if (code < 0xFE) {
    // Key released
    const uint8_t make_code = code ^ 0x80;
    int write_idx = 0;
    for (int read_idx = 0; read_idx < nr_pressed_keys; read_idx++) {
      pressed_keys[write_idx] = pressed_keys[read_idx];
      if (pressed_keys[write_idx] != make_code) {
        write_idx++;
      }
    }
    nr_pressed_keys = write_idx;
  }
}

uint8_t find_in_table(const table_item_t table[], int len, uint8_t key) {
  uint8_t value = 0;
  for (int i = 0; i < len; i++) {
    if (table[i].key == key) {
      value = table[i].value;
      break;
    }
  }
  return value;
}

void sun_to_usb(uint8_t keys[], uint8_t* modifiers) {
  *modifiers = 0;
  int key_idx = 0;
  for (int i = 0; i < nr_pressed_keys; i++) {
    const uint8_t code = pressed_keys[i];
    const uint8_t mod = find_in_table(
      modifiers_table,
      sizeof(modifiers_table)/sizeof(modifiers_table[0]),
      code
    );
    if (mod > 0) {
      *modifiers |= mod;
    }
    else {
      const uint8_t key = find_in_table(
        keys_table,
        sizeof(keys_table)/sizeof(keys_table[0]),
        code
      );
      if (key > 0) {
        keys[key_idx++] = key;
      }
    }
  }
  // Fill-in the rest with zeros
  for (; key_idx < MAX_KEYS; key_idx++) {
    keys[key_idx] = 0;
  }
}

void print_binary(uint8_t data) {
  for (int i = 7; i >= 0; i--) {
    if (data & (1 << i)) {
      DigiKeyboard.print("1");
    }
    else {
      DigiKeyboard.print("0");
    }
  }
  DigiKeyboard.println("");
}

void keyboard_config() {
  // Disable bell
  DigiKeyboard.delay(50);
  keyboard.write(0x03);
  // Disable click
  DigiKeyboard.delay(50);
  keyboard.write(0x0B);
}

// the setup routine runs once when you press reset:
void setup() {
  DigiKeyboard.update();
  // UART comm
  keyboard.begin(1200);
  // LED
  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);
  // Keyboard
  keyboard_config();
  digitalWrite(LED, LOW);
}

// the loop routine runs over and over again forever:
void loop() {
  static uint8_t last_keys[MAX_KEYS] = {0};
  DigiKeyboard.update();
  if (keyboard.available()) {
    DigiKeyboard.update();
    digitalWrite(LED, HIGH);
    DigiKeyboard.update();
    process_code(keyboard.read());
    DigiKeyboard.update();
    uint8_t keys[MAX_KEYS];
    uint8_t modifiers;
    DigiKeyboard.update();
    sun_to_usb(keys, &modifiers);
    DigiKeyboard.update();
    if (memcmp(keys, last_keys, MAX_KEYS)) {
      DigiKeyboard.sendKeyPress(keys[0], modifiers);
      memcpy(last_keys, keys, MAX_KEYS);
      DigiKeyboard.update();
    }
    digitalWrite(LED, LOW);
  }
}
