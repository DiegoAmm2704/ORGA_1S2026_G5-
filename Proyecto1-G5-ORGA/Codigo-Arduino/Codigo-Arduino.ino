#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

// LCD I2C
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Bluetooth HC-05 / HC-06
SoftwareSerial bluetooth(A0, A1); // RX, TX

// Servo puerta
Servo puerta;

// LEDs de ambientes
const int LED_SALA = 2;
const int LED_COMEDOR = 3;
const int LED_COCINA = 4;
const int LED_BANO = 5;
const int LED_HABITACION = 6;

// LEDs de estado
const int LED_AZUL = 7;
const int LED_VERDE = 8;
const int LED_ROJO = 9;

// Ventilador
const int VENTILADOR = 10;

// Servo y boton
const int PIN_SERVO = 11;
const int BOTON_PUERTA = 12;

// DIP switches
const int DIP_GRUPO_1 = A2; // Sala + Comedor
const int DIP_GRUPO_2 = A3; // Cocina + Baño + Habitacion

// Variables
bool modoAutomatico = false;
bool fiestaActiva = false;
bool puertaAbierta = false;
bool estadoAnteriorBoton = HIGH;

unsigned long tiempoFiestaAnterior = 0;
int ledFiestaActual = 0;

void setup() {
  Serial.begin(9600);
  bluetooth.begin(9600);

  lcd.init();
  lcd.backlight();

  pinMode(LED_SALA, OUTPUT);
  pinMode(LED_COMEDOR, OUTPUT);
  pinMode(LED_COCINA, OUTPUT);
  pinMode(LED_BANO, OUTPUT);
  pinMode(LED_HABITACION, OUTPUT);

  pinMode(LED_AZUL, OUTPUT);
  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_ROJO, OUTPUT);

  pinMode(VENTILADOR, OUTPUT);
  pinMode(BOTON_PUERTA, INPUT_PULLUP);

  pinMode(DIP_GRUPO_1, INPUT_PULLUP);
  pinMode(DIP_GRUPO_2, INPUT_PULLUP);

  puerta.attach(PIN_SERVO);
  puerta.write(0); // puerta cerrada al iniciar

  digitalWrite(LED_AZUL, HIGH);
  digitalWrite(LED_VERDE, LOW);
  digitalWrite(LED_ROJO, LOW);

  modoAutomatico = false;
  fiestaActiva = false;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Sistema activo");
  lcd.setCursor(0, 1);
  lcd.print("Modo manual");

  bluetooth.println("Sistema activo - modo manual");
}

void loop() {
  leerBluetooth();
  leerSerial();
  controlarPuerta();

  if (!modoAutomatico) {
    leerDipsManual();
  }

  if (fiestaActiva) {
    lucesFiesta();
  }
}

// ================= BLUETOOTH / SERIAL =================

void leerBluetooth() {
  if (bluetooth.available()) {
    String comando = bluetooth.readStringUntil('\n');
    comando.trim();
    comando.toLowerCase();
    procesarComando(comando);
  }
}

void leerSerial() {
  if (Serial.available()) {
    String comando = Serial.readStringUntil('\n');
    comando.trim();
    comando.toLowerCase();
    procesarComando(comando);
  }
}

void procesarComando(String comando) {
  digitalWrite(LED_ROJO, LOW);

  if (comando == "modo_fiesta") {
    modoFiesta();
  }
  else if (comando == "modo_relajado") {
    modoRelajado();
  }
  else if (comando == "modo_noche") {
    modoNoche();
  }
  else if (comando == "encender_todo") {
    encenderTodo();
  }
  else if (comando == "apagar_todo") {
    apagarTodo();
  }
  else if (comando == "modo_manual") {
    activarModoManual();
  }
  else if (comando == "estado") {
    mostrarEstado();
  }
  else {
    errorComando();
  }
}

// ================= MODO MANUAL CON DIP =================

void leerDipsManual() {
  fiestaActiva = false;

  // Con INPUT_PULLUP:
  // DIP cerrado = LOW = encendido
  // DIP abierto = HIGH = apagado

  if (digitalRead(DIP_GRUPO_1) == LOW) {
    digitalWrite(LED_SALA, HIGH);
    digitalWrite(LED_COMEDOR, HIGH);
  } else {
    digitalWrite(LED_SALA, LOW);
    digitalWrite(LED_COMEDOR, LOW);
  }

  if (digitalRead(DIP_GRUPO_2) == LOW) {
    digitalWrite(LED_COCINA, HIGH);
    digitalWrite(LED_BANO, HIGH);
    digitalWrite(LED_HABITACION, HIGH);
  } else {
    digitalWrite(LED_COCINA, LOW);
    digitalWrite(LED_BANO, LOW);
    digitalWrite(LED_HABITACION, LOW);
  }

  digitalWrite(VENTILADOR, LOW);
}

void activarModoManual() {
  modoAutomatico = false;
  fiestaActiva = false;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Modo manual");
  lcd.setCursor(0, 1);
  lcd.print("DIP activo");

  bluetooth.println("Modo manual activado");
  parpadearVerde();
}

// ================= MODOS BLUETOOTH =================

void modoFiesta() {
  modoAutomatico = true;
  fiestaActiva = true;

  digitalWrite(VENTILADOR, HIGH);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Modo: FIESTA");
  lcd.setCursor(0, 1);
  lcd.print("LEDs alternan");

  bluetooth.println("Modo fiesta activado");
  parpadearVerde();
}

void modoRelajado() {
  modoAutomatico = true;
  fiestaActiva = false;

  digitalWrite(LED_SALA, HIGH);
  digitalWrite(LED_COMEDOR, LOW);
  digitalWrite(LED_COCINA, LOW);
  digitalWrite(LED_BANO, LOW);
  digitalWrite(LED_HABITACION, HIGH);

  digitalWrite(VENTILADOR, LOW);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Modo: RELAJADO");
  lcd.setCursor(0, 1);
  lcd.print("Vent: OFF");

  bluetooth.println("Modo relajado activado");
  parpadearVerde();
}

void modoNoche() {
  modoAutomatico = true;
  fiestaActiva = false;

  digitalWrite(LED_SALA, LOW);
  digitalWrite(LED_COMEDOR, LOW);
  digitalWrite(LED_COCINA, LOW);
  digitalWrite(LED_BANO, LOW);
  digitalWrite(LED_HABITACION, LOW);

  digitalWrite(VENTILADOR, LOW);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Modo: NOCHE");
  lcd.setCursor(0, 1);
  lcd.print("Todo apagado");

  bluetooth.println("Modo noche activado");
  parpadearVerde();
}

void encenderTodo() {
  modoAutomatico = true;
  fiestaActiva = false;

  digitalWrite(LED_SALA, HIGH);
  digitalWrite(LED_COMEDOR, HIGH);
  digitalWrite(LED_COCINA, HIGH);
  digitalWrite(LED_BANO, HIGH);
  digitalWrite(LED_HABITACION, HIGH);

  digitalWrite(VENTILADOR, HIGH);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("LEDs: ON");
  lcd.setCursor(0, 1);
  lcd.print("Vent: ON");

  bluetooth.println("Todo encendido");
  parpadearVerde();
}

void apagarTodo() {
  modoAutomatico = true;
  fiestaActiva = false;

  digitalWrite(LED_SALA, LOW);
  digitalWrite(LED_COMEDOR, LOW);
  digitalWrite(LED_COCINA, LOW);
  digitalWrite(LED_BANO, LOW);
  digitalWrite(LED_HABITACION, LOW);

  digitalWrite(VENTILADOR, LOW);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("LEDs: OFF");
  lcd.setCursor(0, 1);
  lcd.print("Vent: OFF");

  bluetooth.println("Todo apagado");
}

// ================= MODO FIESTA =================

void lucesFiesta() {
  unsigned long tiempoActual = millis();

  if (tiempoActual - tiempoFiestaAnterior >= 300) {
    tiempoFiestaAnterior = tiempoActual;

    digitalWrite(LED_SALA, LOW);
    digitalWrite(LED_COMEDOR, LOW);
    digitalWrite(LED_COCINA, LOW);
    digitalWrite(LED_BANO, LOW);
    digitalWrite(LED_HABITACION, LOW);

    if (ledFiestaActual == 0) {
      digitalWrite(LED_SALA, HIGH);
    }
    else if (ledFiestaActual == 1) {
      digitalWrite(LED_COMEDOR, HIGH);
    }
    else if (ledFiestaActual == 2) {
      digitalWrite(LED_COCINA, HIGH);
    }
    else if (ledFiestaActual == 3) {
      digitalWrite(LED_BANO, HIGH);
    }
    else if (ledFiestaActual == 4) {
      digitalWrite(LED_HABITACION, HIGH);
    }

    ledFiestaActual++;

    if (ledFiestaActual > 4) {
      ledFiestaActual = 0;
    }
  }
}

// ================= PUERTA =================

void controlarPuerta() {
  bool estadoBoton = digitalRead(BOTON_PUERTA);

  if (estadoBoton == LOW && estadoAnteriorBoton == HIGH) {
    delay(50);

    if (!puertaAbierta) {
      puerta.write(90);
      puertaAbierta = true;

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Puerta abierta");

      bluetooth.println("Puerta abierta");
    } 
    else {
      puerta.write(0);
      puertaAbierta = false;

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Puerta cerrada");

      bluetooth.println("Puerta cerrada");
    }
  }

  estadoAnteriorBoton = estadoBoton;
}

// ================= ESTADO Y ERROR =================

void mostrarEstado() {
  lcd.clear();
  lcd.setCursor(0, 0);

  if (modoAutomatico) {
    lcd.print("Modo automatico");
    bluetooth.println("Modo actual: automatico");
  } else {
    lcd.print("Modo manual");
    bluetooth.println("Modo actual: manual");
  }

  lcd.setCursor(0, 1);

  if (puertaAbierta) {
    lcd.print("Puerta abierta");
    bluetooth.println("Puerta: abierta");
  } else {
    lcd.print("Puerta cerrada");
    bluetooth.println("Puerta: cerrada");
  }
}

void errorComando() {
  fiestaActiva = false;

  digitalWrite(LED_ROJO, HIGH);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ERROR:");
  lcd.setCursor(0, 1);
  lcd.print("Comando invalido");

  bluetooth.println("ERROR: comando invalido");
}

void parpadearVerde() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_VERDE, HIGH);
    delay(150);
    digitalWrite(LED_VERDE, LOW);
    delay(150);
  }
}