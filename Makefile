all:
	g++ src/main.cpp -o simulator.exe -Iinclude -lsfml-graphics -lsfml-window -lsfml-system -mwindows

clean:
	rm -f simulator.exe
