#include "ThingSpeak.h"
#include "WiFiEsp.h"
#include "secrets.h"

char ssid[] = SECRET_SSID;   
char pass[] = SECRET_PASS;   
WiFiEspClient  client;


#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
SoftwareSerial Serial1(10, 11); // RX, TX
#define ESP_BAUDRATE  19200
#else
#define ESP_BAUDRATE  115200
#endif

unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;

// Initialize our values

int mq2Val = random(100, 1000);
//int fume = random(100, 1000);
int pirVal = random(100, 1000);

const int  ledSalon = 13, ledEntre = 12, buzzer = 7, fan = 9 , mq2 = A0, pir = 5;
const int threshold = 1000;
void setup() {
  //initialise tout les composant et la vitesse de transmission 
  initialiser();

  // initialise la vitesse de transmission
  setEspBaudRate(ESP_BAUDRATE);

  /*
    while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo native USB port only
    }*/

  Serial.print("Recherche de ESP8266...");
  // initialise 
  WiFi.init(&Serial1);

  // cherche la presence de l'ESP
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("ESP8266 introuvable !");
    // don't continue
    while (true);
  }
  Serial.println("ESP8266 trouvé!");

  ThingSpeak.begin(client);  // Initialise ThingSpeak
}

void loop() {

  // Connexion au wifi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Connexion au Wifi ");
    Serial.println(SECRET_SSID);
    while (WiFi.status() != WL_CONNECTED) {
      WiFi.begin(ssid, pass);  
      Serial.print(".");
      delay(5000);
    }
    Serial.println("\nWIFI CONNECTÉ.");
  }

  // set the fields with the values
  ThingSpeak.setField(1, gaz);
  ThingSpeak.setField(2, fume);
  ThingSpeak.setField(3, presence);

  // envoie des données vers ThingSpeak
  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if (x == 200) {
    Serial.println("Canal MAJ  avec succéss.");
  }
  else {
    Serial.println("Probléme de MAJ . HTTP error code " + String(x));
  }

  // maj des valeurs 
  setSensorValue();

  delay(20000); 
  // Wait 20 seconds to update the channel again
}

// Cette fonction tente de définir la vitesse de transmission de l'ESP8266. 
// peut utiliser 115200, sinon la série de logiciels est limitée à 19200.
void setEspBaudRate(unsigned long baudrate) {
  long rates[6] = {115200, 74880, 57600, 38400, 19200, 9600};

  Serial.print("Setting ESP8266 baudrate to ");
  Serial.print(baudrate);
  Serial.println("...");

  for (int i = 0; i < 6; i++) {
    Serial1.begin(rates[i]);
    delay(100);
    Serial1.print("AT+UART_DEF=");
    Serial1.print(baudrate);
    Serial1.print(",8,1,0,0\r\n");
    delay(100);
  }

  Serial1.begin(baudrate);
}

void initialiser() {
  pinMode(ledSalon, OUTPUT);
  pinMode(ledEntre, OUTPUT);
  pinMode(fan, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(pir, INPUT);
  pinMode(mq2, INPUT);
  digitalWrite(whiteLed, HIGH);
  Serial.begin(9600);
  Serial.println("init pins");

}

void sos() {
  digitalWrite(fan, HIGH);
  digitalWrite(ledSalon, HIGH);
  tone(buzzer, 1000);
  delay(1000);
  noTone(buzzer);
  digitalWrite(ledSalon, LOW);
  delay(1000);
}

void setSensorValue() {
  mq2Val = analogRead(mq2);
  pirVal = digitalRead(pir);
  if (mq2Val >= seuil)
  {
    //Serial.print(" | fumée détectée !");
    sos();

  }
  if (pirVal == HIGH) {
    digitalWrite(ledEntre, HIGH);
    //Serial.println("mouvement détecté ! ");
  } else if  (pirVal == LOW) {
    digitalWrite(ledEntre, LOW);

  }


}
