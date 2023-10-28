@echo off
set PATH=D:\picobuild5\pico-toolchain\gxx-mingw32-msys64\usr\bin\;%PATH%
g++ main.cpp -o app.exe ^
        -I .\glew-2.1.0\include -L .\glew-2.1.0\lib -l glew32_x64 ^
        -I .\glfw-3.3.8\include -L .\glfw-3.3.8\lib -l glfw32_x64 ^
        -l opengl32 -l gdi32
cmd /K
pause