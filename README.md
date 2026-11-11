# cin212-tarea3-buscatesoro
# Tarea 3: Búsqueda del Tesoro en dos Arduino UNO

**Asignatura:** CIN-212 Hardware Digital  
**Plataforma:** Arduino UNO (Maestro y Esclavo)  
**Comunicación:** Serial Asíncrona Binaria a 9600 Baudios  

---

## 📖 Descripción del Proyecto
Este proyecto implementa el juego interactivo de la **Búsqueda del Tesoro** repartido en dos placas Arduino UNO independientes (Maestro y Esclavo) conectadas mediante comunicación serial asíncrona.

- **Esclavo (Módulo Joystick):** Lee la posición de dos potenciómetros (ejes X e Y), procesa la señal mediante conversión ADC uniforme y filtrado por software, detecta la pulsación del botón de inicio con antirrebote y controla dos LEDs PWM para indicar la proximidad al tesoro.
- **Maestro (Módulo Juego):** Implementa la máquina de estados principal (`INACTIVO`, `JUGANDO`, `VICTORIA`, `DERROTA`), genera aleatoriamente la ubicación de un tesoro de $2\times2$ celdas en una grilla de $100\times100$, gestiona un temporizador de 60 segundos con atenuación progresiva en un LED verde (PWM) y evalúa la condición de victoria por permanencia (2000 ms).

---

## 🛠️ Arquitectura y Diagrama Esquemático

### Conexiones de Hardware

#### Esclavo (Joystick)
- **Potenciómetro X:** Pin Central $\rightarrow$ `A0` (Extremos a 5V y GND)
- **Potenciómetro Y:** Pin Central $\rightarrow$ `A1` (Extremos a 5V y GND)
- **Pulsador de Inicio:** Pin `2` $\rightarrow$ GND (Modo `INPUT_PULLUP`)
- **LED Indicador X:** Pin `3` (PWM) con resistencia de $220\,\Omega$ a GND
- **LED Indicador Y:** Pin `5` (PWM) con resistencia de $220\,\Omega$ a GND
- **SoftwareSerial:** Pin `10` (RX), Pin `11` (TX)

#### Maestro (Juego)
- **LED Temporizador Verde:** Pin `9` (PWM) con resistencia de $220\,\Omega$ a GND
- **Pin Flotante:** `A5` (utilizado para `randomSeed`)
- **SoftwareSerial:** Pin `10` (RX), Pin `11` (TX)

#### Enlace entre Placas
- **TX Esclavo (Pin 11)** $\rightarrow$ **RX Maestro (Pin 10)**
- **RX Esclavo (Pin 10)** $\rightarrow$ **TX Maestro (Pin 11)**
- **GND Esclavo** $\rightarrow$ **GND Maestro** (Tierra común)

---

## 📡 Protocolo de Comunicación Serial

La comunicación utiliza bytes binarios directos (`Serial.write()`) a 9600 baudios, garantizando un flujo eficiente y no bloqueante.

### Mensajes Esclavo $\rightarrow$ Maestro
| Dato | Rango / Byte | Descripción | Frecuencia |
| :--- | :--- | :--- | :--- |
| **Coordenada X** | `0` – `100` | Posición del eje X en la grilla | Periódico ($\le 50\text{ ms}$) |
| **Coordenada Y** | `101` – `200` | Posición del eje Y (`Y + 100`) | Periódico ($\le 50\text{ ms}$) |
| **Solicitud Inicio** | `254` (`0xFE`) | Petición para iniciar partida | Única vez por pulsación |

### Mensajes Maestro $\rightarrow$ Esclavo
| Estado / Nivel | Byte | Descripción |
| :--- | :--- | :--- |
| **INACTIVO** | `240` (`0xF0`) | Esperando inicio de juego; LEDs apagados |
| **VICTORIA** | `241` (`0xF1`) | Partida ganada; parpadeo sincrónico en Esclavo |
| **DERROTA** | `242` (`0xF2`) | Tiempo agotado; apagado de LEDs |
| **Nivel Proximidad** | `0xXY` | Nibble superior: Nivel X \| Nibble inferior: Nivel Y<br>*(0: FRÍO, 1: TIBIO, 2: CALIENTE, 3: AQUÍ)* |

---

## 🔄 Máquina de Estados (Maestro)

```text
       ┌──────────────┐
       │   INACTIVO   │ <────────────────────────┐
       └──────┬───────┘                          │
              │ Recibe Byte 254 (Pulsador)       │
              ▼                                  │
       ┌──────────────┐                          │
  ┌───>│   JUGANDO    │                          │
  │    └──────┬───────┘                          │
  │           │                                  │
  │           ├─── dX=0 & dY=0 por 2000 ms ────> ┌┴────────────┐
  │           │                                  │  VICTORIA   │
  │           └─── Tiempo >= 60 s ─────────────> ├─────────────┤
  │                                              │   DERROTA   │
  └──────────────────────────────────────────────┴─────────────┘
```

---

## 🚀 Instrucciones de Ejecución

1. Abrir la carpeta `esclavo/` en la IDE de Arduino y cargar `esclavo.ino` en la placa del Esclavo.
2. Abrir la carpeta `maestro/` en la IDE de Arduino y cargar `maestro.ino` en la placa del Maestro.
3. Asegurar la conexión física cruzada de los pines `10` y `11`, además del cable de masa unificada (GND a GND).
4. Configurar el Monitor Serial a **9600 baudios** para visualizar el estado y depuración de las placas.
5. Presionar el botón conectado al pin 2 del Esclavo para iniciar una nueva partida.