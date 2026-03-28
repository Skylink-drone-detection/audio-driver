# Audio Drivers for Drone Detection

## Instalacja i konfiguracja
```
cd build
rm -rf *
cmake ..
make -j4
```

## Dokumentacja
Projekt składa się z punktu wejściowego **main.c**
oraz modułów zawartych w katalogach include z implementacją w src.

### Biblioteki systemowe
```
sudo apt update
sudo apt install libgpiod-dev
sudo apt install cmake build-essential git
```

### GPIO i SPI
Sterownik używa teraz:
- `libgpiod` do GPIO
- `spidev` do SPI (`/dev/spidev0.0` lub `/dev/spidev0.1`)

Opcjonalne zmienne środowiskowe:
```
export AUDIO_DRIVER_GPIO_CHIP=/dev/gpiochip4
export AUDIO_DRIVER_SPI_DEVICE=/dev/spidev0.1
```

### shm_reader
Moduł przeznaczony do komunikacji między procesami - procesem pobierającym
dane z sensorów (Sterowniki w C)
oraz procesem przetwarzającym dane (Python).
Wykorzystywana jest pamięć dzielona RAM (shared memory) w celu
szybkiego zapisu informacji.
Dane są umieszczane przez proces pobierający w buforze pierścieniowym
w indeksie `head`.

### Odpalanie dokumentacji
```
cd docs/html
python3 -m http.server 8090
http://localhost:8090/files.html
```
