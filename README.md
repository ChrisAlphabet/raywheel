# raywheel

## Disclaimer
This is not production-ready code, just experiments sitting on the couch with a laptop in the tiny window after my newborn falls asleep before I fall asleep.  

## Build
* [Follow instructions to install raylib on GNU Linux](https://github.com/raysan5/raylib/wiki/Working-on-GNU-Linux)  
  * `sudo apt install libasound2-dev libx11-dev libxrandr-dev libxi-dev libgl1-mesa-dev libglu1-mesa-dev libxcursor-dev libxinerama-dev libwayland-dev libxkbcommon-dev`  
* `cd raywheel`  
* `make raylib`  
* `make raywheel`  

## Run  
* `cd raywheel`  
* `./main`  

## Web
Note that you might need to clean raylib first, seemed that building for desktop and web might have some cached file issue:  
* `make raylib_clean`
* `make emscripten_sdk`   
* `source ./external/emsdk/emsdk_env.sh`
* `make raylib_web`
* `make raywheel_web`
* `python -m http.server 8080`
