all:
g++ -Isrc/include -Lsrc/lib -o main main.cpp -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf
g++ -Isrc/include -Lsrc/lib -o snake snake.cpp -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf