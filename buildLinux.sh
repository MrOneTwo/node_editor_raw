
if [ ! -d "app" ]; then
  mkdir app
fi
pushd app
g++ -g -Wall ../src/linux_main.cpp -I../src/SDL2-2.0.8/include -L../src/SDL2-2.0.8/app -lSDL2 -lSDL2main -lGLEW -ldl -lpthread -lGL -o linux_app
popd

