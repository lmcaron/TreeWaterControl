/*****Module de controle de quantité d'eau pour sapin de NOEL*******
*****/

//sorties pompe
const int pompe[4] = {10,11,12,13};
const int voyantArret = 2;
//Variable d'état
int niveau = 0;
int distanceVide = 10;
int distance = 0;
bool etatPompe = LOW;

// Timers auxiliar variables
long now = millis();
long lastMeasure = 0;
long dernierArrosage[4] = {0,0,0,0};
long lastPublish[4] = {0,0,0,0};
long publishInterval = 5000;

//variables de vérification
int nbArrosageCycle = 0;
int nbArrosageMax = 4;
int tArrosage = 7000;
bool alarme = LOW;
volatile bool arret = 0;
int incomingByte = 0;

//definition des pins et variables pour sensor de distance 
// defines pins numbers
const int trigPin = 6;
const int echoPin = 7;

//fonction du detecteur qui renvoie la distance en cm
int detection()
{
  // defines variables
  long duration;
  float distance;
  
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10); 
  digitalWrite(trigPin, LOW);
  
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  
  // Calculating the distance
  distance= duration*0.034/2;
  return distance;
}

//fonction d'arrosage de la plante 
void arrosage(int indexPompe)
{
  etatPompe = HIGH;
  serialOut();
  digitalWrite(pompe[indexPompe], LOW);
  delay(tArrosage);
  digitalWrite(pompe[indexPompe], HIGH);
  etatPompe = LOW;
  serialOut();
}

//fonction d'envoi de toutes les données actuelles sur le port série
void serialOut()
{
  Serial.print("niveau:");
  Serial.print(niveau);
  Serial.print(",");
  Serial.print("alarme:");
  if(alarme == HIGH){
    Serial.print(1);
  }
  else{
    Serial.print(0);
  }
  Serial.print(",");
  Serial.print("pompe:");
  if(etatPompe == HIGH){
    Serial.print(1);
  }
  else{
    Serial.print(0); 
  }
  Serial.println();
}

// The setup function sets your ESP GPIOs to Outputs, starts the serial communication at a baud rate of 115200
// Sets your mqtt broker and sets the callback function
// The callback function is what receives messages and actually controls the LEDs
void setup() 
{
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  for (int j = 0; j < sizeof(pompe)/sizeof(pompe[0]); j++){ 
    pinMode(pompe[j], OUTPUT);
    digitalWrite(pompe[j], HIGH);    
  }
  Serial.begin(19200);
}

void loop() 
{
  if (Serial.available() > 0) {
    // read the incoming byte:
    incomingByte = Serial.read();
    if(incomingByte == 48){alarme = LOW;}
    if(incomingByte == 49){alarme = HIGH;}
  }
    
  now = millis();
  //si fonction millis se réinitialise, les valeurs temps sont ramenées à 0.
  if(now - lastMeasure < 0){
    lastMeasure = 0;
  }
  // Read water level every x seconds
  if (now - lastMeasure > publishInterval) {
    lastMeasure = now;
    niveau = distanceVide - detection();
    if (niveau <= 1 && alarme == LOW){
      nbArrosageCycle = 0;
      while(niveau < 3 && nbArrosageCycle < nbArrosageMax){
        arrosage(1);
        nbArrosageCycle++;
        delay(2000);
        niveau = distanceVide - detection();
      }
      if(nbArrosageCycle >= nbArrosageMax){
        alarme = HIGH;
      }
    }
    serialOut();
  }
} 
