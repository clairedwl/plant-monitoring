# 🌿 SoilMoisture Monitor

Projet personnel de surveillance de l’humidité du sol.  
Un capteur branché à un **ESP32** mesure l’humidité et envoie les données en **JSON** via WiFi à un **serveur Flask** hébergé sur une **Raspberry Pi**.

##  Objectif

Automatiser la détection de sécheresse du sol pour savoir quand arroser ses plantes.

##  Fonctionnalités

- 📡 Lecture d’un capteur d’humidité du sol
- 📲 Envoi des données via WiFi avec l’ESP32 (ESP-IDF)
- 🌐 Réception et traitement des données via un serveur Flask sur Raspberry Pi
- 📈 Visualisation ou stockage local des mesures

##  Matériel utilisé

- ESP32 (DevKitC)
- Capteur d’humidité du sol capacitif
- Raspberry Pi (3 ou 4)
- Alimentation, câbles, breadboard

##  Technologies

- **ESP-IDF** (framework officiel Espressif, langage C)
- **Flask** (Python) pour le serveur
- **JSON** pour le format de transmission des données
- **HTTP (POST)** pour la communication
