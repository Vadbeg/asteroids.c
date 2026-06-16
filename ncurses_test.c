
#include <curses.h>
#include <unistd.h> 

int main(void){
    initscr();
    printw("Hellow World!");

    for (int i = 0; i < 10; i++){
        clear();
        mvprintw(2 * i, 34, "@");
        refresh();
        usleep(100000);
    }
    
    getch();
    clear();
    endwin();

    return 0;
}
