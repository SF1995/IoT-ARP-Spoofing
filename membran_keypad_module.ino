//** Das Skript basiert auf auf einem Beispiel für die Kommunikation mittels MQTT von Karl Söderby**
// https://docs.arduino.cc/tutorials/uno-wifi-rev2/uno-wifi-r2-mqtt-device-to-device

// TEIL 1 -- Bibliotheken
// Die Datei ermöglicht Netzwerkkommunikation mit dem Arudino UNO WIFI REV.2
// Herausgeber: Arduino
#include <WiFiNINA.h>
// Die Datei ermöglicht das Benutzen der 4x4 Matrix
// Herausgeber: Mark Stanley und Alexander Brevig
#include <Keypad.h>
// Die Datei ermöglicht das Senden und erhalten von MQTT Nachrichten. 
// Herausgeber: Arduino
#include <ArduinoMqttClient.h>
// Die Datei enthält die SSID und Passwort des Routers in einer separaten Datei
#include "secret.h"

// TEIL 2 -- Netzwerkkonfigurationen

// Variablen beinhalten sensitive Daten der secrets.h Datei //
char ssid[] = SECRET_SSID;    // Name des Routers (SSID)
char pass[] = SECRET_PASS;    // Passwort für den Zugang zu WIFI 

// Objektinitialisierung //
// Der Wifi Client wird initialisiert, der sich mit dem Router verbindet
WiFiClient wifiClient;
// Der MqttClient wird intialisiert, welcher sich mit dem MQTT Broker (Mosquitto) verbindet und gleichzeitig  wifiClient ist
MqttClient mqttClient(wifiClient);

// Referenzen des Brokers //
// IP-Adresse des Brokers
const char  broker[] = "...........";
// Port des Brokers, an dem die Nachrichten ankommen
int         port     = 1883;
// Topic, unter dem die Nachricht verwendet wird (SIEHE KAPITEL...)
const char  topic[]  = "password";

// TEIL 3 -- Matrixkonfiguration
// Die zweidimensionalen Matrix wird definiert //
// Die Matrix hat 4 Zeilen 
const byte ROWS = 4;
// Die Matrix hat 4 Spalten
const byte COLS = 4;

// Der HexaKeys wird erstellt, um zu definieren, wie die Matrix aufgebaut ist  //
char hexaKeys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'},
};

// Es erfolgt eine Angabe der Pins, die an dem Arduino mit der Membran Matrix verbunden wurden (SIEHE KAPITEL...)
byte rowPins[]= {9,8,7,6};
byte colPins[]= {5,4,3,2};

// Die Matrix wird mit dem Namen "customKeypad"  initialisiert 
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

// TEIL 4 -- Setup
// Das Setup wird einmalig ausgeführt
void setup() {
   
  // Der seriellen Aufbau wird mit der Baud-Rate 9600 gestartet
  Serial.begin(9600);

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

// TEIL 5 -- globale Funktion
// clearPassword() löschen des Passwortes, indem leere Werte eingetragen werden und wird in Teil 6 verwendet
void clearPassword(char* password) {
  for (int i = 0; i < 8; i++) {
    password[i] = '\0';  // Passwort löschen
  }
}

// TEIL 6 -- Loop
// Dieser Teil ermöglicht es, den Programmablauf innerhalb der Funktion nach Beendigung erneut auszuführen. Dies wird so lange wiederholt, bis der Arduino nicht mehr mit Strom versorgt wird
// loop() führt den Hauptteil des Skriptes aus
void loop(){
  
  // poll() wird regelmäßig aufgerufen, damit die Bibliothek MQTT Keep Alive senden kann, was verhindert, dass der Broker die Verbindung trennt
  mqttClient.poll();

  // Das Array wird für die Speicherung des Passworts (plus 1 für das Nullzeichen) verwendet
  static char password[9];  // Array zur Speicherung des Passworts (plus 1 für das Nullzeichen)
  // Der Index dient zur Verfolgung der aktuellen Position im Passwort
  static int index = 0;     

  // Falls der Zähler (index) kleiner als 8 ist, wird der Tastendruck ausgewertet (das Passwort soll 7 Zeichen lang sein. Das Zeichen, dass das Passwort abschließt, jedoch nicht zu dem Passwort gehört, ist '*'
  if (index < 8){
    char key = customKeypad.getKey();
    
    // und falls der Tastendruck keinen leeren Wert beinhaltet
    if (key != '\0') {
      
      // und falls der abschließende Tastendruck ein '*' ist
      if (key == '*') {
        
        // und falls der index (vor dem inkrementieren) auf sieben steht (was bedeutet, dass das Passwort 7 Zeichen beinhaltet)
        if (index == 7) {
          
          // wird ein Nullzeichen hinzugefügt, um dass Passwort abzuschließen und Nachrichten auf dem seriellen Monitor ausgegeben, dass eine Bestätigungsnachricht beinhaltet und das Passwort sowie die Topic nennt 
          password[index] = '\0';  
          Serial.println("Das Passwort wurde korrekt eingegeben!");
          Serial.print("Das Passwort ist ");
          Serial.println(password); 
          Serial.print("Sending message to topic: ");
          Serial.println(topic);
          Serial.println(password);

          // Ebenfalls sendet in diesem Fall der mqttClient des Arduino die Topic und das Passwort an den Broker
          mqttClient.beginMessage(topic);
          mqttClient.print(password);
          mqttClient.endMessage();
          Serial.println();

          
          // Nach dem Versenden wird der Index für die weitere Eingabe von Passwörtern auf Null gesetzt und die bereits erwähnte Funktion, die das Passwort zurücksetzt, verwendet 
          index = 0;                
          clearPassword(password);
          
          // Falls der '*' gedrückt wurde, jedoch der Index nicht bei 7 steht, bedeutet das, dass die Passwörter nicht richtig eingegeben wurden. Auch hier werden Index und Passwort zurückgesetzt
        } else {
          Serial.println("Passwort unvollständig. Bitte erneut eingeben.");
          index = 0;                
          clearPassword(password);
        }
        
        // Falls der abschließende Tastendruck kein '*' ist, jedoch der Index bei sieben liegt (also bereits das gesamte Passwort eingegeben wurde und das abschließende Zeichen kein '*' ist), wird das Passwort als nicht richtig gewertet
        // Es werden die gleichen Schritte wie bei der else-Bedinung eingeleitet
      } else {

        if (index == 7) {
          Serial.println("Passwort nicht richtig. Bitte erneut eingeben.");
          index = 0;                
          clearPassword(password);
          
          // Falls der Index nicht bei sieben steht und das Passwort nicht mit einem '*' abgeschlossen wurde, bedeutet dies, dass das Passwort noch nicht vollständig eingegeben wurde
          // Somit können weitere Ziffern hinzugefügt werden können, um das Passwort zu vervollständigen
          // Der Index wird in dieser Bedingung jedes Mal erhöht, um zu erkennen, wann das Passwort die erforderlichen sieben Stellen beinhaltet
        }else{
          password[index] = key;    
          Serial.print("Passwort: ");
          Serial.println(password); 
          index++;                  
        }
      }
    }
  }
}
