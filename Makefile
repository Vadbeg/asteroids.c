
run:
	clang -Wall -Wextra -Wpedantic main.c -o main && ./main

raylib:
	clang -Wall -Wextra -Wpedantic raylib_test.c \
		-o raylib_test -lraylib \
		-I/opt/homebrew/include \
		-L/opt/homebrew/lib \
	&& ./raylib_test
