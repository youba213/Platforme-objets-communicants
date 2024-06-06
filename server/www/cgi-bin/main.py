#!/usr/bin/env python3
import os
import paho.mqtt.client as mqtt
import cgi
import time

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
    if message.payload.decode() == "0":  
        button_value = "OFF"
    else:
        button_value = "ON"

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
time.sleep(1)
mqttc.loop_stop()

# Générer le HTML
html = f"""
<!DOCTYPE html>
<html>
<head>
  <title>Peri Web Server</title>
  <META HTTP-EQUIV="Refresh" CONTENT="5; URL=/cgi-bin/main.py">
  <style>
    .switch {{
        position: relative;
        display: inline-block;
        width: 60px;
        height: 34px;
    }}
    .switch input {{
        opacity: 0;
        width: 0;
        height: 0;
    }}
    .slider {{
        position: absolute;
        cursor: pointer;
        top: 0;
        left: 0;
        right: 0;
        bottom: 0;
        background-color: #ccc;
        -webkit-transition: .4s;
        transition: .4s;
    }}
    .slider:before {{
        position: absolute;
        content: "";
        height: 26px;
        width: 26px;
        left: 4px;
        bottom: 4px;
        background-color: white;
        -webkit-transition: .4s;
        transition: .4s;
    }}
    input:checked + .slider {{
        background-color: #2196F3;
    }}
    input:focus + .slider {{
        box-shadow: 0 0 1px #2196F3;
    }}
    input:checked + .slider:before {{
        -webkit-transform: translateX(26px);
        -ms-transform: translateX(26px);
        transform: translateX(26px);
    }}
    .slider.round {{
        border-radius: 34px;
    }}
    .slider.round:before {{
        border-radius: 50%;
    }}
  </style>
</head>
<body>
  <h1>Envoyer un message</h1>
  <form method="POST" action="led.py">
    <input name="val" cols="20"></input>
    <input type="submit" value="Entrer"></input>
  </form>

  <h2>Etat du bouton</h2>
  <p>{button_value}</p>

  <h2>Valeur de la lumiere</h2>
  <p>{photoR_value}%</p>

  <!-- Switch pour contrôler la LED -->
  <h2>LED</h2>
  <form method="POST" action="led.py">
    <label class="switch">
      <input type="checkbox" name="led" value="1" onchange="this.form.submit()" {switch_checked} />
      <span class="slider round"></span>
    </label>
  </form>
</body>
</html>
"""

print("Content-Type: text/html")
print()
print(html)
