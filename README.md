# GPS-fancy-display
 M5StickC GPS unit displays


Requires the [TinyGPS++ Lib ](http://arduiniana.org/libraries/tinygpsplus/) and the [GPS unit](https://shop.m5stack.com/products/mini-gps-bds-unit)

# Controls

- Holding BtnA cycles through the screens; Positional/Speed/Heading/Time/Battery status/Distance-to
- Tapping BtnB cycles through secondary displays, eg speed units. It also forces a GUI redraw so on screens without secondary displays can be used as a 'force-screen-update'.

## 'Distance-to' screen

The `Distance-to` screen shows the distance to a given lat/lon. It is possible to set any co-ordinate with 1dp precision. The controls cycle through:

- Add 0.1
- Add 10
- Invert sign (- >> +, + >> -)

First for Lat, then for Lon. The text highlighting should make it clear what is being changed. So, to set Lat to `-31.2` one would do the following:

- Change to the distance-to screen by holding `BtnA`
- Tap `BtnA` twelve times to add 1.2 total
- Tap `BtnB` to switch to 'Add 10' mode and tap `BtnA` three times to add a total of 30
- Tap `BtnB` to switch to 'Invert Sign' mode and tap `BtnA` once to switch from 31.2 to -31.2

![Distance-to screen](https://i.imgur.com/jnB24MG.jpeg)


## Serial

NMEA sentences are output verbatim to serial at `9600` baud.

A PowerShell script, `SerialPortReader.ps1`, is included that can stream from the COM port to a file (which should be openable in, for example, Google Earth Pro).

Example usage:

```PowerShell
C:\Projects\GPS-fancy-display> .\SerialPortReader.ps1

cmdlet SerialPortReader.ps1 at command pipeline position 1
Supply values for the following parameters:
PortName: COM3
BaudRate: 9600
Checking PortName...
--> PortName COM3 is available
Establishing connection to the port...
[...]
```