#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <time.h>
#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

#define SDA 3
#define SCL 17

// Wi-Fi credentials
const char* ssid = "mo";
const char* password = "p5338z1wspy8d";

// MQTT broker IP
const char* mqtt_server = "35.233.231.192";

WiFiClient espClient;
PubSubClient mqttClient(espClient);
LiquidCrystal_I2C lcd(0x27, 16, 2);

char game[3][3] = {{'_','_','_'},
                    {'_','_','_'},
                    {'_','_','_'}};

int count = 100;
int aiPlayer = 0;
int aiPlayer2 = 0;
int player1 = 0;
int player2 = 0;
char player;
bool isPlaying = false;
bool isPlaying2 = false;

//Checks for a winner between X and O
char checkForWinner(char grid[3][3])
{
  int countBlank = 0;
  for(int i = 0; i < 3; i++)
  {
    for(int y = 0; y < 3; y++)
    {
      //Checks we are at the middle point
      if(i == 1 && y == 1 && grid[i][y] != '_')
      {
        if((grid[i - 1][y] == grid[i][y] && grid[i + 1][y] == grid[i][y]) || 
           (grid[i][y - 1] == grid[i][y] && grid[i][y + 1] == grid[i][y]) ||
           (grid[i + 1][y - 1] == grid[i][y] && grid[i - 1][y + 1] == grid[i][y]))
           {
              return grid[i][y];
           }
      }
      else
      {
        //Checks if we are at the middle of i index
        if(i == 1 && grid[i][y] != '_')
        {
          //Checks if it is a winner
          if(grid[i - 1][y] == grid[i][y] && grid[i + 1][y] == grid[i][y])
          {
            return grid[i][y];
          }
        }
        //Checks if we are on the middle of the y point
        if(y == 1 && grid[i][y] != '_')
        {
          if(grid[i][y - 1] == grid[i][y] && grid[i][y + 1] == grid[i][y])
          {
            return grid[i][y];
          }
        }
      }
    }
  }

  for(int i = 0; i < 3; i++)
  {
    for(int y = 0; y < 3; y++)
    {
      if(grid[i][y] != '_')
      {
        countBlank++;
      }
    }
  }
  //No Winners yet
  return countBlank == 9 ? 'b' : ' ';
}

void reset(char game[3][3])
{
  for(int i = 0; i < 3; i++)
  {
    for(int y = 0; y < 3; y++)
    {
      game[i][y] = '_';
    }
  }
}

void outputGrid(char game[3][3])
{
  for(int i = 0; i < 3; i++)
  {
    for(int y = 0; y < 3; y++)
    {
      Serial.print(game[i][y]);
      Serial.print(" ");
    }
    Serial.println();
  }
}

void chooseMenu(char*topic, byte* payload, unsigned int length) {
  char option = (char)payload[0];
  String topicStr = String(topic);
  
  //Gets the messages from player1 and player2
  char text[128];  // Make sure the buffer is large enough for your expected payload size
  text[0] = '\0';  // Initialize empty string

  for (int i = 0; i < length; i++) {
    char c[2];
    c[0] = (char)payload[i];  // Convert byte to char
    c[1] = '\0';              // Null-terminate
    strcat(text, c);          // Append to the text string
  }

  if((char)payload[0] == '0' && strlen(text) == 1)
  {
  
      mqttClient.subscribe("esp32/aiRound");
      isPlaying = false;
      isPlaying2 = false;

      //Sets the Grid
      lcd.begin(16, 2);  // 16 columns, 2 rows
      lcd.clear();
      lcd.setCursor(0, 0);  // Column 0, Row 0
      lcd.print("A1: ");
      lcd.print(aiPlayer);

      lcd.setCursor(0, 1);  // Column 0, Row 1
      lcd.print("A2: ");
      lcd.print(aiPlayer2);
  }
  else if((char)payload[0] == '1' && strlen(text) == 1)
  {
      mqttClient.subscribe("esp32/players");
      isPlaying = true;
      isPlaying2 = true;
      player = 'X';

      //Sets the Grid
      mqttClient.subscribe("esp32/aiVsPlayer");
      lcd.begin(16, 2);  // 16 columns, 2 rows
      lcd.clear();
      lcd.setCursor(0, 0);  // Column 0, Row 0
      lcd.print("P1: ");
      lcd.print(aiPlayer);

      lcd.setCursor(0, 1);  // Column 0, Row 1
      lcd.print("P2: ");
      lcd.print(aiPlayer);
  }
  else if((char)payload[0] == '2' && strlen(text) == 1)
  {

    isPlaying = true;
    isPlaying = false;

      //Sets the Grid
      mqttClient.subscribe("esp32/aiVsPlayer");
      lcd.begin(16, 2);  // 16 columns, 2 rows
      lcd.clear();
      lcd.setCursor(0, 0);  // Column 0, Row 0
      lcd.print("P1: ");
      lcd.print(aiPlayer);

      lcd.setCursor(0, 1);  // Column 0, Row 1
      lcd.print("A1: ");
      lcd.print(aiPlayer);
      
  }
  
  if((topicStr == "esp32/aiRound" || topicStr == "esp32/aiRound2") && !isPlaying && !isPlaying2)
  {

      //Gets the integers
      char *token;
      int num1, num2;

      token = strtok(text, ",");
      if (token != NULL) {
          num1 = atoi(token);  // Convert first number
      }

      token = strtok(NULL, ",");
      if (token != NULL) {
          num2 = atoi(token);  // Convert second number
      }
    
      if(game[num1][num2] == '_')
      {
        if(topicStr == "esp32/aiRound")
        {
          game[num1][num2] = 'X';
          mqttClient.unsubscribe("esp32/aiRound");
          mqttClient.subscribe("esp32/aiRound2");
        }
        else
        {
          game[num1][num2] = 'O';
          mqttClient.unsubscribe("esp32/aiRound2");
          mqttClient.subscribe("esp32/aiRound");
        }
      }
      outputGrid(game);
      char winner = checkForWinner(game);
      Serial.println(winner);
      if(winner != ' ')
      {
        if(winner == 'X')
        {
          aiPlayer++;
          Serial.println("X wins: ");
        }
        else if(winner == 'O')
        {
          aiPlayer2++;
          Serial.println("O wins: ");
        }
        else
        {
          Serial.println("DRAW!");
        }
        reset(game);
        count--;
        lcd.clear();
        lcd.setCursor(0, 0);  // Column 0, Row 0
        lcd.print("A1: ");
        lcd.print(aiPlayer);

        lcd.setCursor(0, 1);  // Column 0, Row 1
        lcd.print("A2: ");
        lcd.print(aiPlayer2);
      }

      if(count == 0)
      {
        mqttClient.unsubscribe("esp32/aiRound2");
        mqttClient.unsubscribe("esp32/aiRound");
        count = 100;
        aiPlayer = 0;
        aiPlayer2 = 0;
      }
               
  }
  else if(topicStr == "esp32/players" && isPlaying && isPlaying2)
  {
       //Gets the integers
      char *token;
      int num1, num2;

      token = strtok(text, ",");
      if (token != NULL) {
          num1 = atoi(token);  // Convert first number
      }

      token = strtok(NULL, ",");
      if (token != NULL) {
          num2 = atoi(token);  // Convert second number
      }

       if(game[num1][num2] == '_')
      {
       
        if(player == 'X')
        {
          game[num1][num2] = 'X';
          player = 'O';
        }
        else
        {
           game[num1][num2] = 'O';
           player = 'X';
        }

        char winner = checkForWinner(game);
        Serial.println(winner);
        if(winner != ' ')
        {
          if(winner == 'X')
          {
            player1++;
            Serial.println("X wins:");
          }
          else if(winner == 'O')
          {
            player2++;
            Serial.println("O wins:");
          }
          else
          {
            Serial.println("DRAW!");
          }
          reset(game);

          count--;
          lcd.clear();
          lcd.setCursor(0, 0);  // Column 0, Row 0
          lcd.print("P1: ");
          lcd.print(player1);

          lcd.setCursor(0, 1);  // Column 0, Row 1
          lcd.print("P2: ");
          lcd.print(player2);

  }
  
  else if((topicStr == "esp32/aiVsPlayer" || topicStr == "esp32/aiRound") && isPlaying && !isPlaying2)
  {
    outputGrid(game);
    
      //Gets the integers
      char *token;
      int num1, num2;

      token = strtok(text, ",");
      if (token != NULL) {
          num1 = atoi(token);  // Convert first number
      }

      token = strtok(NULL, ",");
      if (token != NULL) {
          num2 = atoi(token);  // Convert second number
      }

      if(game[num1][num2] == '_')
      {
       
        if(topicStr == "esp32/aiVsPlayer")
        {
          game[num1][num2] = 'X';
          mqttClient.unsubscribe("esp32/aiVsPlayer");
          mqttClient.subscribe("esp32/aiRound");
        }
        else if(topicStr == "esp32/aiRound")
        {
           game[num1][num2] = 'O';
           mqttClient.subscribe("esp32/aiVsPlayer");
           mqttClient.unsubscribe("esp32/aiRound");
        }

        char winner = checkForWinner(game);
        Serial.println(winner);
        if(winner != ' ')
        {
          if(winner == 'X')
          {
            player1++;
            Serial.println("X wins:");
          }
          else if(winner == 'O')
          {
            aiPlayer++;
            Serial.println("O wins:");
          }
          else
          {
            Serial.println("DRAW!");
          }
          reset(game);

          count--;
          lcd.clear();
          lcd.setCursor(0, 0);  // Column 0, Row 0
          lcd.print("P1: ");
          lcd.print(player1);

          lcd.setCursor(0, 1);  // Column 0, Row 1
          lcd.print("AI: ");
          lcd.print(aiPlayer);
        }
        outputGrid(game);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  String topicStr = String(topic);
  
  chooseMenu(topic, payload, length);
}

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA, SCL);
  WiFi.begin(ssid, password);
  srand(time(NULL)); // Ensure different random values each run

  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");

  mqttClient.setServer(mqtt_server, 1883);
  mqttClient.setCallback(callback);

  // Switch to alternative LCD address if 0x27 not detected
  if (!i2CAddrTest(0x27)) {
    lcd = LiquidCrystal_I2C(0x3F, 16, 2);
  }
}

void reconnect() {
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (mqttClient.connect("ESP32Client")) {
      Serial.println("connected");
      mqttClient.subscribe("esp32/liquid");
      
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      delay(5000);
    }
  }
}


void loop() {
  if (!mqttClient.connected()) {
    reconnect();
  }
  mqttClient.loop();
}

bool i2CAddrTest(uint8_t addr) {
  Wire.beginTransmission(addr);
  return Wire.endTransmission() == 0;
}
