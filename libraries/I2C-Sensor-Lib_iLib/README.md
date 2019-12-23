I2C-Sensor-Lib (iLib)
====

Library for I2C sensors and some atmel-specific functions. The following sensors can be used with an uniform interface and come with arduino-examples: 

- Austria Microsystems TCS3772: light sensor - RGB and clear
- Silicon Labs SI7021: humidity sensor
- Invensense MPU9250: 9DOF - 3 axis acceleration and gyro PLUS AK8963-IC with magnetic-field sensor
- Freescale MPL3115A2: pressure
- Maxim MAX44009: ambient and lux with incredible wide dynamic
- NXP PCF2127: Realtime-Clock with 2ppm 
- Bosch BMP280: pressure
- ST L3G-Series: 3 axis gyro / angular rate
- Freescale MAG3110: 3 axis Compass / Magnetic field
- Freescale MMA8451: 3 axis acceleration
- Fairchild FAN5421: Single-Cell Li-Ion Switching Charger
- STM LPS331: Pressure Sensor
- Maxim MAX17047: Fuel Gauge for various Cells

Additional Features:
- AVR: measure VCC of the power-pin
- AVR: emulate a spektrum-serial control
- fast math fn for e-function, power-of with scaling (interpolation with sampling points)
- HDLC: a protocol for serial communication featuring frames, checksum and auto-escaping of characters
- WS2812B: efficient controller