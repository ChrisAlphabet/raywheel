raylib:
	$(MAKE) -C ./external/raylib/src PLATFORM=PLATFORM_DESKTOP

raylib_web:
	$(MAKE) -C ./external/raylib/src PLATFORM=PLATFORM_WEB

raylib_clean:
	$(MAKE) -C ./external/raylib/src clean

emscripten_sdk:
	./build_emscripten_sdk.sh

raywheel:
	gcc -o main ./src/main.c -std=c2x -Wall -Wextra -g -I./external/raylib/src ./external/raylib/src/libraylib.a -lm -lpthread -ldl -lrt -lX11 

raywheel_web:
	emcc -o main.html ./src/main.c -std=c2x -Os -Wall -Wextra -I./external/raylib/src ./external/raylib/src/libraylib.web.a -s USE_GLFW=3 -s ASYNCIFY --preload-file ./src -s FORCE_FILESYSTEM=1 -s WASM=1 --shell-file ./external/raylib/src/minshell.html -DPLATFORM_WEB        

