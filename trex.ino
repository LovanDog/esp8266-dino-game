#include <ESP8266WiFi.h>
#include <SSD1306.h>
#include <EEPROM.h>

#define OLED_SDA  D1
#define OLED_SCL  D2
#define OLED_ADDR 0x3C

SSD1306 display(OLED_ADDR, OLED_SDA, OLED_SCL);

// dino pos
int dinoX = 20;
int dinoY = 30;

// dino size
int dinoWidth = 10;
int dinoHeight = 10;

// gravity
float gravity = 0.3;
float velocity = 0;
float jumpVelocity = -3; // lower/higher jumps

// game state
bool isJumping = false;
enum GameState {
  START_SCREEN,
  PLAYING,
  GAME_OVER
};

// obstacle
int obstacleWidth = 10;
int obstacleHeight = 10;
int obstacleSpeed = 3;
int obstacleX = 128;
int obstacleY = 30;

// score/speed/
int currentScore = 0;
int bestScore = 0;
int gameSpeed = 40;

int x = 123; // int for score x distance 

// level up arrow
bool showArrow = false;
unsigned long arrowStartTime;

GameState gameState = START_SCREEN;

void setup() {
  pinMode(0, INPUT_PULLUP); // flash
  pinMode(16, INPUT_PULLUP); // reset
  
  Serial.begin(115200);

  EEPROM.begin(sizeof(bestScore));
  EEPROM.get(0, bestScore);

  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  display.clear();
  display.display();
}

void loop() {
  unsigned long loopStartTime = millis();

  display.clear();

  if (showArrow) {
    display.drawString(0, 0, "^^");
    if (millis() - arrowStartTime >= 3000) {
      showArrow = false;
    }
  }

  switch (gameState) {
    case START_SCREEN:
      display.clear();
      display.setFont(ArialMT_Plain_16);
      display.drawString(0, 0, "PRESS FLASH");
      display.setFont(ArialMT_Plain_10);
      display.drawString(0, 20, "to start");
      display.display();
      
      if (digitalRead(FLASH_BUTTON) == LOW) {
        gameState = PLAYING;
      }
      break;
      
    case PLAYING:
      handleInput();
      updateGame();
      renderGame();

      display.display();
      break;
      
    case GAME_OVER:
      handleGameOver();
      break;
  }

  display.display();
  maintainFrameRate(loopStartTime);
}

void handleInput() {
  if (digitalRead(FLASH_BUTTON) == LOW && !isJumping) {
    isJumping = true;
    velocity = jumpVelocity;
  }
}

void updateGame() {
  // gravity
  if (isJumping) {
    velocity += gravity;
    dinoY += velocity;

    // ground
    if (dinoY >= 30) {
      dinoY = 30;
      velocity = 0;
      isJumping = false;
    }
  }

  obstacleX -= obstacleSpeed;
  if (obstacleX < -obstacleWidth) {
    obstacleWidth = random(5, 17);
    obstacleX = random(150, 200);
    currentScore++;

    if (currentScore % 10 == 0 && gameSpeed > 10) {
      gameSpeed -= 5;
      showArrow = true;
      arrowStartTime = millis();
    }
  }

  if (dinoX + dinoWidth >= obstacleX && dinoX <= obstacleX + obstacleWidth &&
      dinoY + dinoHeight >= obstacleY && dinoY <= obstacleY + obstacleHeight) {
    gameState = GAME_OVER;
  }
}

void renderGame() {
  // dino
  display.fillRect(dinoX, dinoY, dinoWidth, dinoHeight);
  
  // obstacle
  display.fillRect(obstacleX, obstacleY, obstacleWidth, obstacleHeight);
  
  // ground
  display.drawLine(0, 40, 128, 40);
  
  // score
  int x = (currentScore < 10) ? 123 : 117;
  display.drawString(x, 0, String(currentScore));
}

void handleGameOver() {
  if (currentScore > bestScore) {
    bestScore = currentScore;
    EEPROM.put(0, bestScore);
    EEPROM.commit();
  }

  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 0, "Game Over");
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 20, "Best: " + String(bestScore));

  display.display();

  resetGame();
}

void maintainFrameRate(unsigned long loopStartTime) {
  unsigned long loopEndTime = millis();
  if (loopEndTime - loopStartTime < gameSpeed) {
    delay(gameSpeed - (loopEndTime - loopStartTime));
  }
}


void resetGame() {
  dinoY = 30;
  velocity = 0;
  obstacleX = 128;
  currentScore = 0;
  gameSpeed = 50;
}
