#include <SoftwareSerial.h>

// Pines de comunicación SoftwareSerial (RX, TX)
SoftwareSerial mySerial(10, 11);

// Definición de pines físicos
const int PIN_LED_VERDE = 9; // LED del temporizador (PWM)
const int PIN_LIBRE_ADC = A5; // Pin analógico flotante para la semilla aleatoria

// Estados del Maestro (enum)
enum EstadoJuego {
  INACTIVO,
  JUGANDO,
  VICTORIA,
  DERROTA
};

EstadoJuego estadoActual = INACTIVO;

// Parámetros de la grilla y del tesoro
int tesoroX = 0; // Esquina superior derecha del tesoro (2x2)
int tesoroY = 0;
int coordX_Esclavo = 0;
int coordY_Esclavo = 0;

// Temporizadores (millis)
unsigned long tiempoInicioPartida = 0;
const unsigned long DURACION_PARTIDA = 60000; // 60 segundos configurables

unsigned long tiempoSobreTesoro = 0;
bool estabaSobreTesoro = false;
const unsigned long TIEMPO_PERMANENCIA = 2000; // 2000 ms continuos para ganar

unsigned long ultimoEnvio = 0;
const unsigned long INTERVALO_ENVIO = 50;

unsigned long ultimoByteRecibido = 0;
const unsigned long TIMEOUT_ENLACE = 2000; // 2000 ms
bool enlacePerdido = false;

void setup() {
  pinMode(PIN_LED_VERDE, OUTPUT);
  Serial.begin(9600);
  mySerial.begin(9600);

  // Inicializar semilla aleatoria usando pin analógico libre
  randomSeed(analogRead(PIN_LIBRE_ADC));
  ultimoByteRecibido = millis();
}

// Sortea la posición del tesoro garantizando comenza en frío (d >= 11)
void inicializarPartida() {
  tesoroX = random(12, 101); // Coordenadas entre 12 y 100
  tesoroY = random(12, 101);
  tiempoInicioPartida = millis();
  estabaSobreTesoro = false;
  estadoActual = JUGANDO;
}

// Calcula la distancia absoluta a la celda más cercana del tesoro 2x2
int calcularDistanciaEje(int pos, int tPos) {
  if (pos == tPos || pos == (tPos - 1)) {
    return 0; // Dentro del tesoro
  } else if (pos < (tPos - 1)) {
    return (tPos - 1) - pos;
  } else {
    return pos - tPos;
  }
}

// Convierte la distancia en un nivel discreto según la Tabla 3
byte obtenerNivelProximidad(int d) {
  if (d > 10)       return 0; // FRÍO
  else if (d >= 5)  return 1; // TIBIO
  else if (d >= 1)  return 2; // CALIENTE
  else              return 3; // AQUÍ (d = 0)
}

void loop() {
  unsigned long currentMillis = millis();

  // 1. Recepción no bloqueante desde el Esclavo
  while (mySerial.available() > 0) {
    byte b = mySerial.read();
    ultimoByteRecibido = currentMillis;
    enlacePerdido = false;

    if (b <= 100) {
      coordX_Esclavo = b;
    } else if (b >= 101 && b <= 200) {
      coordY_Esclavo = b - 100;
    } else if (b == 254) { // Solicitud de inicio
      if (estadoActual == INACTIVO) {
        inicializarPartida();
      }
    }
  }

  // 2. Control de Pérdida de Enlace (Timeout > 2000 ms)
  if (currentMillis - ultimoByteRecibido > TIMEOUT_ENLACE) {
    enlacePerdido = true;
    estadoActual = INACTIVO;
    // Parpadeo rápido del LED verde (250 ms)
    digitalWrite(PIN_LED_VERDE, (currentMillis / 250) % 2);
    return;
  }

  // 3. Máquina de Estados del Maestro
  switch (estadoActual) {
    case INACTIVO:
      analogWrite(PIN_LED_VERDE, 0); // LED verde apagado
      if (currentMillis - ultimoEnvio >= INTERVALO_ENVIO) {
        ultimoEnvio = currentMillis;
        mySerial.write((byte)240); // Código INACTIVO
      }
      break;

    case JUGANDO: {
      unsigned long tiempoTranscurrido = currentMillis - tiempoInicioPartida;

      // Verificar si expiró el tiempo (Derrota)
      if (tiempoTranscurrido >= DURACION_PARTIDA) {
        estadoActual = DERROTA;
        break;
      }

      // Atenuar el LED verde proporcionalmente al tiempo restante
      int pwmVerde = map(DURACION_PARTIDA - tiempoTranscurrido, 0, DURACION_PARTIDA, 0, 255);
      analogWrite(PIN_LED_VERDE, constrain(pwmVerde, 0, 255));

      // Calcular distancias y niveles por eje
      int dX = calcularDistanciaEje(coordX_Esclavo, tesoroX);
      int dY = calcularDistanciaEje(coordY_Esclavo, tesoroY);

      byte nivelX = obtenerNivelProximidad(dX);
      byte nivelY = obtenerNivelProximidad(dY);

      // Verificación de la condición de victoria por permanencia (2000 ms)
      if (dX == 0 && dY == 0) {
        if (!estabaSobreTesoro) {
          estabaSobreTesoro = true;
          tiempoSobreTesoro = currentMillis;
        } else if (currentMillis - tiempoSobreTesoro >= TIEMPO_PERMANENCIA) {
          estadoActual = VICTORIA;
        }
      } else {
        estabaSobreTesoro = false;
      }

      // Enviar los niveles codificados al Esclavo
      if (currentMillis - ultimoEnvio >= INTERVALO_ENVIO) {
        ultimoEnvio = currentMillis;
        byte mensajeNivel = (nivelX << 2) | (nivelY & 0x03);
        mySerial.write(mensajeNivel);
      }
      break;
    }

    case VICTORIA:
      mySerial.write((byte)241); // Enviar código de VICTORIA
      analogWrite(PIN_LED_VERDE, 255); // LED verde al 100% fijo
      delay(5000); // Duración de la animación
      estadoActual = INACTIVO;
      break;

    case DERROTA:
      mySerial.write((byte)242); // Enviar código de DERROTA
      // Parpadeo del LED verde 10 veces (250 ms ON / 250 ms OFF)
      for (int i = 0; i < 10; i++) {
        digitalWrite(PIN_LED_VERDE, HIGH);
        delay(250);
        digitalWrite(PIN_LED_VERDE, LOW);
        delay(250);
      }
      estadoActual = INACTIVO;
      break;
  }
}