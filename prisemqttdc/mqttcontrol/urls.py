# mqttcontrol/urls.py

from django.urls import path
from .views import control_page, send_command, get_button_states

urlpatterns = [
    path('control-page/', control_page, name='control_page'),
    path('send-command/', send_command, name='send_command'),  # Ajoutez cette ligne
    path('get-button-states/', get_button_states, name='get_button_states'),
]