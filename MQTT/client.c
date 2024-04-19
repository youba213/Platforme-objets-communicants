#include <stdio.h>
#include <stdlib.h>
#include <mosquitto.h>

#define TOPIC_DATA "data"
#define TOPIC_COMMAND "command"

// Structure pour stocker les données reçues
typedef struct {
  float temperature;
  int humidite;
} Data;

// Fonction de callback pour la réception de messages
void on_message(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message) {
  if (strcmp(message->topic, TOPIC_DATA) == 0) {
    // Décodage des données reçues
    Data data;
    memcpy(&data, message->payload, sizeof(Data));

    // Traitement des données
    printf("Température: %.2f°C\n", data.temperature);
    printf("Humidité: %d%%\n", data.humidite);

  } else if (strcmp(message->topic, TOPIC_COMMAND) == 0) {
    // Traitement de la commande reçue
    printf("Commande reçue: %s\n", (char *)message->payload);

    // Exécution de la commande
    // ...
  }
}

int main() {
  // Initialisation du client MQTT
  struct mosquitto *mosq = mosquitto_new(NULL, true, NULL);
  if (!mosq) {
    fprintf(stderr, "Erreur lors de l'initialisation du client MQTT\n");
    return 1;
  }

  // Configuration du client
  mosquitto_username_pw_set(mosq, "username", "password");
  mosquitto_connect(mosq, "localhost", 1883, 60);

  // Abonnement aux topics
  mosquitto_subscribe(mosq, NULL, TOPIC_DATA, 0);
  mosquitto_subscribe(mosq, NULL, TOPIC_COMMAND, 0);

  // Boucle de traitement
  while (1) {
    // Détection des messages reçus
    mosquitto_loop(mosq, -1, 1);

    // Envoi de commandes (optionnel)
    // ...
  }

  // Déconnexion du client
  mosquitto_disconnect(mosq);
  mosquitto_destroy(mosq);

  return 0;
}
