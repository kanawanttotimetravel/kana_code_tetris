all: compile link run

compile: 
	g++ main.cpp -c  -Isrc/include

link: 
	g++ main.o -o main -Lsrc/lib -lsfml-graphics -lsfml-window -lsfml-audio -lsfml-system

run:
	main.exe
	