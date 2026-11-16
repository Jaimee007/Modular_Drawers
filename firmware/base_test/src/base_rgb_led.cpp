// base_rgb_led: drive a common-anode RGB LED through three N-MOSFETs from the
// ESP32-C3 SuperMini, with PWM, serial commands and a limit-switch mode change.
// Env: base_rgb_led (pio run -e base_rgb_led -t upload)
// Doc: docs/07_pruebas_banco.md#base_rgb_led
//
// Wiring, one branch per colour (same circuit as the module PCB):
//   +V (4.5..6 V bench supply, later the USB-C 5 V rail) --> LED common anode
//   LED colour cathode --[R]--> MOSFET drain,  source --> GND,  gate <-- GPIO
//   gate --[100 kohm]--> GND      (keeps the colour off while the chip boots)
//   Bench supply GND, SuperMini GND and the MOSFET sources must be joined.
//   Limit switch: one side to SWITCH_PIN, the other to GND (internal pull-up).
//
// Logic is direct: GPIO high = MOSFET on = colour lit.
//
// Serial commands (115200, end each line with Enter):
//   r | g | b | w | off          one colour, white (all three) or off
//   rgb R G B                    PWM levels 0..255, e.g. "rgb 255 80 0"
//   fade                         slow R-G-B-W fade, any key stops it
//   mode N                       jump to mode N (see below)
//   help                         list the commands
//
// Limit switch: each press goes to the next mode.
//   0 off, 1 red, 2 green, 3 blue, 4 white, 5 PWM colour cycle
#include <Arduino.h>

// GPIOs not used by the final design: DATA is 20/21, SENSE 0/1/3/10, status LED 8.
#define PIN_R 4
#define PIN_G 5
#define PIN_B 7
#define SWITCH_PIN 6

// LEDC channels and PWM setup. 8 bits = 0..255 levels, 5 kHz is flicker-free.
#define PWM_FREQ 5000
#define PWM_BITS 8
static const uint8_t CH_R = 0, CH_G = 1, CH_B = 2;

#define DEBOUNCE_MS 25
#define MODE_COUNT 6

static uint8_t level[3];         // current R, G, B levels
static int mode = 0;
static bool switchStable = false; // true = pressed (pin low)
static bool switchRaw = false;
static uint32_t switchSince;
static bool cycling = false;      // modes with an animation running

static void setRGB(uint8_t r, uint8_t g, uint8_t b)
{
    level[0] = r;
    level[1] = g;
    level[2] = b;
    ledcWrite(CH_R, r);
    ledcWrite(CH_G, g);
    ledcWrite(CH_B, b);
}

static void printHelp()
{
    Serial.println("commands: r g b w off | rgb R G B (0-255) | fade | mode N (0-5) | help");
    Serial.println("limit switch: next mode (0 off, 1 red, 2 green, 3 blue, 4 white, 5 cycle)");
}

// Hue 0..255 -> RGB at full brightness, for the colour cycle.
static void hueToRGB(uint8_t h, uint8_t &r, uint8_t &g, uint8_t &b)
{
    uint8_t region = h / 43;
    uint8_t rem = (h - region * 43) * 6; // 0..255 inside the region
    switch (region) {
    case 0: r = 255; g = rem; b = 0; break;
    case 1: r = 255 - rem; g = 255; b = 0; break;
    case 2: r = 0; g = 255; b = rem; break;
    case 3: r = 0; g = 255 - rem; b = 255; break;
    case 4: r = rem; g = 0; b = 255; break;
    default: r = 255; g = 0; b = 255 - rem; break;
    }
}

static void applyMode(int m)
{
    mode = ((m % MODE_COUNT) + MODE_COUNT) % MODE_COUNT;
    cycling = (mode == 5);
    switch (mode) {
    case 0: setRGB(0, 0, 0); break;
    case 1: setRGB(255, 0, 0); break;
    case 2: setRGB(0, 255, 0); break;
    case 3: setRGB(0, 0, 255); break;
    case 4: setRGB(255, 255, 255); break;
    case 5: break; // the loop animates it
    }
    static const char *names[MODE_COUNT] = {"off", "red", "green", "blue", "white", "cycle"};
    Serial.printf("mode %d: %s\n", mode, names[mode]);
}

static void runFade()
{
    Serial.println("fade: send any key to stop");
    static const uint8_t colours[4][3] = {{255, 0, 0}, {0, 255, 0}, {0, 0, 255}, {255, 255, 255}};
    for (uint8_t c = 0; c < 4; c++) {
        for (int v = 0; v <= 255; v += 5) {
            setRGB(colours[c][0] * v / 255, colours[c][1] * v / 255, colours[c][2] * v / 255);
            delay(15);
            if (Serial.available()) goto done;
        }
        for (int v = 255; v >= 0; v -= 5) {
            setRGB(colours[c][0] * v / 255, colours[c][1] * v / 255, colours[c][2] * v / 255);
            delay(15);
            if (Serial.available()) goto done;
        }
    }
done:
    while (Serial.available()) Serial.read();
    setRGB(0, 0, 0);
    Serial.println("fade done");
}

static void handleLine(String line)
{
    line.trim();
    line.toLowerCase();
    if (line.length() == 0) return;
    cycling = false;

    if (line == "r") setRGB(255, 0, 0);
    else if (line == "g") setRGB(0, 255, 0);
    else if (line == "b") setRGB(0, 0, 255);
    else if (line == "w") setRGB(255, 255, 255);
    else if (line == "off") setRGB(0, 0, 0);
    else if (line == "fade") { runFade(); return; }
    else if (line == "help") { printHelp(); return; }
    else if (line.startsWith("mode ")) { applyMode(line.substring(5).toInt()); return; }
    else if (line.startsWith("rgb ")) {
        int r, g, b;
        if (sscanf(line.c_str() + 4, "%d %d %d", &r, &g, &b) != 3 ||
            r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255) {
            Serial.println("usage: rgb R G B   (each 0-255)");
            return;
        }
        setRGB(r, g, b);
    } else {
        Serial.printf("unknown command '%s' (type help)\n", line.c_str());
        return;
    }
    Serial.printf("RGB = %u %u %u\n", level[0], level[1], level[2]);
}

// Debounced limit switch: the state only changes after DEBOUNCE_MS stable.
static void pollSwitch()
{
    bool raw = (digitalRead(SWITCH_PIN) == LOW);
    uint32_t now = millis();
    if (raw != switchRaw) {
        switchRaw = raw;
        switchSince = now;
    } else if (raw != switchStable && now - switchSince >= DEBOUNCE_MS) {
        switchStable = raw;
        if (switchStable) { // act on press only, not on release
            Serial.println("switch pressed");
            applyMode(mode + 1);
        }
    }
}

void setup()
{
    Serial.begin(115200);
    delay(1500); // let the USB CDC port enumerate

    // Set the PWM pins low before the LEDC takes them over.
    const uint8_t pins[3] = {PIN_R, PIN_G, PIN_B};
    const uint8_t chans[3] = {CH_R, CH_G, CH_B};
    for (uint8_t i = 0; i < 3; i++) {
        pinMode(pins[i], OUTPUT);
        digitalWrite(pins[i], LOW);
        ledcSetup(chans[i], PWM_FREQ, PWM_BITS);
        ledcAttachPin(pins[i], chans[i]);
    }
    setRGB(0, 0, 0);

    pinMode(SWITCH_PIN, INPUT_PULLUP);
    switchRaw = switchStable = (digitalRead(SWITCH_PIN) == LOW);
    switchSince = millis();

    Serial.printf("base_rgb_led up: R=GPIO%d G=GPIO%d B=GPIO%d switch=GPIO%d\n", PIN_R, PIN_G,
                  PIN_B, SWITCH_PIN);
    printHelp();
}

void loop()
{
    static String line;
    while (Serial.available()) {
        char c = (char)Serial.read();
        if (c == '\n' || c == '\r') {
            handleLine(line);
            line = "";
        } else if (line.length() < 40) {
            line += c;
        }
    }

    pollSwitch();

    if (cycling) {
        static uint8_t hue;
        static uint32_t last;
        if (millis() - last >= 20) {
            last = millis();
            uint8_t r, g, b;
            hueToRGB(hue++, r, g, b);
            setRGB(r, g, b);
        }
    }
}
