#include <Adafruit_NeoPixel.h>

// Pines de los WS2813
#define PIN_TORSO 9
#define PIN_CABEZA 10

// Cantidad de LEDs
#define NUM_LEDS_TORSO 78
#define NUM_LEDS_CABEZA 32

// Objetos NeoPixel
Adafruit_NeoPixel stripTorso(NUM_LEDS_TORSO, PIN_TORSO, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel stripCabeza(NUM_LEDS_CABEZA, PIN_CABEZA, NEO_GRB + NEO_KHZ800);

// Color único para ambas tiras (modifica aquí el color deseado)
const int COLOR_R = 200;  // Rojo
const int COLOR_G = 0;  // Verde
const int COLOR_B = 15;   // Azul

// Botones
const int boton1 = 2; // Torso
const int boton2 = 4; // Cabeza
const int button_pin = 7;
const int motor_pin = 8;

unsigned long tiempoInicio1 = 0;
unsigned long tiempoInicio2 = 0;

unsigned long tiempoFade1 = 0;
unsigned long tiempoFade2 = 0;

bool estadoBoton1Actual = HIGH;
bool estadoBoton1Anterior = HIGH;
bool estadoBoton2Actual = HIGH;
bool estadoBoton2Anterior = HIGH;

int button_state;

unsigned long duracion1 = 0;
unsigned long duracion2 = 0;

enum EstadoTorso { APAGADO, ENCENDIENDO, CICLO, ESPECIAL, APAGANDO };
EstadoTorso estadoTorso = APAGADO;

enum EstadoCabeza { APAGADO_CABEZA, ENCENDIENDO_CABEZA, APAGANDO_CABEZA };
EstadoCabeza estadoCabeza = APAGADO_CABEZA;

int brilloLed1 = 0;
int brilloLed2 = 0;

const unsigned long debounceDelay = 50;
unsigned long ultimoCambioBoton1 = 0;
unsigned long ultimoCambioBoton2 = 0;

const unsigned long velocidadFadeEncendidoTorso = 100;
const unsigned long velocidadFadeEncendidoTorso2 = 10;
const unsigned long velocidadFadeApagadoTorso = 50;
const unsigned long velocidadFadeApagadoTorso2 = 50;
const int pasoFadeTorso = 5;
const unsigned long velocidadFadeEncendidoCabeza = 100;
const unsigned long velocidadFadeApagadoCabeza = 50;
const int pasoFadeCabeza = 5;

void setup() {
  stripTorso.begin();
  stripTorso.show();
  stripCabeza.begin();
  stripCabeza.show();

  pinMode(boton1, INPUT_PULLUP);
  pinMode(boton2, INPUT_PULLUP);
  pinMode(button_pin, INPUT);
  pinMode(motor_pin, OUTPUT);

  Serial.begin(9600);
}

void loop() {
  unsigned long ahora = millis();

  bool lecturaBoton1 = digitalRead(boton1);
  bool lecturaBoton2 = digitalRead(boton2);

  if (lecturaBoton1 != estadoBoton1Anterior) {
    ultimoCambioBoton1 = ahora;
  }
  if ((ahora - ultimoCambioBoton1) > debounceDelay) {
    if (lecturaBoton1 != estadoBoton1Actual) {
      estadoBoton1Actual = lecturaBoton1;
      if (estadoBoton1Actual == LOW) {
        tiempoInicio1 = ahora;
      } else {
        duracion1 = ahora - tiempoInicio1;
        procesarBoton1();
      }
    }
  }
  estadoBoton1Anterior = lecturaBoton1;

  if (lecturaBoton2 != estadoBoton2Anterior) {
    ultimoCambioBoton2 = ahora;
  }
  if ((ahora - ultimoCambioBoton2) > debounceDelay) {
    if (lecturaBoton2 != estadoBoton2Actual) {
      estadoBoton2Actual = lecturaBoton2;
      if (estadoBoton2Actual == LOW) {
        tiempoInicio2 = ahora;
      } else {
        duracion2 = ahora - tiempoInicio2;
        procesarBoton2();
      }
    }
  }
  estadoBoton2Anterior = lecturaBoton2;

  if (estadoBoton1Actual == LOW && estadoBoton2Actual == LOW) {
    estadoTorso = ESPECIAL;
  }

  actualizarTorso(ahora);
  actualizarCabeza(ahora);
  
  button_state = digitalRead(button_pin);  // read button state
  if(button_state == HIGH){           // if button is pushed
    digitalWrite(motor_pin,HIGH);     // turn motor on
    Serial.println("ON");
  }
  else{                               // if button is not pushed
    digitalWrite(motor_pin,LOW);     // turn motor off
    Serial.println("OFF");
  }
}

void procesarBoton1() {
  if (estadoBoton2Actual == LOW) return;

  if (duracion1 >= 2000) {
    estadoTorso = APAGANDO;
  } else if (estadoTorso == APAGADO) {
    estadoTorso = ENCENDIENDO;
  } else if (estadoTorso == ENCENDIENDO) {
    estadoTorso = CICLO;
  }
}

void procesarBoton2() {
  if (estadoBoton1Actual == LOW) return;

  if (duracion2 >= 2000) {
    estadoCabeza = APAGANDO_CABEZA;
  } else if (estadoCabeza == APAGADO_CABEZA) {
    estadoCabeza = ENCENDIENDO_CABEZA;
  }
}

void actualizarTorso(unsigned long ahora) {
  static int direccion = pasoFadeTorso;
  static bool subiendo = true;

  if (estadoTorso == APAGADO) {
    setColorTorso(0);
    brilloLed1 = 0;
  }

  if (estadoTorso == ENCENDIENDO) {
    if (ahora - tiempoFade1 >= velocidadFadeEncendidoTorso) {
      tiempoFade1 = ahora;
      brilloLed1 += pasoFadeTorso;
      if (brilloLed1 > 255) brilloLed1 = 255;
      setColorTorso(brilloLed1);
    }
  }

  if (estadoTorso == CICLO) {
    unsigned long velocidadActual = subiendo ? velocidadFadeEncendidoTorso : velocidadFadeApagadoTorso;
    if (ahora - tiempoFade1 >= velocidadActual) {
      tiempoFade1 = ahora;
      brilloLed1 += direccion;
      if (brilloLed1 >= 255) {
        brilloLed1 = 255;
        direccion = -pasoFadeTorso;
        subiendo = false;
      }
      if (brilloLed1 <= 0) {
        brilloLed1 = 0;
        direccion = pasoFadeTorso;
        subiendo = true;
        delay(200);
      }
      setColorTorso(brilloLed1);
    }
  }

  if (estadoTorso == ESPECIAL) {
    unsigned long velocidadActual = subiendo ? velocidadFadeEncendidoTorso2 : velocidadFadeApagadoTorso2;
    if (ahora - tiempoFade1 >= velocidadActual) {
      tiempoFade1 = ahora;
      brilloLed1 += direccion;
      if (brilloLed1 >= 255) {
        brilloLed1 = 255;
        direccion = -pasoFadeTorso;
        subiendo = false;
      }
      if (brilloLed1 <= 0) {
        brilloLed1 = 0;
        direccion = pasoFadeTorso;
        subiendo = true;
        delay(200);
      }
      setColorTorso(brilloLed1);
    }
  }

  if (estadoTorso == APAGANDO) {
    if (brilloLed1 > 0 && (ahora - tiempoFade1 >= velocidadFadeApagadoTorso)) {
      tiempoFade1 = ahora;
      brilloLed1 -= pasoFadeTorso;
      if (brilloLed1 < 0) brilloLed1 = 0;
      setColorTorso(brilloLed1);
    }
    if (brilloLed1 == 0) {
      estadoTorso = APAGADO;
    }
  }
}

void actualizarCabeza(unsigned long ahora) {
  static int direccion2 = pasoFadeCabeza;

  if (estadoCabeza == APAGADO_CABEZA) {
    setColorCabeza(0);
    brilloLed2 = 0;
  }

  if (estadoCabeza == ENCENDIENDO_CABEZA) {
    if (ahora - tiempoFade2 >= velocidadFadeEncendidoCabeza) {
      tiempoFade2 = ahora;
      brilloLed2 += direccion2;
      if (brilloLed2 > 255) brilloLed2 = 255;
      setColorCabeza(brilloLed2);
    }
  }

  if (estadoCabeza == APAGANDO_CABEZA) {
    if (brilloLed2 > 0 && (ahora - tiempoFade2 >= velocidadFadeApagadoCabeza)) {
      tiempoFade2 = ahora;
      brilloLed2 -= pasoFadeCabeza;
      if (brilloLed2 < 0) brilloLed2 = 0;
      setColorCabeza(brilloLed2);
    }
    if (brilloLed2 == 0) {
      estadoCabeza = APAGADO_CABEZA;
    }
  }

  button_state = digitalRead(button_pin);
  if (button_state == HIGH) {
    digitalWrite(motor_pin, HIGH);
    Serial.println("ON");
  } else {
    digitalWrite(motor_pin, LOW);
    Serial.println("OFF");
  }
}

void setColorTorso(int brightness) {
  uint32_t color = stripTorso.Color((COLOR_R * brightness) / 255, (COLOR_G * brightness) / 255, (COLOR_B * brightness) / 255);
  for (int i = 0; i < NUM_LEDS_TORSO; i++) {
    stripTorso.setPixelColor(i, color);
  }
  stripTorso.show();
}

void setColorCabeza(int brightness) {
  uint32_t color = stripCabeza.Color((COLOR_R * brightness) / 255, (COLOR_G * brightness) / 255, (COLOR_B * brightness) / 255);
  for (int i = 0; i < NUM_LEDS_CABEZA; i++) {
    stripCabeza.setPixelColor(i, color);
  }
  stripCabeza.show();
}
