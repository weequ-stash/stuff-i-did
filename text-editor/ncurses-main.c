#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "vector.h"
#include <ncursesw/ncurses.h>

int WINDOW_ROWS, WINDOW_COLS;

#define CTRL(x) ((x) & 0x1f)

vector* g_filebufs; // names of files opened
//vector* g_curfile;  // data (strings) of current opened file

void print_help() {
}

bool file_exists(const char *filename) {
    return (access(filename, F_OK) == 0);
}

size_t get_filesize_bytes(char* filename) {
    size_t filesize;

    char tmp_command[7 + strlen(filename)];
    sprintf(tmp_command, "wc -c %s", filename);
    FILE* filesize_fp = popen(tmp_command, "r");
    if(!filesize_fp) exit(1);

    char tmp_filesize[32]; //! max 30-ish digits
    if(!fgets(tmp_filesize, sizeof(tmp_filesize), filesize_fp)) exit(1);

    filesize = atoll(tmp_filesize);
    pclose(filesize_fp);

    return filesize;
}

size_t get_filesize_lines(char* filename) {
    FILE* fp = fopen(filename, "r");

    size_t cnt = 0;
    for(char tmp_c = fgetc(fp); tmp_c != EOF; tmp_c = fgetc(fp)){
        if(tmp_c == '\n') ++cnt;
    }
    fclose(fp);

    return cnt;
}

void print_lines(char* filename, int start, int n) { // 1-indexed, n - number of lines to print
    char line_buffer[WINDOW_COLS + 1]; //TODO optional: review

    FILE* fp = fopen(filename, "r");
    if(!fp) {
        perror(filename);
        return;
    }

    erase();

    for(int i = 1; i < start; ++i) {
        fgets(line_buffer, sizeof(line_buffer), fp);
    }
    for(int i = start; i < start + n; ++i) {
        if(!fgets(line_buffer, sizeof(line_buffer), fp)) break;
        printw("%s", line_buffer);
    }

    refresh();
    fclose(fp);
}

void startup_handler(int argc, char** argv) {
    char tmp_dims[32];

    FILE* dims_fp = popen("tput lines", "r");
    if(!dims_fp) exit(1);
    if(!fgets(tmp_dims, sizeof(tmp_dims), dims_fp)) exit(1);
    WINDOW_ROWS = atoll(tmp_dims);
    pclose(dims_fp);

    dims_fp = popen("tput cols", "r");
    if(!dims_fp) exit(1);
    if(!fgets(tmp_dims, sizeof(tmp_dims), dims_fp)) exit(1);
    WINDOW_COLS = atoll(tmp_dims);
    pclose(dims_fp);

    --argc;
    ++argv;

    if(!(g_filebufs = vector_init(argc))) exit(1);

    for(int i = 0; i < argc; ++i) {
        if(strcmp(argv[i], "--help") == 0) {
            print_help();
            exit(0);
        }

        char tmp_filename[strlen(argv[i]) + 2];

        sprintf(tmp_filename, "%s ", argv[i]); // add a space at the end
        vector_push(g_filebufs, tmp_filename);
    }

    if(g_filebufs->size > 0) {
        for(int i = 0; i < g_filebufs->size; ++i) {
            char tmp_command[7 + 2 * strlen(g_filebufs->data[i])];
            char tmp_filename[strlen(g_filebufs->data[i]) + 1];
            strcpy(tmp_filename, g_filebufs->data[i]); // existing "filename"

            g_filebufs->data[i][strlen(g_filebufs->data[i]) - 1] = '~'; // new temp "filename~"

            if(!file_exists(argv[i]))
                sprintf(tmp_command, "touch %s", g_filebufs->data[i]);
            else
                sprintf(tmp_command, "cp %s%s", tmp_filename, g_filebufs->data[i]);

#ifdef DEBUG
            printf("CREATED TMP FILE: %s\n", tmp_command);
#endif
            system(tmp_command);
        }
    }
    else {
        // get the number of temp(new) files already created
        FILE* dir_wc_fp = popen("find . -maxdepth 1 -type f -name '[0-9]*-new-file~' | wc -w", "r");
        if(!dir_wc_fp) exit(1);
        char digitbuf[256];
        if(!fgets(digitbuf, sizeof(digitbuf), dir_wc_fp)) exit(1);
        if(pclose(dir_wc_fp) == -1) exit(1);

        // end the string where \n is (wc returns a string with \n at the end)
        digitbuf[strlen(digitbuf) - 1] = '\0';

        char tmp_filename[strlen(digitbuf) + 11];

        sprintf(tmp_filename, "%s-new-file~", digitbuf);

        char tmp_command[strlen(tmp_filename) + 6];
        sprintf(tmp_command, "touch %s", tmp_filename);
#ifdef DEBUG
        printf("CREATED TMP FILE: %s\n", tmp_command);
#endif
        system(tmp_command);

        vector_push(g_filebufs, tmp_filename);
    }

    // now we have g_filebufs filled with names of the created temp files

//    system("echo \e[?1049h"); // save cursor pos and switch to an alternate screen buffer
//    system("echo \e[H");

    print_lines(g_filebufs->data[0], 0, WINDOW_ROWS);
}

void app_input_handler() {
    int cur_filebuf_id = 0;
    size_t cur_filebuf_size = get_filesize_lines(g_filebufs->data[cur_filebuf_id]);
    int cursor_pos = 1;

    bool window_should_close = 0;
    while(!window_should_close){
        int ch = getch();
        switch(ch) {
        case KEY_UP:
            cursor_pos = cursor_pos - 1 < 1 ? cursor_pos : cursor_pos - 1;
            print_lines(g_filebufs->data[cur_filebuf_id], cursor_pos, WINDOW_ROWS);
            break;
        case KEY_DOWN:
            cursor_pos = cursor_pos + 1 > cur_filebuf_size - WINDOW_ROWS + 2 ? cursor_pos : cursor_pos + 1;
            print_lines(g_filebufs->data[cur_filebuf_id], cursor_pos, WINDOW_ROWS);
            break;
        case CTRL('q'):
            window_should_close = 1;
            break;
        default:
            break;
        }
    }
}

void exit_handler() {
//    system("echo \e[?1049l"); // restore cursor pos and switch back to the original screen buffer

    char tmp_command[256 + 6];
    for(int i = 0; i < g_filebufs->size; ++i) {
        sprintf(tmp_command, "rm ./%s", g_filebufs->data[i]);
        system(tmp_command);
#ifdef DEBUG
        printf("DELETED TMP FILE: %s\n", tmp_command);
#endif
    }

    vector_delete(g_filebufs);
}

int main(int argc, char** argv) {
    initscr();
    raw();
    noecho();
    keypad(stdscr, TRUE);
    startup_handler(argc, argv);

    // main 'loop'
    app_input_handler();
    // end of the main 'loop'

    exit_handler();

    endwin();
    return 0;
}