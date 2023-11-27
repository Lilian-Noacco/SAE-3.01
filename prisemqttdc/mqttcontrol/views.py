from django.shortcuts import render
from .models import Prise
from django.http import JsonResponse
import json
import paho.mqtt.publish as publish
from django.contrib.auth.decorators import login_required
from django.http import JsonResponse
from django.views.decorators.csrf import csrf_exempt

from django.views.decorators.csrf import csrf_exempt
import json
@login_required(login_url="/auth/login/")
def control_page(request):
    # Récupérer toutes les prises depuis la base de données
    power_outlets = Prise.objects.all()
    print(power_outlets)
    # Transmettre les données à la page HTML
    return render(request, 'control_page.html', {'prises': power_outlets})

@csrf_exempt
@login_required(login_url="/auth/login/")
def get_button_states(request):
    power_outlets = Prise.objects.all()
    data = [{'device': outlet.device, 'ledState': outlet.ledstate} for outlet in power_outlets]
    return JsonResponse(data, safe=False)

@csrf_exempt
@login_required(login_url="/auth/login/")
def send_command(request):
    if request.method == 'POST':
        try:
            data = json.loads(request.body.decode())
            device_name = data.get("deviceupdate")
            led_state = data.get("ledState")

            # Envoyer la trame JSON à la prise via MQTT
            mqtt_topic = f"dlj/prisetest/prises"
            mqtt_payload = json.dumps({"device": device_name, "ledState": led_state})

            # Spécifiez les détails de votre broker MQTT
            mqtt_broker_address = "broker.hivemq.com"
            mqtt_broker_port = 1883

            publish.single(mqtt_topic, mqtt_payload, hostname=mqtt_broker_address, port=mqtt_broker_port)

            # Répondre avec un message JSON
            return JsonResponse({'status': 'success'})
        except json.JSONDecodeError:
            return JsonResponse({'status': 'error', 'message': 'Invalid JSON format.'})

    return JsonResponse({'status': 'error', 'message': 'Invalid request method.'})
