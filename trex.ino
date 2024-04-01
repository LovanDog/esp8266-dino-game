#include <ESP8266WiFi.h>
#include <SSD1306.h>

#define OLED_SDA  D1
#define OLED_SCL  D2
#define OLED_ADDR 0x3C

SSD1306 display(OLED_ADDR, OLED_SDA, OLED_SCL);

#define FLASH_BUTTON 0  // Flash button pin
#define RESET_BUTTON 16 // Reset button pin

// Dinosaur position
int dinoX = 20;
int dinoY = 30;

// Dinosaur size
int dinoWidth = 10;
int dinoHeight = 10;

// Gravity
float gravity = 0.3;
float velocity = 0;
float jumpVelocity = -3; // Adjust this value for lower/higher jumps

// Game state
bool isJumping = false;
bool isGameOver = false;

// Obstacle properties
int obstacleWidth = 10;
int obstacleHeight = 10;
int obstacleSpeed = 3;
int obstacleX = 128;
int obstacleY = 30;

// Score
int score = 0;
int x = 123;

const unsigned char epd_bitmap_output_onlinepngtools [] PROGMEM = {
	0x07, 0xc0, 0x07, 0x40, 0x07, 0xc0, 0x8f, 0xc0, 0xff, 0x00, 0xfe, 0x00, 0x7f, 0x00, 0x3c, 0x00, 
	0x14, 0x00, 0x1a, 0x00
};


void setup() {
  pinMode(FLASH_BUTTON, INPUT_PULLUP);
  pinMode(RESET_BUTTON, INPUT_PULLUP);
  
  Serial.begin(115200);

  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  display.clear();
  display.display();
}

void loop() {
  // Clear the display
  display.clear();
  
  // Check for game over
  if (!isGameOver) {
    // Check for jump action
    if (digitalRead(FLASH_BUTTON) == LOW && !isJumping) {
      isJumping = true;
      velocity = jumpVelocity; // Jump velocity
    }
    
    // Apply gravity
    if (isJumping) {
      velocity += gravity;
      dinoY += velocity;
      
      // Check for ground
      if (dinoY >= 30) {
        dinoY = 30;
        velocity = 0;
        isJumping = false;
      }
    }
    
    // Move the obstacle
    obstacleX -= obstacleSpeed;
    if (obstacleX < -obstacleWidth) {
      obstacleWidth = random(5, 15);
      obstacleX = random(150, 200); // Generate obstacle at random position
      score++; // Increment score when a new obstacle appears
    }
    
    // Check for collision with the obstacle
    if (dinoX + dinoWidth >= obstacleX && dinoX <= obstacleX + obstacleWidth &&
        dinoY + dinoHeight >= obstacleY && dinoY <= obstacleY + obstacleHeight) {
      isGameOver = true;
    }
  }
  
  // Draw the dinosaur
  display.fillRect(dinoX, dinoY, dinoWidth, dinoHeight);
  
  // Draw the obstacle
  display.fillRect(obstacleX, obstacleY, obstacleWidth, obstacleHeight);
  
  // Draw the ground
  display.drawLine(0, 40, 128, 40);
  
  // Draw the score
  if (score < 10) {
    x = 123;
  } else if (score > 9) {
    x = 117;
  }
  display.drawString(x, 0, String(score));
  
  // Game over
  if (isGameOver) {
    display.drawString(0, 0, "Lose. Press rst to restart");
  }
  
  // Update the display
  display.display();
  
  // Check for reset button press to restart the game
  if (digitalRead(RESET_BUTTON) == LOW) {
    resetGame();
  }
  
  // Delay for smooth rendering
  delay(50);
}

// Function to reset the game
void resetGame() {
  isGameOver = false;
  dinoY = 30;
  velocity = 0;
  obstacleX = 128;
  score = 0;
}
