#include <SPI.h>
#include <RF24.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Piny pre modul NRF24L01
#define CE_PIN 9
#define CS_PIN 10

// Veľkosť bufferu pre prijatú správu
#define BUFFER_SIZE 32

// Inicializácia modulu NRF24L01
RF24 radio(CE_PIN, CS_PIN);

// Adresa pre komunikáciu medzi zariadeniami
const uint64_t address = 0xF0F0F0F0E1LL;

// Inicializácia displeja OLED
#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

// Klávesnica na displeji
const char keyboard[4][10] = {
  {'1', '2', '3', '4', '5', '6', '7', '8', '9', '0'},
  {'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P'},
  {'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ' '},
  {'Z', 'X', 'C', 'V', 'B', 'N', 'M', '.', ',', '!'}
};

// Aktuálna vybraná položka na klávesnici
char selectedChar = ' ';

// Premenné pre tlačidlá
#define UP_BUTTON 2
#define DOWN_BUTTON 3
#define LEFT_BUTTON 4
#define RIGHT_BUTTON 5
#define ENTER_BUTTON 6

bool upButtonPressed = false;
bool downButtonPressed = false;
bool leftButtonPressed = false;
bool rightButtonPressed = false;
bool enterButtonPressed = false;

// Buffer pre prijatú správu
char receivedMessage[BUFFER_SIZE] = "";

void setup() {
  // Inicializácia sériovej komunikácie
  Serial.begin(9600);

  // Inicializácia modulu NRF24L01
  radio.begin();
  radio.openWritingPipe(address);
  radio.openReadingPipe(1, address);
  radio.setPALevel(RF24_PA_LOW);

  // Inicializácia displeja OLED
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.display();
  delay(2000);
  display.clearDisplay();
  
  // Inicializácia tlačidiel
  pinMode(UP_BUTTON, INPUT_PULLUP);
  pinMode(DOWN_BUTTON, INPUT_PULLUP);
  pinMode(LEFT_BUTTON, INPUT_PULLUP);
  pinMode(RIGHT_BUTTON, INPUT_PULLUP);
  pinMode(ENTER_BUTTON, INPUT_PULLUP);
}

void loop() {
  // Odoslanie a prijatie správy
  sendAndReceiveMessage();

  // Ovládanie klávesnice a odoslanie správy po stlačení tlačidla "Enter"
  if (enterButtonPressed) {
    enterButtonPressed = false;
    sendMessage();
  }

  // Ovládanie klávesnice
  if (upButtonPressed) {
    upButtonPressed = false;
    moveSelection(-1, 0);
  }
  if (downButtonPressed) {
    downButtonPressed = false;
    moveSelection(1, 0);
  }
  if (leftButtonPressed) {
    leftButtonPressed = false;
    moveSelection(0, -1);
  }
  if (rightButtonPressed) {
    rightButtonPressed = false;
    moveSelection(0, 1);
  }

  // Zobrazenie klávesnice na displeji
  displayKeyboard();
}

// Odoslanie a prijatie správy
void sendAndReceiveMessage() {
  if (radio.available()) {
    radio.read(receivedMessage, BUFFER_SIZE);
    Serial.println("Prijatá správa: " + String(receivedMessage));
    memset(receivedMessage, 0, BUFFER_SIZE);
  }
}

// Odoslanie správy
void sendMessage() {
  radio.stopListening();
  radio.write(selectedChar, sizeof(selectedChar));
  radio.startListening();
  Serial.println("Odoslaná správa: " + String(selectedChar));
  selectedChar = ' ';
}

// Ovládanie vybraného písmena na klávesnici
void moveSelection(int rowOffset, int colOffset) {
  int currentRow = -1;
  int currentCol = -1;

  // Nájdenie aktuálnej pozície vybraného písmena
  for (int row = 0; row < 4; row++) {
    for (int col = 0; col < 10; col++) {
      if (keyboard[row][col] == selectedChar) {
        currentRow = row;
        currentCol = col;
        break;
      }
    }
    if (currentRow != -1 && currentCol != -1) {
      break;
    }
  }

  // Zmena pozície vybraného písmena
  int newRow = (currentRow + rowOffset + 4) % 4;
  int newCol = (currentCol + colOffset + 10) % 10;
  selectedChar = keyboard[newRow][newCol];
}

// Zobrazenie klávesnice na displeji
void displayKeyboard() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  
  for (int row = 0; row < 4; row++) {
    for (int col = 0; col < 10; col++) {
      display.print(keyboard[row][col]);
      display.print(" ");
    }
    display.println();
  }

  display.println();
  display.print("Selected: ");
  display.print(selectedChar);

  display.display();
}

