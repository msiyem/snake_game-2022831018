all:
	g++ -Isrc/include -Lsrc/lib -o snake snake.cpp -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer
	g++ -Isrc/include -Lsrc/lib -o task301 task301.cpp -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer