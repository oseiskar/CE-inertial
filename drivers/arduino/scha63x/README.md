# Murata SCHA63x driver for Arduino

Simple driver for the Murata SCHA63X series of IMUs for the Arduino board. The full data sheet for the sensor can be requested from [Murata](https://www.murata.com/en-global/products/sensor/gyro/overview/lineup/scha63t), also available on [docs](docs/murata/SCHA63T-K03-Datasheet-Full.pdf) folder. 

## Installing

Symlinks should be used with external Arduino libraries instead of directly importing the files into the libraries folder from the Arduino IDE. Check the default Arduino folder from Arduino IDE's preferences (default for macOS  `/Users/*usr*/Documents/Arduino`). Navigate to `libraries` subdirectory and create symlink to the local `scha63x` (`pwd` to get the path) folder there. The second argument of `ln` is the name of the symlink, i.e. name of the library. NOTE individual header files cannot be excluded when importing a library into Arduino sketch, this might cause conflicts with Arduino's own libraries.  

```bash
  ln -s /path/to/scha63x scha63x
```

## TODOs and current state

* All modules compile with Arduino IDE. Program fails with error code SCHA63X_ERR_TEST_MODE_ACTIVATION, probably bug in parsing the messages over SPI. 
* The `murata.ino` file is the Arduino project file, mostly ready
* The driver in `scha63x.c` is mostly ok
* `Timer.cpp` is ready and tested
* `scha63x_spi.cpp` utilizing Arduino's `SPI` library in progress
* `Serial.cpp` for data storage in progress
* `HW` PCB schematics from the breadboard wiring 
