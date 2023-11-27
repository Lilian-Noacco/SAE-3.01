from django.db import models

# Create your models here.
from django.db import models

class Prise(models.Model):
    device = models.CharField(primary_key=True, max_length=6)
    ledstate = models.CharField(db_column='ledState', max_length=4)  # Field name made lowercase.

    class Meta:
        managed = False
        db_table = 'prise'