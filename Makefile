main:
	clang -Wall -Wextra -Wpedantic main.c \
		-o main -lraylib \
		-I/opt/homebrew/include \
		-L/opt/homebrew/lib \
	&& ./main
