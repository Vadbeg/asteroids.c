
run:
	clang -Wall -Wextra -Wpedantic main.c -o main && ./main

ncurses:
	clang -Wall -Wextra -Wpedantic ncurses_test.c -o ncurses_test -lncurses && ./ncurses_test
