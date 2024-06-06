#include <Wire.h>
#include <SPI.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define BUTTON_PIN 23
#define PhotoR_PIN 36
#define LED_PIN LED_BUILTIN
#define MAX_WAIT_FOR_TIMER 5
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     16 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


const char* password =      "youba123";
const char* ssid =          "iPhone de Youba";

const char* mqttServer =    "172.20.10.12";
const int   mqttPort =      1883;               // Port MQTT par défaut
const char* mqttUser =     "youba";             // Username of MQTT
const char* mqttPassword = "you22";

WiFiClient WifiClient;                // Creation de wifi client
PubSubClient client(mqttServer, mqttPort, WifiClient);

//--------- definition de la tache mailbox ---------------------------------------------------------$
enum {EMPTY, FULL};

struct mailbox_s {
  int state;
  char message[256]; // Tableau de caractères pour stocker le message
};

struct mailbox_s mb = {.state = EMPTY};
struct mailbox_s mb = {.state = EMPTY};

//--------- definition du timer  -------------------------------------------------------------------$
unsigned int waitFor(int timer, unsigned long period){
  static unsigned long waitForTimer[MAX_WAIT_FOR_TIMER];  // il y a autant de timers que de tâches p$
  unsigned long newTime = micros() / period;              // numéro de la période modulo 2^32
  int delta = newTime - waitForTimer[timer];              // delta entre la période courante et cell$
  if ( delta < 0 ) delta = 1 + newTime;                   // en cas de dépassement du nombre de péri$
  if ( delta ) waitForTimer[timer] = newTime;             // enregistrement du nouveau numéro de pér$
  return delta;
}



//----------------------------------------------------------------------------- definition de la tac$
struct oled_s {
  int timer;                                              // numéro de timer utilisé par WaitFor
  unsigned long period;                                             // periode d'affichage
} ;

void setup_oled(struct oled_s * ctx, int timer, unsigned long period) {
  ctx->timer = timer;
  ctx->period = period;
  Wire.begin(4, 15); // pins SDA , SCL                                    // initialisation du débit$
  Serial.begin(9600);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
  Serial.println(F("SSD1306 allocation failed"));
  for(;;); // Don't proceed, loop forever
  }
  display.setTextSize(2);
  display.setTextColor(WHITE);
}
void loop_oled(struct oled_s * ctx, struct mailbox_s * mb) {
  if (!(waitFor(ctx->timer,ctx->period))) return;         // sort s'il y a moins d'une période écoul$
//  if (mb->state != FULL) return; // attend que la mailbox soit pleine

  display.clearDisplay();
  display.setCursor(0, 0);
  display.print(mb->message);
  //display.print("%");
  display.display();
  mb->state = EMPTY;

}
//--------- definition de la tache Wifi ------------------------------------------------------------$

void setup_wifi() {
  Serial.println();
  Serial.print("Connexion à ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connecté");
  Serial.println("Adresse IP: ");
  Serial.println(WiFi.localIP());
}

//--------- definition de la tache rpi -------------------------------------------------------------$
void connect_rpi() {
  while (!client.connected()) {
    Serial.print("Tentative de connexion MQTT...");
    if (client.connect("ESP32Client", mqttUser, mqttPassword)) {
      Serial.println("connecté");
      client.subscribe("ESP/Led");
      client.subscribe("ESP/Oled");
    } else {
      Serial.print("échec, rc=");
      Serial.print(client.state());
      Serial.println(" nouvelle tentative dans 5 secondes");
      delay(5000);
    }
  }
}
//--------- Déclaration des tâches -----------------------------------------------------------------$
struct oled_s oled1;
struct mailbox_s mailbox1;
struct mailbox_s mailbox2;


//--------- Déclaration de callback ----------------------------------------------------------------$

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message reçu [");
  Serial.print(topic);
  Serial.print("] ");
  memset(mailbox2.message, 0, sizeof(mailbox2.message));
  // je compare le topic pour voir quelle tache est concerné
  if(strcmp(topic, "ESP/Led")==0){
    if(strncmp(mailbox2.message, "ON", 2) == 0)
    {
      Serial.println("LED ON");
      digitalWrite(LED_PIN, HIGH);
    }
    else if(strncmp(mailbox2.message, "OFF", 3) == 0)
    {
      Serial.println("LED OFF");
      digitalWrite(LED_PIN, LOW);
    }
    else
    {
      Serial.println("command not found");
    }
  }

  else if(strcmp(topic, "ESP/Oled") == 0)
  {
    memset(mailbox1.message, 0, sizeof(mailbox1.message));
    for (int i = 0; i < length; i++) {
    mailbox1.message[i] = (char)payload[i];
    }
    Serial.print(mailbox1.message);
    mailbox1.state = FULL;
    }
  else
  {
    Serial.println("No treatement for this topic");
  }
}
// Fonction pour publier les valeurs des boutons-poussoirs dans le topic ESP/Button ----------------$
void publishButtonValue() {
  // Lire la valeur du bouton-poussoir (exemple)
  int buttonValue = digitalRead(BUTTON_PIN);

  // Convertir la valeur du bouton-poussoir en chaîne de caractères
  char message[10];
  snprintf(message, sizeof(message), "%d", buttonValue);
  Serial.println("etat de bouton: " + String(buttonValue));

  // Publier la valeur dans le topic ESP/Button
  client.publish("ESP/Button", message);
}

// Fonction pour publier les valeurs de la photorésistance dans le topic ESP/PhotoR ----------------$
void publishPhotoRValue() {
  // Lire la valeur de la photorésistance (exemple)
  int photoRValue = map(analogRead(PhotoR_PIN), 0, 4095, 100, 0);

  char message[10];
  snprintf(message, sizeof(message), "%d", photoRValue);
  Serial.println("lumiere: "+ String(photoRValue)+" %");


  // Publier la valeur dans le topic ESP/PhotoR
  client.publish("ESP/PhotoR", message);
}


//--------- Setup et Loop --------------------------------------------------------------------------$

void setup() {
   pinMode(LED_PIN, OUTPUT);
  setup_oled(&oled1, 2, 1000000 );
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
}

void loop() {
if (!client.connected()) {
    connect_rpi();
  }
  client.loop();
  loop_oled(&oled1, &mailbox1);
if(waitFor(1,1000000)){
  publishButtonValue(); // Publier la valeur du bouton-poussoir
}
if(waitFor(3,1000000)){
  publishPhotoRValue(); // Publier la valeur de la photorésistance
}
}




