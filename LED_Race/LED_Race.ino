#include "FastLED.h"
#define NUM_LEDS 300
#define LED_PIN 6

#define PLAYER1_PIN 7
#define PLAYER2_PIN 8

#define ACCELERATION 0.4
#define FRICTION 0.03
#define HIGHSPEED 3
#define MAX_LOOPS 2

//#define COLOR_PETROL 0x007575
//#define COLOR_LINKEDIN 0x0077B5

CRGB leds[NUM_LEDS];

struct Player {
  int buttonPin;
  int buttonState;
  int prevPosition;
  int position;
  int loop;
  float speed;
  bool isWinner;
  CRGB::HTMLColorCode color;
  CRGB::HTMLColorCode highSpeedColor;
};

struct Player player1;
struct Player player2;

int raceCountdown = 3;
bool raceFinished = false;

void setup() {
  initPlayer(&player1, PLAYER1_PIN, CRGB::Red, CRGB::Green);
  initPlayer(&player2, PLAYER2_PIN, CRGB::Blue, CRGB::Yellow);

  FastLED.addLeds < NEOPIXEL, LED_PIN > (leds, NUM_LEDS);

  clearTrack();

  Serial.begin(9600);
}

void loop() {
  if (raceCountdown >= 0) {
    delay(random(500, 1001));
    drawCountdown(raceCountdown);
    raceCountdown--;
    return;
  }
  
  if (raceFinished) {
    delay(1000);
    return;
  }

  movePlayer(&player1);
  movePlayer(&player2);
  drawPlayer(player1);
  drawPlayer(player2);

  if (isRaceFinished()) {
    struct Player winner = findWinner(player1, player2);
    drawWinner(winner);
    raceFinished = true;
  }

  delay(15);
}

void initPlayer(struct Player *player, int pin, CRGB::HTMLColorCode color, CRGB::HTMLColorCode highSpeedColor) {
  player->buttonPin = pin;
  player->buttonState = LOW;
  player->prevPosition = 0;
  player->position = 0;
  player->loop = 0;
  player->speed = 0.0;
  player->isWinner = false;
  player->color = color;
  player->highSpeedColor = highSpeedColor;
}

bool buttonReleased(struct Player *player) {
  int previousState = player->buttonState;
  player->buttonState = digitalRead(player->buttonPin);

  //  Serial.println(player.buttonPin);

  if (previousState == HIGH && player->buttonState == LOW) {
    return true;
  }

  return false;
}

void movePlayer(struct Player *player) {
  if (buttonReleased(player)) {
    player->speed += ACCELERATION; // acceleration
  }

  player->speed -= player->speed * FRICTION;

  if (player->speed < 0) {
    player->speed = 0;
  }

  player->prevPosition = player->position;
  player->position = (int)(player->position + player->speed) % NUM_LEDS;

  if (player->position < player->prevPosition) {
    player->loop += 1;
  }

  if (player->loop == MAX_LOOPS) {
    player->isWinner = true;
  }
  //  Serial.println("Speed: " + (String)player->speed + " Position: " + (String)player->position + " P1: " + (String)player1.loop + " P2: " + (String)player2.loop);
}

void drawCountdown(int countdownStage) {
  switch (countdownStage) {
    case 3:
      setTrackColor(CRGB::Blue);
      break;
    case 2:
      setTrackColor(CRGB::Red);
      break;
    case 1:
      setTrackColor(CRGB::Green);
      break;
    default:
      clearTrack();
  }
}

void drawPlayer(struct Player player) {
  if (player.position != player.prevPosition || (player.position == 0 && player.loop == 0)) {
    for (int j = 0; j < player.loop + 1; j++) {
      int index = modulo(player.prevPosition - j, NUM_LEDS);
      leds[index] = CRGB::Black;
      //      Serial.println("Black " + (String)index);
    }

    for (int j = 0; j < player.loop + 1; j++) {
      int index = modulo(player.position - j, NUM_LEDS);
      leds[index] = player.speed > HIGHSPEED ? player.highSpeedColor : player.color;
      //      Serial.println("LED " + (String)index);
    }

    FastLED.show();
  }
}

// see also
// Copy ten led colors from leds[src .. src+9] to leds[dest .. dest+9]
// memmove( &leds[dest], &leds[src], 10 * sizeof( CRGB) );

bool isRaceFinished() {
  return player1.isWinner || player2.isWinner;
}

struct Player findWinner(struct Player player1, struct Player player2) {
  if (player1.isWinner) {
    return player1;
  } else if (player2.isWinner) {
    return player2;
  }
}

void drawWinner(struct Player winner) {
  setTrackColor(winner.color);
}

void clearTrack() {
  setTrackColor(CRGB::Black);
}

void setTrackColor(CRGB::HTMLColorCode color) {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = color;
  }
  FastLED.show();
}

// utilities
int modulo(int x, int n) {
  return (x % n + n) % n;
}
