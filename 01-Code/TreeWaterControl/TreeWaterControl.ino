/*****Module de controle de quantité d'eau pour sapin de NOEL*******
*****/

//sorties pompe
const int pompe[4] = {10,11,12,13};

//Variable d'état
int niveau = 0;
int distanceVide = 11;
int distance = 0;
bool etatPompe = LOW;

// Timers auxiliar variables
long now = millis();
long lastAction[3] = {0,0,0};
long actionInterval[3] = {500,3000,15000};
long lastMeasure = 0;

//variables de vérification
bool alarme = LOW;
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

//fonction depart d'arrosage de la plante 
void departArrosage(int indexPompe)
{
  etatPompe = HIGH;
  digitalWrite(pompe[indexPompe], LOW);
}

//fonction arret d'arrosage de la plante 
void arretArrosage(int indexPompe)
{
  etatPompe = LOW;
  digitalWrite(pompe[indexPompe], HIGH);
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

//fonction qui chronomètre le temps pour exécuter des actions
bool timeReached(int nTimer)
{
  if(now - lastAction[nTimer] > actionInterval[nTimer]){
    lastAction[nTimer] = now;
    return HIGH;
    }
  else{
    return LOW;
  }
}

void setup() 
{
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  for (int j = 0; j < sizeof(pompe)/sizeof(pompe[0]); j++){ 
    pinMode(pompe[j], OUTPUT);
    digitalWrite(pompe[j], HIGH);    
  }
  Serial.begin(57600);
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
  if (timeReached(0)) {
    //lecture de niveau
    niveau = distanceVide - detection();
    
    //conditions d'activation/arret de pompe
    if (niveau <= 2 && alarme == LOW && etatPompe == LOW){
      departArrosage(1);
      lastAction[2] = now;
    }
    if(niveau >= 3){
      arretArrosage(1); 
    }
    //sécurité si la pompe arrose trop longtemps
    if(etatPompe == HIGH && timeReached(2)){
      arretArrosage(1);
      alarme = HIGH;
    } 
    if(niveau >=5){
      arretArrosage(1);
      alarme = HIGH;
    }
    if(alarme == HIGH){
      arretArrosage(1);
    }
    //envoi de données d'état actuel
    if(timeReached(1)){
      serialOut();
    }
  }
} 
