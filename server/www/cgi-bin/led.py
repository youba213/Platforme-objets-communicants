#!/usr/bin/env python3
import cgi
import paho.mqtt.publish as publish

# Fonction pour publier un message MQTT
def publish_mqtt(topic, message):
    publish.single(topic, message, hostname="172.20.10.12")

# Récupérer la valeur du formulaire
form = cgi.FieldStorage()
val = form.getvalue('val')
led_state = form.getvalue('led')

switch_state_file = '/tmp/switch_state.txt'

# Si une valeur est présente dans le formulaire, publier dans le topic correspondant
if val is not None:
    publish_mqtt("ESP/Oled", val)

# Gérer l'état du switch
if led_state == "1":
    publish_mqtt("ESP/Led", "ON")
    with open(switch_state_file, 'w') as f:
        f.write("1")
else:
    publish_mqtt("ESP/Led", "OFF")
    with open(switch_state_file, 'w') as f:
        f.write("0")

# Rediriger vers la page principale pour éviter la resoumission du formulaire
print("Content-Type: text/html")
print()
print("""
<!DOCTYPE html>
<html>
<head>
  <title>Peri Web Server</title>
 
  <META HTTP-EQUIV="Refresh" CONTENT="1; URL=/cgi-bin/main.py">
</head>
<body>
</body>
</html>
""")
