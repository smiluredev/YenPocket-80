#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET    -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define BTN_PIN 2

// Splash screen config
unsigned long splashStart = 0;
const unsigned long splashDuration = 10000; // 10 segundos

// Estados do sistema
enum State {
  SPLASH,
  MENU,
  GAME,
  GAMEOVER
};

State currentState = SPLASH;

// Dino Game config
int playerX = 10;
int playerY = 24; // chão
int playerVY = 0;
bool isJumping = false;

unsigned long lastFrameTime = 0;
const unsigned long frameInterval = 50; // 20 FPS

int obstacleX = SCREEN_WIDTH;
const int obstacleY = 24;
const int obstacleWidth = 8;
const int obstacleHeight = 8;

bool gameOverFlag = false;
int score = 0;

// Para debouncing do botão
bool lastButtonState = HIGH;
bool buttonPressed = false;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

void setup() {
  pinMode(BTN_PIN, INPUT_PULLUP);
  Wire.begin();

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    while(1);
  }

  display.clearDisplay();
  splashStart = millis();
  lastFrameTime = millis();
}

void drawSplash() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  const char *text = "== YenPocket ==";
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
  int16_t x = (SCREEN_WIDTH - w) / 2;
  int16_t y = (SCREEN_HEIGHT - h) / 2;

  display.setCursor(x, y);
  display.println(text);
  display.display();
}

void drawMenu() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(20, 12);
  display.println("Press BTN to Start");

  display.display();
}

void startGame() {
  playerX = 10;
  playerY = 24;
  playerVY = 0;
  isJumping = false;

  obstacleX = SCREEN_WIDTH;
  score = 0;
  gameOverFlag = false;

  currentState = GAME;
}

void drawGame() {
  display.clearDisplay();

  // Chão
  display.drawLine(0, 31, SCREEN_WIDTH, 31, SSD1306_WHITE);

  // Player
  display.fillRect(playerX, playerY - 8, 8, 8, SSD1306_WHITE);

  // Obstáculo
  display.fillRect(obstacleX, obstacleY - obstacleHeight, obstacleWidth, obstacleHeight, SSD1306_WHITE);

  // Score
  display.setTextSize(1);
  display.setCursor(SCREEN_WIDTH - 40, 0);
  display.print("Score: ");
  display.print(score);

  display.display();
}

void drawGameOver() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(20, 10);
  display.println("GAME OVER!");

  display.setCursor(20, 20);
  display.print("Score: ");
  display.println(score);

  display.setCursor(0, 30);
  display.println("Press BTN to restart");

  display.display();
}

// Função para debouncing e leitura única do botão
bool readButton() {
  bool reading = digitalRead(BTN_PIN);

  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading == LOW && !buttonPressed) {
      buttonPressed = true;
      lastButtonState = reading;
      return true;
    }
    if (reading == HIGH) {
      buttonPressed = false;
    }
  }
  lastButtonState = reading;
  return false;
}

void loop() {
  unsigned long now = millis();

  switch (currentState) {
    case SPLASH:
      drawSplash();
      if (now - splashStart >= splashDuration) {
        currentState = MENU;
      }
      break;

    case MENU:
      drawMenu();
      if (readButton()) {
        startGame();
      }
      break;

    case GAME:
      if (now - lastFrameTime < frameInterval) return;
      lastFrameTime = now;

      // Pulo
      if (readButton() && !isJumping) {
        isJumping = true;
        playerVY = -6;
      }

      if (isJumping) {
        playerY += playerVY;
        playerVY += 1;
        if (playerY >= 24) { ẞ
          playerY = 24;
          playerVY = 0;
          isJumping = false;
        }
      }

      obstacleX -= 3;
      if (obstacleX < -obstacleWidth) {
        obstacleX = SCREEN_WIDTH;
        score++;
      }

      // Colisão
      if (playerX + 8 > obstacleX && playerX < obstacleX + obstacleWidth) {
        if (playerY >= obstacleY - obstacleHeight) {
          gameOverFlag = true;
          currentState = GAMEOVER;
        }
      }

      drawGame();
      break;

    case GAMEOVER:
      drawGameOver();
      if (readButton()) {
        startGame();
      }
      break;
  }
}
 
