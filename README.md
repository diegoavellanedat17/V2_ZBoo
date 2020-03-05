# V2_ZBoo
Configuration and files V2

### Instalacion de Raspbian via NOOBS

[Link de instalación completa para el Raspbian](https://projects.raspberrypi.org/en/projects/raspberry-pi-setting-up/2)

### Display SSD1306 raspberry 

[Link para funcionamiento del Display](https://learn.adafruit.com/monochrome-oled-breakouts/python-usage-2)

## Serial Comunication

[Comunicación Serial](https://www.electronicwings.com/raspberry-pi/raspberry-pi-uart-communication-using-python-and-c)

## Raspberry pi Camera
[Raspberri Camera tutorial](https://projects.raspberrypi.org/en/projects/getting-started-with-picamera/2)
[Upload and Download Image Firebase](https://www.youtube.com/watch?v=I1eskLk0exg)

## Beacon Scanner 
En ocasiones cuando se quiere conectar a un BEACON en calidad de cliente, se queda un tiempo muy largo esperando y est ohace que no siga escaneando por lo tanto en la libreria en el archivo FreeRTOS.cpp se cambia la linea 68 por 
```
xSemaphoreTake(m_semaphore,15000) // anteriormente no era 15000 sino un valor que daba alrededor de 50 días
```
