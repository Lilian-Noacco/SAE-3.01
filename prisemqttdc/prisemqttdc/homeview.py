from django.shortcuts import render, HttpResponseRedirect
def accueil(request): #cette vue sert uniquement à afficher la page d'accueil quand on se rends sur le site
    return render(request, "accueil.html")

def custom_404(request, exception):
    return render(request, '404.html', status=404)