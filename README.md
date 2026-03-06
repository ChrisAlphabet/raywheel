# Raywheel

## What is this?
Have you seen [wheelofnames.com](https://wheelofnames.com/) and ever thought to your self "what if it was much worse?"  

## Disclaimer
This is not "production-ready code", just Raylib experiments in the fleeting moments between when my newborn is asleep and I am somehow awake. 

## Build
* [Follow instructions to install raylib on GNU Linux](https://github.com/raysan5/raylib/wiki/Working-on-GNU-Linux)  
  * `sudo apt install libasound2-dev libx11-dev libxrandr-dev libxi-dev libgl1-mesa-dev libglu1-mesa-dev libxcursor-dev libxinerama-dev libwayland-dev libxkbcommon-dev`  
* `cd raywheel`  
* `make raylib`  
* `make raywheel`  

## Run  
* `cd raywheel`  
* `./main`  
* Drag a csv file in that contains names and the weeks since last judged unworthy:
```
name,weeks
Chris,0
Foo,2
Bar,5
```

## Web
Note that you might need to clean raylib first, seemed that building for desktop and web might have some cached file issue:  
* `make raylib_clean`
* `make emscripten_sdk`   
* `source ./external/emsdk/emsdk_env.sh`
* `make raylib_web`
* `make raywheel_web`
* `python -m http.server 8080`
