#define LED_GREEN_1 12
#define LED_GREEN_2 11
#define LED_GREEN_3 10
#define LED_GREEN_4 9
#define LED_RED 8
#define BUTTON1 4
#define BUTTON2 5
#define BUTTON3 6
#define BUTTON4 7

#define INITIALIZE_STATE 1
#define PLAY_STATE 2
#define FAULT_STATE 3



volatile bool interrupted = false;
volatile unsigned short int buttonNumberPressed;
volatile bool greska;
volatile int brojac;
volatile bool pritisnutoDugme;
volatile bool pokrenutoBrojanje;
unsigned short int state;

unsigned short int combinationLength;

int *combination;
unsigned short int brojPokusaja;
unsigned short int buttonPressedOrder;



void setup(){
pinMode(LED_GREEN_1,OUTPUT);
pinMode(LED_GREEN_2,OUTPUT);
pinMode(LED_GREEN_3,OUTPUT);
pinMode(LED_GREEN_4,OUTPUT);
pinMode(LED_RED,OUTPUT);
pinMode(2,INPUT);// interrupt pin
pinMode(BUTTON1,INPUT);//BUTTON 1
pinMode(BUTTON2,INPUT);//BUTTON 2
pinMode(BUTTON3,INPUT);//BUTTON 3
pinMode(BUTTON4,INPUT);//BUTTON 4
attachInterrupt(digitalPinToInterrupt(2),buttonPressedISR,RISING);

state = INITIALIZE_STATE;
combinationLength = 2;
brojPokusaja = 3;
greska = false;
brojac = 0;
pokrenutoBrojanje = false;
pritisnutoDugme = false;

// TIMER 1 for interrupt frequency 2 Hz:
cli(); // stop interrupts
TCCR1A = 0; // set entire TCCR1A register to 0
TCCR1B = 0; // same for TCCR1B
TCNT1  = 0; // initialize counter value to 0
// set compare match register for 2 Hz increments
OCR1A = 31249; // = 16000000 / (256 * 2) - 1 (must be <65536)
// turn on CTC mode
TCCR1B |= (1 << WGM12);
// Set CS12, CS11 and CS10 bits for 256 prescaler
TCCR1B |= (1 << CS12) | (0 << CS11) | (0 << CS10);
// enable timer compare interrupt
TIMSK1 |= (1 << OCIE1A);
sei(); // allow interrupts

Serial.begin(9600);
}

void loop(){

switch(state){
  case INITIALIZE_STATE:
  detachInterrupt(digitalPinToInterrupt(2));
  free(combination);
  combination = (int*) malloc(combinationLength * sizeof(unsigned short int));
  buttonPressedOrder = 0;
  
  //Generisanje random kombinacije od 1 do 4
  for(int i = 0;i<combinationLength;i++){
    combination[i] = random(1,5);
  }

  //Paljenje dioda po generisanoj kombinaciji
  for(int j = 0;j<combinationLength;j++){
    if(combination[j] == 1){
      digitalWrite(LED_GREEN_1,HIGH);
      delay(1500);
      digitalWrite(LED_GREEN_1,LOW);
      delay(1500);
    }
    else if(combination[j] == 2){
      digitalWrite(LED_GREEN_2,HIGH);
      delay(1500);
      digitalWrite(LED_GREEN_2,LOW);
      delay(500);
    }
    else if(combination[j] == 3){
      digitalWrite(LED_GREEN_3,HIGH);
      delay(1500);
      digitalWrite(LED_GREEN_3,LOW);
      delay(500);
    }
    else if(combination[j] == 4){
      digitalWrite(LED_GREEN_4,HIGH);
      delay(1500);
      digitalWrite(LED_GREEN_4,LOW);
      delay(500);
    }
  }
  state = PLAY_STATE;
  attachInterrupt(digitalPinToInterrupt(2),buttonPressedISR,RISING);
  break;  

  case PLAY_STATE:
  pokrenutoBrojanje = true;
   if(interrupted && buttonNumberPressed >0){
   //ButtonPressed
   pritisnutoDugme = true; 
   Serial.println(buttonNumberPressed);
    if(combination[buttonPressedOrder] == buttonNumberPressed){
      buttonPressedOrder++;
      if(combinationLength == buttonPressedOrder){
    //Pogodjena kombinacija
        digitalWrite(LED_GREEN_1,HIGH);
        digitalWrite(LED_GREEN_2,HIGH);
        digitalWrite(LED_GREEN_3,HIGH);
        digitalWrite(LED_GREEN_4,HIGH);
        digitalWrite(LED_RED,HIGH);
        delay(600);
        digitalWrite(LED_GREEN_1,LOW);
        digitalWrite(LED_GREEN_2,LOW);
        digitalWrite(LED_GREEN_3,LOW);
        digitalWrite(LED_GREEN_4,LOW);
        digitalWrite(LED_RED,LOW);

        combinationLength++;
        state = INITIALIZE_STATE;
      }
    }
    else{
    //greska
    brojPokusaja--;
    greska = true;
      if(brojPokusaja <= 0){
        state = FAULT_STATE;
        Serial.println("KRAJ IGRE");
        TIMSK1 = 0;
    }
    Serial.println("GRESKA");

  }
  interrupted = 0; 
 }
  //Kraj play Stanja
  break;

  case FAULT_STATE:
  greska = false;
  digitalWrite(LED_RED,HIGH);
  break;

}

}

void buttonPressedISR(){
  interrupted = 1;
  noInterrupts();
  if(digitalRead(BUTTON1)){
     buttonNumberPressed = 1;
  }
  else if(digitalRead(BUTTON2)){
     buttonNumberPressed = 2;
  }
  else if(digitalRead(BUTTON3)){
     buttonNumberPressed = 3;
  }
  else if(digitalRead(BUTTON4)){
     buttonNumberPressed = 4;
  }
  interrupts();  
}

//Timer1 ISR funkcija(SVAKIH ~ 0.5s)
ISR(TIMER1_COMPA_vect){
  if(pokrenutoBrojanje){
    brojac++;
    if(brojac>=12 && !pritisnutoDugme){
      state = FAULT_STATE;
      brojac = 0;
    }
    else if(brojac>=12 && pritisnutoDugme){
      pokrenutoBrojanje = false;
      pritisnutoDugme = false;
      brojac = 0;
    }
  }
  if(greska){
      if(digitalRead(LED_RED)){
        digitalWrite(LED_RED,LOW);
      }
      else{
        digitalWrite(LED_RED,HIGH);
      }
  }
  TCNT1 = 0;
}