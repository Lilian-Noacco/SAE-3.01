import json
import mysql.connector
import paho.mqtt.client as mqtt

# Configuration du broker MQTT
mqtt_broker_address = "broker.hivemq.com"
mqtt_broker_port = 1883
mqtt_topic = "dlj/prisetest/prises"

# Configuration de la base de données MySQL
mysql_host = "127.0.0.1"
mysql_user = "root"
mysql_password = ""
mysql_database = "PowerOutlet"

def on_connect(client, userdata, flags, rc):
    print(f"Connected with result code {rc}")
    client.subscribe(mqtt_topic)

def on_message(client, userdata, msg):
    try:
        # Charger le message JSON
        data = json.loads(msg.payload.decode())

        # Vérifier si le message contient les clés nécessaires
        if "deviceupdate" in data and "ledState" in data:
            device_name = data["deviceupdate"]
            led_state = data["ledState"]

            # Mettre à jour la base de données MySQL
            update_led_state(device_name, led_state)

            print(f"Received message for device {device_name}: LED state - {led_state}")
        else:
            print("Message JSON invalide. Les clés nécessaires sont manquantes.")
    except json.JSONDecodeError:
        print("Erreur lors du décodage du message JSON.")

def update_led_state(device_name, led_state):
    # Connexion à la base de données MySQL
    connection = mysql.connector.connect(
        host=mysql_host,
        user=mysql_user,
        password=mysql_password,
        database=mysql_database
    )

    # Créer un curseur pour interagir avec la base de données
    cursor = connection.cursor()

    # Exécuter la requête de mise à jour
    sql_update_query = f"UPDATE prise SET ledState = '{led_state}' WHERE device = '{device_name}'"
    cursor.execute(sql_update_query)

    # Valider la transaction
    connection.commit()

    # Fermer le curseur et la connexion à la base de données
    cursor.close()
    connection.close()

# Création du client MQTT
mqtt_client = mqtt.Client()
mqtt_client.on_connect = on_connect
mqtt_client.on_message = on_message

# Connexion au broker MQTT
mqtt_client.connect(mqtt_broker_address, mqtt_broker_port, 60)

# Boucle principale du client MQTT
mqtt_client.loop_forever()