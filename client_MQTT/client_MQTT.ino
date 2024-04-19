#include <Wire.h>
#include <SPI.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define BUZZER_PIN 17  // Pin du buzzer
#define PhotoR_PIN 36
#define MAX_WAIT_FOR_TIMER 5
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     16 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


const char* password =      "youba123";
const char* ssid =          "iPhone de Youba";

//const char* ssid =          "PopFati";
//const char* password =      "espci1234";
const char* mqttServer =    "172.20.10.12";
const int   mqttPort =      1883;               // Port MQTT par défaut
const char* mqttUser =     "youba";             // Username of MQTT
const char* mqttPassword = "you22";        

WiFiClient WifiClient;                // Creation de wifi client
PubSubClient client(mqttServer, mqttPort, WifiClient);

//--------- definition de la tache mailbox -------------------------------------------------------------------------------------//
enum {EMPTY, FULL};

struct mailbox_s {
  int state;
  char message[256]; // Tableau de caractères pour stocker le message
  int val;
};

struct mailbox_s mb = {.state = EMPTY};

//--------- definition du timer  -------------------------------------------------------------------------------------//
unsigned int waitFor(int timer, unsigned long period){
  static unsigned long waitForTimer[MAX_WAIT_FOR_TIMER];  // il y a autant de timers que de tâches périodiques
  unsigned long newTime = micros() / period;              // numéro de la période modulo 2^32 
  int delta = newTime - waitForTimer[timer];              // delta entre la période courante et celle enregistrée
  if ( delta < 0 ) delta = 1 + newTime;                   // en cas de dépassement du nombre de périodes possibles sur 2^32 
  if ( delta ) waitForTimer[timer] = newTime;             // enregistrement du nouveau numéro de période
  return delta;
}



//--------- définition de la tache Led -------------------------------------------------------------------------------------//

struct Led_s {
  int timer;                                              // numéro du timer pour cette tâche utilisé par WaitFor
  unsigned long period;                                   // periode de clignotement
  int pin;                                                // numéro de la broche sur laquelle est la LED
  int etat;                                               // etat interne de la led
}; 

void setup_Led( struct Led_s * ctx, int timer, unsigned long period, byte pin) {
  ctx->timer = timer;
  ctx->period = period;
  ctx->pin = pin;
  ctx->etat = 0;
  pinMode(pin,OUTPUT);
  digitalWrite(pin, ctx->etat);
}

void loop_Led( struct Led_s * ctx) {
  if (!waitFor(ctx->timer, ctx->period)) return;          // sort s'il y a moins d'une période écoulée
  digitalWrite(ctx->pin,ctx->etat);                       // ecriture
  ctx->etat = 1 - ctx->etat;                              // changement d'état
} 


//--------- definition de la tache Mess -------------------------------------------------------------------------------------//

struct Mess_s {
  int timer;                                              // numéro de timer utilisé par WaitFor
  unsigned long period;                                             // periode d'affichage
  int val;
  //char mess[20];
} ; 

void setup_Mess( struct Mess_s * ctx, int timer, unsigned long period) {
  ctx->timer = timer;
  ctx->period = period;
  Serial.begin(9600);                                     // initialisation du débit de la liaison série
}

void loop_Mess(struct Mess_s *ctx, const char * mess) {
  if (!(waitFor(ctx->timer,ctx->period))) return;         // sort s'il y a moins d'une période écoulée
  Serial.println(ctx -> val);                              // affichage du message
}

//----------------------------------------------------------------------------- definition de la tache oled ------------------------------------------//
struct oled_s {
  int timer;                                              // numéro de timer utilisé par WaitFor
  unsigned long period;                                             // periode d'affichage
} ; 

void setup_oled(struct oled_s * ctx, int timer, unsigned long period) {
  ctx->timer = timer;
  ctx->period = period;
  Wire.begin(4, 15); // pins SDA , SCL                                    // initialisation du débit de la liaison série
  Serial.begin(9600);
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
  Serial.println(F("SSD1306 allocation failed"));
  for(;;); // Don't proceed, loop forever
  }
  display.setTextSize(2);
  display.setTextColor(WHITE); 
}

void loop_oled(struct oled_s * ctx, struct mailbox_s * mb) {
  if (!(waitFor(ctx->timer,ctx->period))) return;         // sort s'il y a moins d'une période écoulée
//  if (mb->state != FULL) return; // attend que la mailbox soit pleine
  
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print(mb->message);
  //display.print("%");
  display.display();
  mb->state = EMPTY;

}

//--------- definition de la tache lum -------------------------------------------------------------------------------------//
struct lum_s {
  int timer;                                              // numéro de timer utilisé par WaitFor
  unsigned long period;                                             // periode d'affichage
  int pin;
} ; 

void setup_lum(struct lum_s * ctx, int timer, unsigned long period, byte pin) {
  ctx->timer = timer;
  ctx->period = period;  
  ctx->pin = pin;
  pinMode(pin,INPUT);
}

void loop_lum(struct lum_s * ctx, struct mailbox_s * mb, struct Mess_s * mes) {
  if (!(waitFor(ctx->timer,ctx->period))) return;         // sort s'il y a moins d'une période écoulée
    mes-> val = map(analogRead(ctx->pin), 0, 4095, 100, 0);
    mb-> val = map(analogRead(ctx->pin), 0, 4095, 100, 0);
    mb->state = FULL;
    

}
//--------- definition de la tache ISR -------------------------------------------------------------------------------------//
char loop_SerialEvent(){
  return Serial.read(); 
  }

//--------- definition de la tache Wifi -------------------------------------------------------------------------------------//

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

//--------- definition de la tache rpi -------------------------------------------------------------------------------------//
void connect_rpi() {
  while (!client.connected()) {
    Serial.print("Tentative de connexion MQTT...");
    if (client.connect("ESP32Client", mqttUser, mqttPassword)) {
      Serial.println("connecté");
      client.subscribe("esp32/test");
    } else {
      Serial.print("échec, rc=");
      Serial.print(client.state());
      Serial.println(" nouvelle tentative dans 5 secondes");
      delay(5000);
    }
  }
}


//--------- Déclaration des tâches -------------------------------------------------------------------------------------//

struct Led_s Led1;
struct Mess_s Mess1;
struct oled_s oled1;
struct lum_s lum1;
struct mailbox_s mailbox1;
struct mailbox_s mailbox2;


//--------- Déclaration de callback -------------------------------------------------------------------------------------//

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message reçu [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    mailbox1.message[i] = (char)payload[i];
//    Serial.print((char)payload[i]);
  }
  mailbox1.state = FULL;
  Serial.print(mailbox1.message);

  Serial.println("Message copié dans la boîte aux lettres");
  Serial.println();
}

//--------- Setup et Loop -------------------------------------------------------------------------------------//

void setup() {
  setup_Led(&Led1, 0, 100000, LED_BUILTIN);                        // Led est exécutée toutes les 100ms 
  setup_Mess(&Mess1, 1, 1000000);              // Mess est exécutée toutes les secondes 
  setup_oled(&oled1, 2, 1000000 );
//  setup_lum(&lum1, 4, 100000, PhotoR_PIN);

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
//  delay(5000); // Attente de 5 secondes entre chaque envoi de message
//  client.publish("esp32/test", "Hello from ESP32");
}
