# glvolume
OpenGL Shader Code to display a multicoloured rotating 3D volumetric cutout cube on an EGL Context (Pi3A)

### Disclaimer: This code has changed several times and will likely break again.

![glvolume](https://github.com/TheMindVirus/gltriangle/blob/glvolume-pi3a/glvolume.png)

## Dependencies
m, bcm_host, brcmEGL and brcmGLESv2 - aka graphics drivers, build the userland!
https://github.com/raspberrypi/userland/

## Issues
There are many limitations including hard faults, iteration restrictions and random v3d lock-ups.
All of them can be resolved with a reboot...