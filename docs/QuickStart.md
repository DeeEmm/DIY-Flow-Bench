## Getting Started

### VSCode
---

DIYFB is built using VScode and platformIO. Dependencies are automatically pulled in as part of the supplied platfformio.ini file. Whilst it is possible to compile using the Arduino IDE, VScode is the recommended build environment

In addition to setting up platformIO, you will also need to install python as this is used to manage the versioning. If you cannot or do not wish to install python, or you are using the Arduino IDE, then you can manually update the versioning by uncommenting the build_flags in the [common] section and commenting out the call to user-actions.py

### Default Build Environment
---

Witin platformIO.ini you will see that there are several build environments already set up for different baords. Generally building the [esp32dev] environment will work for most ESP32 boards. But if for some reason it doesn't and you need to create a custom environemnt you can copy and past the default [env:myESP32board] section as the basis for your new setup.

Don't forget to change the default_envs = esp32dev to your board in the [platformIO] section

### Compiling and uploading the firmware
---

Invoking the 'Upload' command for your environment in VScode will compile and upload the firmware to your device. 

Altenratively you can also upload the pre-compiled binary firmware.bin directly using a third party uploader.

Once uploaded you will then need to upload the GUI and confguration files.


### Setting up your pin configuration 
---

Before you can use the device the I/O configuration needs to be set. Pins are defined on the 'pins' page of the GUI. There you can change the input and output pin numbers to suit your hardware.

Note that the default recommended board is the Wemos ESP32 D1. This is the board that the DIYFB shield is designed for. Generic versions are available very cheaply online.

### Configuring the bench - configuration.json
---

If you are using the code on a custom shield or you are using different sensors to those recommended then you will need to change the configuration to suit. 

The firmware currently supports MAF and differential based benches

### Translations - XX_language.json
---
The core messaging and descriptor within the GUI can be translated by uploading a language.json file. This will then override the default descriptions contained in the firmware. An english example EN_language.json file is provided as the basis for all translations. Simply rename using the international standard to replace the EN prefix.