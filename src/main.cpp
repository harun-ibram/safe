#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <stdint.h>

// --- Display pin definitions ---
#define RES_PORT  PORTD
#define RES_DDR   DDRD
#define RES_PIN   PD2

#define DC_PORT   PORTD
#define DC_DDR    DDRD
#define DC_PIN    PD3

#define RES_HIGH()   (RES_PORT  |=  (1 << RES_PIN))
#define RES_LOW()    (RES_PORT  &= ~(1 << RES_PIN))
#define DC_HIGH()    (DC_PORT   |=  (1 << DC_PIN))
#define DC_LOW()     (DC_PORT   &= ~(1 << DC_PIN))

// --- Keypad pin definitions ---
// Rows = outputs (PC5, PC4, PC3, PC2)
#define ROW_DDR   DDRC
#define ROW_PORT  PORTC
#define R1        PC5
#define R2        PC4
#define R3        PC3
#define R4        PC2

// Columns = inputs with pull-ups (PD0, PD1, PD5, PD7)
#define COL_DDR   DDRD
#define COL_PORT  PORTD
#define COL_PIN   PIND
#define C1        PD0
#define C2        PD1
#define C3        PD5
#define C4        PD7

// Key layout matching physical keypad
static const char keymap[4][4] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

static const uint8_t rows[4] = {R1, R2, R3, R4};
static const uint8_t cols[4] = {C1, C2, C3, C4};

// --- Keypad init ---
void keypad_init() {
    // Rows as outputs, start HIGH
    ROW_DDR  |=  (1 << R1) | (1 << R2) | (1 << R3) | (1 << R4);
    ROW_PORT |=  (1 << R1) | (1 << R2) | (1 << R3) | (1 << R4);

    // Columns as inputs with internal pull-ups
    COL_DDR  &= ~((1 << C1) | (1 << C2) | (1 << C3) | (1 << C4));
    COL_PORT |=   (1 << C1) | (1 << C2) | (1 << C3) | (1 << C4);
}

// Returns pressed key char, or 0 if none
char keypad_scan() {
    for (uint8_t r = 0; r < 4; r++) {
        // Pull current row LOW
        ROW_PORT &= ~(1 << rows[r]);
        _delay_us(10); // Settle time

        for (uint8_t c = 0; c < 4; c++) {
            if (!(COL_PIN & (1 << cols[c]))) {
                // Key pressed — wait for release before returning
                while (!(COL_PIN & (1 << cols[c])));
                _delay_ms(20); // Debounce
                ROW_PORT |= (1 << rows[r]); // Row back HIGH
                return keymap[r][c];
            }
        }

        // Row back HIGH before next row
        ROW_PORT |= (1 << rows[r]);
    }
    return 0; // No key pressed
}

// ----------------------------------------------------------------
// Font, SPI, and display code (unchanged from before)
// ----------------------------------------------------------------

static const uint8_t font5x8[][5] PROGMEM = {
    {0x00,0x00,0x00,0x00,0x00}, // 32 ' '
    {0x00,0x00,0x5F,0x00,0x00}, // 33 '!'
    {0x00,0x07,0x00,0x07,0x00}, // 34 '"'
    {0x14,0x7F,0x14,0x7F,0x14}, // 35 '#'
    {0x24,0x2A,0x7F,0x2A,0x12}, // 36 '$'
    {0x23,0x13,0x08,0x64,0x62}, // 37 '%'
    {0x36,0x49,0x55,0x22,0x50}, // 38 '&'
    {0x00,0x05,0x03,0x00,0x00}, // 39 '''
    {0x00,0x1C,0x22,0x41,0x00}, // 40 '('
    {0x00,0x41,0x22,0x1C,0x00}, // 41 ')'
    {0x14,0x08,0x3E,0x08,0x14}, // 42 '*'
    {0x08,0x08,0x3E,0x08,0x08}, // 43 '+'
    {0x00,0x50,0x30,0x00,0x00}, // 44 ','
    {0x08,0x08,0x08,0x08,0x08}, // 45 '-'
    {0x00,0x60,0x60,0x00,0x00}, // 46 '.'
    {0x20,0x10,0x08,0x04,0x02}, // 47 '/'
    {0x3E,0x51,0x49,0x45,0x3E}, // 48 '0'
    {0x00,0x42,0x7F,0x40,0x00}, // 49 '1'
    {0x42,0x61,0x51,0x49,0x46}, // 50 '2'
    {0x21,0x41,0x45,0x4B,0x31}, // 51 '3'
    {0x18,0x14,0x12,0x7F,0x10}, // 52 '4'
    {0x27,0x45,0x45,0x45,0x39}, // 53 '5'
    {0x3C,0x4A,0x49,0x49,0x30}, // 54 '6'
    {0x01,0x71,0x09,0x05,0x03}, // 55 '7'
    {0x36,0x49,0x49,0x49,0x36}, // 56 '8'
    {0x06,0x49,0x49,0x52,0x3C}, // 57 '9'
    {0x00,0x36,0x36,0x00,0x00}, // 58 ':'
    {0x00,0x56,0x36,0x00,0x00}, // 59 ';'
    {0x08,0x14,0x22,0x41,0x00}, // 60 '<'
    {0x14,0x14,0x14,0x14,0x14}, // 61 '='
    {0x00,0x41,0x22,0x14,0x08}, // 62 '>'
    {0x02,0x01,0x51,0x09,0x06}, // 63 '?'
    {0x32,0x49,0x79,0x41,0x3E}, // 64 '@'
    {0x7E,0x11,0x11,0x11,0x7E}, // 65 'A'
    {0x7F,0x49,0x49,0x49,0x36}, // 66 'B'
    {0x3E,0x41,0x41,0x41,0x22}, // 67 'C'
    {0x7F,0x41,0x41,0x22,0x1C}, // 68 'D'
    {0x7F,0x49,0x49,0x49,0x41}, // 69 'E'
    {0x7F,0x09,0x09,0x09,0x01}, // 70 'F'
    {0x3E,0x41,0x49,0x49,0x7A}, // 71 'G'
    {0x7F,0x08,0x08,0x08,0x7F}, // 72 'H'
    {0x00,0x41,0x7F,0x41,0x00}, // 73 'I'
    {0x20,0x40,0x41,0x3F,0x01}, // 74 'J'
    {0x7F,0x08,0x14,0x22,0x41}, // 75 'K'
    {0x7F,0x40,0x40,0x40,0x40}, // 76 'L'
    {0x7F,0x02,0x0C,0x02,0x7F}, // 77 'M'
    {0x7F,0x04,0x08,0x10,0x7F}, // 78 'N'
    {0x3E,0x41,0x41,0x41,0x3E}, // 79 'O'
    {0x7F,0x09,0x09,0x09,0x06}, // 80 'P'
    {0x3E,0x41,0x51,0x21,0x5E}, // 81 'Q'
    {0x7F,0x09,0x19,0x29,0x46}, // 82 'R'
    {0x46,0x49,0x49,0x49,0x31}, // 83 'S'
    {0x01,0x01,0x7F,0x01,0x01}, // 84 'T'
    {0x3F,0x40,0x40,0x40,0x3F}, // 85 'U'
    {0x1F,0x20,0x40,0x20,0x1F}, // 86 'V'
    {0x3F,0x40,0x38,0x40,0x3F}, // 87 'W'
    {0x63,0x14,0x08,0x14,0x63}, // 88 'X'
    {0x07,0x08,0x70,0x08,0x07}, // 89 'Y'
    {0x61,0x51,0x49,0x45,0x43}, // 90 'Z'
    {0x00,0x7F,0x41,0x41,0x00}, // 91 '['
    {0x02,0x04,0x08,0x10,0x20}, // 92 '\'
    {0x00,0x41,0x41,0x7F,0x00}, // 93 ']'
    {0x04,0x02,0x01,0x02,0x04}, // 94 '^'
    {0x40,0x40,0x40,0x40,0x40}, // 95 '_'
    {0x00,0x01,0x02,0x04,0x00}, // 96 '`'
    {0x20,0x54,0x54,0x54,0x78}, // 97 'a'
    {0x7F,0x48,0x44,0x44,0x38}, // 98 'b'
    {0x38,0x44,0x44,0x44,0x20}, // 99 'c'
    {0x38,0x44,0x44,0x48,0x7F}, // 100 'd'
    {0x38,0x54,0x54,0x54,0x18}, // 101 'e'
    {0x08,0x7E,0x09,0x01,0x02}, // 102 'f'
    {0x0C,0x52,0x52,0x52,0x3E}, // 103 'g'
    {0x7F,0x08,0x04,0x04,0x78}, // 104 'h'
    {0x00,0x44,0x7D,0x40,0x00}, // 105 'i'
    {0x20,0x40,0x44,0x3D,0x00}, // 106 'j'
    {0x7F,0x10,0x28,0x44,0x00}, // 107 'k'
    {0x00,0x41,0x7F,0x40,0x00}, // 108 'l'
    {0x7C,0x04,0x18,0x04,0x78}, // 109 'm'
    {0x7C,0x08,0x04,0x04,0x78}, // 110 'n'
    {0x38,0x44,0x44,0x44,0x38}, // 111 'o'
    {0x7C,0x14,0x14,0x14,0x08}, // 112 'p'
    {0x08,0x14,0x14,0x18,0x7C}, // 113 'q'
    {0x7C,0x08,0x04,0x04,0x08}, // 114 'r'
    {0x48,0x54,0x54,0x54,0x20}, // 115 's'
    {0x04,0x3F,0x44,0x40,0x20}, // 116 't'
    {0x3C,0x40,0x40,0x40,0x7C}, // 117 'u'
    {0x1C,0x20,0x40,0x20,0x1C}, // 118 'v'
    {0x3C,0x40,0x30,0x40,0x3C}, // 119 'w'
    {0x44,0x28,0x10,0x28,0x44}, // 120 'x'
    {0x0C,0x50,0x50,0x50,0x3C}, // 121 'y'
    {0x44,0x64,0x54,0x4C,0x44}, // 122 'z'
    {0x00,0x08,0x36,0x41,0x00}, // 123 '{'
    {0x00,0x00,0x7F,0x00,0x00}, // 124 '|'
    {0x00,0x41,0x36,0x08,0x00}, // 125 '}'
    {0x10,0x08,0x08,0x10,0x08}, // 126 '~'
};

void spi_init() {
    DDRB |= (1 << PB2) | (1 << PB3) | (1 << PB5);
    PORTB |= (1 << PB2);
    DDRB &= ~(1 << PB4);
    SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR0);
    SPSR &= ~(1 << SPI2X);
}

void spi_send(uint8_t byte) {
    SPDR = byte;
    while (!(SPSR & (1 << SPIF)));
}

void ili9341_cmd(uint8_t cmd)  { DC_LOW();  spi_send(cmd); }
void ili9341_data(uint8_t d)   { DC_HIGH(); spi_send(d);   }

void ili9341_reset() {
    RES_HIGH(); _delay_ms(10);
    RES_LOW();  _delay_ms(20);
    RES_HIGH(); _delay_ms(150);
}

void ili9341_init() {
    ili9341_reset();
    ili9341_cmd(0x01); _delay_ms(150);
    ili9341_cmd(0x11); _delay_ms(120);
    ili9341_cmd(0x36); ili9341_data(0x48);
    ili9341_cmd(0x3A); ili9341_data(0x55);
    ili9341_cmd(0x29); _delay_ms(50);
}

void ili9341_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    ili9341_cmd(0x2A);
    ili9341_data(x0 >> 8); ili9341_data(x0 & 0xFF);
    ili9341_data(x1 >> 8); ili9341_data(x1 & 0xFF);
    ili9341_cmd(0x2B);
    ili9341_data(y0 >> 8); ili9341_data(y0 & 0xFF);
    ili9341_data(y1 >> 8); ili9341_data(y1 & 0xFF);
    ili9341_cmd(0x2C);
}

void ili9341_fill(uint16_t color) {
    ili9341_set_window(0, 0, 239, 319);
    DC_HIGH();
    uint8_t hi = color >> 8, lo = color & 0xFF;
    for (uint32_t i = 0; i < 76800; i++) { spi_send(hi); spi_send(lo); }
}

void ili9341_draw_char(uint16_t x, uint16_t y, char c,
                       uint16_t fg, uint16_t bg, uint8_t scale) {
    if (c < 32 || c > 126) c = '?';
    uint8_t col_data[5];
    for (uint8_t i = 0; i < 5; i++)
        col_data[i] = pgm_read_byte(&font5x8[c - 32][i]);

    ili9341_set_window(x, y, x + (6 * scale) - 1, y + (8 * scale) - 1);
    DC_HIGH();
    uint8_t fg_hi = fg >> 8, fg_lo = fg & 0xFF;
    uint8_t bg_hi = bg >> 8, bg_lo = bg & 0xFF;

    for (uint8_t row = 0; row < 8; row++) {
        for (uint8_t sr = 0; sr < scale; sr++) {
            for (uint8_t col = 0; col < 5; col++) {
                uint8_t on = (col_data[col] >> row) & 1;
                for (uint8_t sc = 0; sc < scale; sc++) {
                    spi_send(on ? fg_hi : bg_hi);
                    spi_send(on ? fg_lo : bg_lo);
                }
            }
            for (uint8_t sc = 0; sc < scale; sc++) {
                spi_send(bg_hi); spi_send(bg_lo);
            }
        }
    }
}

void ili9341_draw_string(uint16_t x, uint16_t y, const char *str,
                         uint16_t fg, uint16_t bg, uint8_t scale) {
    while (*str) {
        ili9341_draw_char(x, y, *str++, fg, bg, scale);
        x += 6 * scale;
        if (x + 6 * scale > 240) break;
    }
}

// ----------------------------------------------------------------
// Color definitions
// ----------------------------------------------------------------
#define BLACK   0x0000
#define WHITE   0xFFFF
#define RED     0xF800
#define GREEN   0x07E0
#define BLUE    0x001F
#define YELLOW  0xFFE0
#define CYAN    0x07FF

// ----------------------------------------------------------------
// Main: display typed keys on screen
// ----------------------------------------------------------------

// Screen is 240 wide, 320 tall. At scale 3, each char is 18x24px.
// That gives us 13 chars per row, 13 rows per screen.
#define CHAR_SCALE  3
#define CHAR_W      (6 * CHAR_SCALE)   // 18
#define CHAR_H      (8 * CHAR_SCALE)   // 24
#define COLS        (240 / CHAR_W)     // 13 chars per row
#define ROWS        (320 / CHAR_H)     // 13 rows


void buzzer_init() {
    // PD6 (OC0A) as output
    DDRD |= (1 << PD6);

    // Timer0 CTC mode (WGM01 = 1)
    TCCR0A = (1 << WGM01);

    // Toggle OC0A on compare match
    TCCR0A |= (1 << COM0A0);

    // Prescaler = 64
    TCCR0B = (1 << CS01) | (1 << CS00);

    // 1 kHz tone:
    // OCR0A = (F_CPU / (2 * prescaler * freq)) - 1
    OCR0A = 124;
}

void buzzer_on() {
    // Timer running already produces tone
    TCCR0A |= (1 << COM0A0);
}

void buzzer_off() {
    // Disconnect output compare -> pin becomes normal GPIO
    TCCR0A &= ~(1 << COM0A0);
    PORTD &= ~(1 << PD6);
}

char pass[4] = {'1', '2', '3', '4'};
int correct = 0;

#define SERVO PB1


volatile uint16_t servo_angle = 1500;

void servo_init() {
    DDRB |= (1 << SERVO);

    // Fast PWM, TOP = ICR1
    TCCR1A = (1 << WGM11);
    TCCR1B = (1 << WGM13) | (1 << WGM12);

    // Non-inverting PWM
    TCCR1A |= (1 << COM1A1);

    // Prescaler 8
    TCCR1B |= (1 << CS11);

    // 50 Hz (20 ms period)
    ICR1 = 40000;
}

void servo_set_angle(uint8_t angle) {
    if (angle > 180) angle = 180;

    // Map 0–180° → 2000–4000 ticks (~1ms–2ms pulse)
    uint16_t pulse = 2000 + ((uint32_t)angle * 2000) / 180;

    OCR1A = pulse;
}

int main(void) {
    // Set unused display pins to known states so they
    // don't interfere with PORTD column reads
    RES_DDR |= (1 << RES_PIN);   // PD2 output
    DC_DDR  |= (1 << DC_PIN);    // PD3 output
    RES_HIGH();
    DC_LOW();


    spi_init();
    ili9341_init();
    buzzer_init();
    keypad_init();
    servo_init();
    OCR1A = 2000;

    // Startup beep
    buzzer_on();
    _delay_ms(200);
    buzzer_off();

    ili9341_fill(BLACK);

    uint8_t cur_col = 0;  // Current character column (0..COLS-1)
    uint8_t cur_row = 0;  // Current character row    (0..ROWS-1)

    uint8_t unlocked_drawn = 0;

    while (1) {
        char key = keypad_scan();


        if (correct == 4) {
            // Unlocked: set servo to open position
            OCR1A = 4000; // 2 ms pulse for 180
            if (!unlocked_drawn) {
            ili9341_fill(GREEN);
            ili9341_draw_string(20, 150, "UNLOCKED", WHITE, GREEN, 4);
            unlocked_drawn = 1;
        }
        } else {
            // Locked: set servo to closed position
            OCR1A = 2000; // 1 ms pulse for 0
        }

        

        
        if (key) {
            if (correct == 4) continue; // Ignore input after unlocked

            // '*' acts as backspace
            if (key == '*') {
                if (cur_col == 0 && cur_row == 0) continue; // Nothing to delete
                if (cur_col == 0) { cur_row--; cur_col = COLS - 1; }
                else { cur_col--; }
                // Erase the character by drawing a blank
                ili9341_draw_char(cur_col * CHAR_W, cur_row * CHAR_H,
                                ' ', WHITE, BLACK, CHAR_SCALE);
                continue;
            }

            // '#' clears the screen
            if (key == '#') {
                ili9341_fill(BLACK);
                cur_col = 0;
                cur_row = 0;
                continue;
            }

            // Draw the key at current position
            ili9341_draw_char(cur_col * CHAR_W, cur_row * CHAR_H,
                            '*', WHITE, BLACK, CHAR_SCALE);

            // Advance cursor
            cur_col++;
            if (cur_col >= COLS) {
                cur_col = 0;
                cur_row++;
            }

            // Scroll: if we hit the bottom, clear and start over
            if (cur_row >= ROWS) {
                ili9341_fill(BLACK);
                cur_col = 0;
                cur_row = 0;
            }

            buzzer_on();
            _delay_ms(1000);
            buzzer_off();


            if (key == pass[correct]) {
                for (int i = 0; i < 3; i++) {
                        buzzer_on();
                        _delay_ms(200);
                        buzzer_off();
                        _delay_ms(200);
                }
                correct++;
                if (correct == 4) {
                    for (int i = 0; i < 3; i++) {
                        buzzer_on();
                        _delay_ms(200);
                        buzzer_off();
                    }
                }
            } else {
                correct = 0; // Reset on wrong key
            }
        }
    }
}