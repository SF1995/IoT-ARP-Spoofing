// TEIL 1 -- Bibliotheken
// HINWEIS: Der Bewegungssensor benötigt keine Bibliothek
// Die Datei ermöglicht Netzwerkkommunikation mit dem Arudino UNO WIFI REV.2
// Herausgeber: Arduino
#include <WiFiNINA.h>
// Die Datei ermöglicht das Senden und erhalten von MQTT Nachrichten. 
// Herausgeber: Arduino
#include <ArduinoMqttClient.h>
// Die Datei enthält die SSID und Passwort des Routers in einer separaten Datei
#include "secrets.h"
// Diese Definition speichert, an welchen Pins des Arduinos die LED und der Bewegungssensor angeschlossen sind
#define LedPin 2
#define PirPin 8

// TEIL 2 -- Netzwerkkonfigurationen

// Variablen beinhalten sensitive Daten der secrets.h Datei //
char ssid[] = SECRET_SSID;        // Name des Routers (SSID)
char pass[] = SECRET_PASS;        // Passwort für den Zugang zu WIFI 

// Objektinitialisierung //
// Der Wifi Client wird initialisiert, der sich mit dem Router verbindet
WiFiClient wifiClient;
// Der MqttClient wird initalisiert, welcher sich mit dem MQTT Broker (Mosquitto) verbindet und gleichzeitig wifiClient ist
MqttClient mqttClient(wifiClient);

// Referenzen des Brokers //
// IP-Adresse des Brokers
const char broker[] = ".............";
// Port des Brokers, an dem die Nachrichten ankommen
int        port     = 1883;
// Topic, unter dem die Nachricht verwendet wird (SIEHE KAPITEL...)
const char topic[]  = "movement";



// TEIL 3 -- Setup
// Das Setup wird einmalig ausgeführt
void setup() {
  
  // Der seriellen Aufbau wird mit der Baud-Rate 9600 gestartet
  Serial.begin(9600);
  // Die Methode pinMode weißt der LED einen Pin zu, durch den sie verbunden und bekommt den Wert OUTPUT, da sie  durch den Arduino zum leuchten gebracht wird
  pinMode(LedPin, OUTPUT);
  // Hier wird ebenfalls dem Bewegungssensor einen Pin zugewiesen. Der Wert INPUT signalisiert, dass der Bewegungssensor Bewegungen aufzeichnet und an den Arduino in Form von 0 (keine Bewegung) und 1 (Bewegung) weiterleitet
  pinMode(PirPin, INPUT);

  // Es wird auf die Verbindung zum seriellen Anschluss gewartet. Wird nur für den nativen USB-Anschluss benötigt (welcher verwendet wird für das Hochladen des Programms)
  while (!Serial) {
     ;
  }

  // Es wird versucht, sich mit dem Netzwerk zu verbinden (über den Router)
  // Hier werden die Variablen aus TEIL 2 verwendet
  Serial.print("Attempting to connect to WPA SSID: ");
  Serial.println(ssid);
  
  // Falls sich der Arduino nicht mit dem Netzwerk verbunden hat, wird ein Punkt abgezeichnet und der Verbindungsaufabau nach 5 Sekunden erneut durchgeführt
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    Serial.print(".");
    delay(5000);
  }
  
  // Falls sich der Arduino mit dem Netzwerk verbunden hat, werden eine Bestätigungsnachricht und die IP-Adresse des Arduinos auf dem seriellen Monitor ausgegeben
  Serial.println("You're connected to the network");
  Serial.print("IP of the Arduino: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  // Es wird versucht, sich mit dem Broker zu Verbinden, was auf dem seriellen Monitor angezeigt wird
  Serial.print("Attempting to connect to the MQTT broker: ");
  Serial.println(broker);

  // Falls sich der Arduino nicht mit dem Broker verbunden hat, wird eine Fehlermeldung auf dem seriellen Monitor ausgegeben und nach 2 Sekunden wird die Verbindung erneut augebaut
  while (!mqttClient.connect(broker, port)) {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());
    delay(2000);
  }

  // Falls sich der mqttClient mit dem Broker verbunden hat, wir eine Bestätigungsnachricht gesendet
  Serial.println("You're connected to the MQTT broker!");
  Serial.println();

}

// TEIL 4 -- globale Funktion
// Die Variable count wird auf eins gesetzt. Sie weißt jeder Bewegung einen aufsteigenden Wert zu, anhand dessen die Bewegung identifiziert werden kann. Das zählen beginnt bei eins, da es sich um die erste Bewegung handelt (nicht 0)
int count = 1;

// TEIL 5 -- Loop
// Dieser Teil ermöglicht es, den Programmablauf innerhalb der Funktion nach Beendigung erneut auszuführen. Dies wird so lange wiederholt, bis der Arduino nicht mehr mit Strom versorgt wird
// loop() führt den Hauptteil des Skriptes aus
void loop() {

  // poll() wird regelmäßig aufgerufen, damit die Bibliothek MQTT Keep Alive senden kann, was verhindert, dass der Broker die Verbindung trennt
  mqttClient.poll();

  // Die Variable s ließt den Wert ein, der von dem Bewegungssensor versendet wird
  int s = digitalRead(PirPin);

  // Sobald der Wert nicht null ist (somit 1 --> Bewegung verzeichnet)
  if (s != 0) {
    
          // die LED Pin wird durch digitalWrite() zum Leuchten gebracht (1 bedeutet "An", 0 wäre "Aus" und s ist durch die if-Bedingung immer 1), da Bewegung verzeichnet wurde (dient ebenfalls der Kontrolle)
          digitalWrite(LedPin, s);

          // Hier wird count zu einem String gecastet, um den Wert seriell mit einem Satz zu verknüpfen
          String anzahl = String(count);

          // Ausgabe der Information über das Ereignis, das auf den seriellen Monitor ausgegeben wird
          Serial.println(anzahl + "te Bewegung wahrgenommen in Raum 1!");
          Serial.print("Sending message to topic: ");
          Serial.println(topic);
          Serial.println("Bewegung");


          // Ebenfalls sendet der mqttClient des Arduino die Topic und die Bewegung an den Broker
          mqttClient.beginMessage(topic);
          mqttClient.print(anzahl + "te Bewegung in Raum 1!");
          mqttClient.endMessage();
          Serial.println();
          // count wird inkrementiert, wodurch die aufsteigenden Werte zustande kommen
          count += 1;
          // Nach zwei Sekunden wird die Schleife verlassen
          delay(2000);
  }
  // Die LED muss durch Low (=0) ausgeschaltet werden. Ansonsten würde sie durchgehend leuchten
  digitalWrite(LedPin, LOW);
  
}
