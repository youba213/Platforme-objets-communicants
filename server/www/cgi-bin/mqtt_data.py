#!/usr/bin/env python3
import os
import paho.mqtt.client as mqtt
import json

# Fichier pour l'état du switch
switch_state_file = '/tmp/switch_state.txt'

# Lire l'état du switch
try:
    with open(switch_state_file, 'r') as f:
        switch_state = f.read().strip()
except IOError:
    switch_state = "0"

switch_checked = 'checked' if switch_state == "1" else ''

# Variables pour stocker les valeurs des boutons et de la lumière
button_value = "Aucune donnee recue"
photoR_value = "Aucune donnee recue"

# Callback appelé lorsqu'un message est reçu sur le topic ESP/button
def on_button_message(client, userdata, message):
    global button_value
    button_value = message.payload.decode()

# Callback appelé lorsqu'un message est reçu sur le topic ESP/PhotoR
def on_photoR_message(client, userdata, message):
    global photoR_value
    photoR_value = message.payload.decode()

# Configuration du client MQTT
mqttc = mqtt.Client()
mqttc.message_callback_add("ESP/Button", on_button_message)
mqttc.message_callback_add("ESP/PhotoR", on_photoR_message)
mqttc.connect("172.20.10.12")

# Abonnement aux topics
mqttc.subscribe("ESP/Button")
mqttc.subscribe("ESP/PhotoR")
mqttc.loop_start()

# Attendre un court instant pour recevoir les messages
import time
time.sleep(1)
mqttc.loop_stop()

# Générer le JSON
data = {
    "button_value": button_value,
    "photoR_value": photoR_value,
    "switch_checked": switch_checked
}

print("Content-Type: application/json")
print()
print(json.dumps(data))
