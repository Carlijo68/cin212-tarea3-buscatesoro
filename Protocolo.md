# Protocolo de Comunicación Serial - Tarea 3

## Descripción General
Comunicación bidireccional asíncrona mediante mensajes de 1 byte a 9600 baudios, empleando `SoftwareSerial` con pines dedicados, conexión cruzada TX-RX y GND común[cite: 3].

## Formato de Mensajes (1 Byte)
La información se transmite en formato binario mediante `write()`[cite: 3]:

* **Coordenada X (Esclavo -> Maestro):** Valores de 0 a 100[cite: 2, 3]. Se envían periódicamente (máximo cada 50 ms)[cite: 3].
* **Coordenada Y (Esclavo -> Maestro):** Valores de 0 a 100[cite: 2, 3]. Se envían periódicamente junto con X[cite: 3].
* **Solicitud de Inicio (Esclavo -> Maestro):** Enviado una única vez por pulsación, exclusivamente en estado INACTIVO[cite: 2, 3].
* **Estado del Juego (Maestro -> Esclavo):** Envío periódico del estado actual (INACTIVO, JUGANDO, VICTORIA, DERROTA)[cite: 2, 3].
* **Nivel de Proximidad (Maestro -> Esclavo):** Códigos de nivel por eje (0: FRÍO, 1: TIBIO, 2: CALIENTE, 3: AQUÍ) enviados durante el estado JUGANDO[cite: 2, 3].
* **Valores Inválidos:** Cualquier otro valor fuera de rango se descarta sin afectar la ejecución[cite: 3].

## Consideraciones de Enlace
* **Recepción no bloqueante:** Procesamiento mediante bucles condicionales con `available()` y `read()`[cite: 3].
* **Timeout de enlace:** Si transcurren más de 2000 ms sin recibir un byte válido, se detecta pérdida de enlace[cite: 2, 3].