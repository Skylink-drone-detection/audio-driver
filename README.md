# Audio Drivers for Drone Detection

## Instalacja i konfiguracja
*Narazie brak punktu wejściowego*

Konfiguracja wymaga manualnego pobrania biblioteki WiringPi
### 1. WiringPi (GPIO Library) mozna uzyc bcm2835.h(szybcze juz uzywane w adc)
**Uwaga:** `sudo apt install wiringpi` **NIE DZIAŁA**

### Oficjalna instalacja z Git
```
cd ~
git clone https://github.com/WiringPi/WiringPi.git
cd WiringPi
sudo ./build
```

### Sprawdzenie instalacji
```
gpio -v
```
## Dokumentacja
Projekt składa się z punktu wejściowego **main.c**
oraz modułów zawartych w katalogach include z implementacją w src.

### shm_reader
Moduł przeznaczony do komunikacji między procesami - procesem pobierającym
dane z sensorów (Sterowniki w C)
oraz procesem przetwarzającym dane (Python).
Wykorzystywana jest pamięć dzielona RAM (shared memory) w celu
szybkiego zapisu informacji.
Dane są umieszczane przez proces pobierający w buforze pierścieniowym
w indeksie `head`.

### Biblioteka bcm28355
sudo apt update
sudo apt install libbcm2835-dev  # Dla MCP3564
sudo apt install cmake build-essential git

