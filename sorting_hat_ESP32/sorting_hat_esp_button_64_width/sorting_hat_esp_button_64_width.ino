#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "sorting_hat_model.h"  // this is our ML model file (auto-generated)

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// I2C pins for the OLED screen (based on how we wired it)
#define SDA_PIN 8
#define SCL_PIN 9

// Button pins (connected to D2 ~ D5)
const int buttonPins[4] = {2, 3, 4, 5};

// Question text
const char* questions[] = {
    "1. What do you value?",
    "2. What to do if someone cheats?",
    "3. Favorite subject?",
    "4. How do you face challenges?",
    "5. How do friends describe you?",
    "6. What to do with a mystery book?",
    "7. Preferred pet?",
    "8. How do you solve problems?",
    "9. What kind of friends do you like?",
    "10. Dream career?"
};

// Each question has 4 options (A to D)
const char* options[][4] = {
    {"A) Bravery", "B) Loyalty", "C) Intelligence", "D) Ambition"},
    {"A) Call them out", "B) Let them be", "C) Inform teacher", "D) Gain from it"},
    {"A) Defense Arts", "B) Herbology", "C) Charms", "D) Potions"},
    {"A) Face head-on", "B) Team up", "C) Plan first", "D) Outsmart it"},
    {"A) Bold", "B) Kind", "C) Smart", "D) Resourceful"},
    {"A) Read it now", "B) Check safety", "C) Study it", "D) Use for gain"},
    {"A) Owl", "B) Toad", "C) Cat", "D) Phoenix"},
    {"A) Act fast", "B) Find a compromise", "C) Analyze first", "D) Outsmart"},
    {"A) Adventurous", "B) Loyal", "C) Thoughtful", "D) Powerful"},
    {"A) Auror", "B) Healer", "C) Scholar", "D) Minister"}
};

// Store answers (1-4 for A-D)
int responses[10] = {0};
int questionIndex = 0;  // keep track of which question we’re on

Eloquent::ML::Port::DecisionTree clf;  // this is our actual decision tree model

void setup() {
  Serial.begin(115200); // print stuff to computer
  Wire.begin(SDA_PIN, SCL_PIN);  // setup the screen pins

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED init failed :(");
    while (true); // freeze forever
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 10);
  display.println("Sorting Hat Ready!");
  display.display();
  delay(1500); // wait a bit so we can see it

  // make buttons work (pull-up so LOW = pressed)
  for (int i = 0; i < 4; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
  }

  showQuestion(); // show first question
}

void loop() {
  // check if any button was pressed
  for (int i = 0; i < 4; i++) {
    if (digitalRead(buttonPins[i]) == LOW) {
      delay(200); // debounce
      responses[questionIndex] = i + 1; // A = 1, B = 2, etc.
      Serial.print("Q");
      Serial.print(questionIndex + 1);
      Serial.print(" answered: ");
      Serial.println(i + 1);

      questionIndex++;
      if (questionIndex >= 10) {
        display.clearDisplay();
        display.setCursor(0, 10);
        display.println("All answered!");
        display.display();

        classifyHouse(); // run the ML model!
        delay(10000);    // chill and let people see result
        ESP.restart();   // start over
      } else {
        showQuestion(); // next one
      }

      break; // only handle one press at a time
    }
  }
}

void showQuestion() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(questions[questionIndex]);
  for (int i = 0; i < 4; i++) {
    display.setCursor(10, 16 + i * 10);
    display.println(options[questionIndex][i]);
  }
  display.display();
}

void classifyHouse() {
  float features[10];
  for (int i = 0; i < 10; i++) {
    features[i] = (float)responses[i]; // copy answers to float array
  }

  int prediction = clf.predict(features); // let model guess

  const char* house = "";
  switch (prediction) {
    case 0: house = "Gryffindor"; break;
    case 1: house = "Hufflepuff"; break;
    case 2: house = "Ravenclaw";  break;
    case 3: house = "Slytherin";  break;
    default: house = "Unknown";  break;
  }

  display.clearDisplay();
  display.setCursor(0, 10);
  display.println("You belong to...");
  display.setTextSize(2);
  display.setCursor(0, 30);
  display.println(house);
  display.display();

  Serial.println("Sorting complete!");
  Serial.print("Predicted House: ");
  Serial.println(house);
}
