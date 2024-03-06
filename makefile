build:
	gcc -Wall ./*.c -I /opt/homebrew/include -L /opt/homebrew/lib/ -lSDL2 -o main 