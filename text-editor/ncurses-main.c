#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "vector.h"
#include <ncursesw/ncurses.h>

int WINDOW_ROWS, WINDOW_COLS;

#define CTRL(x) ((x) & 0x1f)

vector* g_filebufs; // names of files opened

typedef struct {
    int x, y;
} Vec2;

void print_help() {
}

bool file_exists(const char *filename) {
    return (access(filename, F_OK) == 0);
}
/*
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
*/
size_t get_filesize_lines(char* filename) { // counts clipped lines as separate
    FILE* fp = fopen(filename, "r");

    size_t cnt = 0, charcnt = 0;
    for(char tmp_c = fgetc(fp); tmp_c != EOF; tmp_c = fgetc(fp)){
        ++charcnt;

        if(tmp_c == '\n' || charcnt >= WINDOW_COLS) {
            ++cnt;
            charcnt = 0;
        }
    }
    fclose(fp);

    return cnt + 1;
}

void print_lines(char* filename, int start, int n, int* screenbuf_linesizes) { // 0-indexed, n - number of lines to print
    char line_buffer[WINDOW_COLS + 1]; //TODO optional: review

    FILE* fp = fopen(filename, "r");
    if(!fp) {
        perror(filename);
        return;
    }

    erase();

    for(int i = 0; i < start; ++i) {
        fgets(line_buffer, sizeof(line_buffer), fp);
    }
    for(int i = 0; i < n; ++i) {
        if(!fgets(line_buffer, sizeof(line_buffer) - 1, fp)) break;
        line_buffer[WINDOW_COLS] = '\0';
        printw("%s", line_buffer);
        screenbuf_linesizes[i] = strlen(line_buffer);
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
}

void draw_cursor(int what_button, Vec2* cpos, int* wpos, int cur_filebuf_id, size_t cur_filebuf_size, int* screenbuf_linesizes) {
    // TODO: review all the cpos->y <= WINDOW_ROWS - 1 (last line doesn't have a newline)
    bool x_needs_update = 0;

    // check y
    if(cpos->y < 0) {
        *wpos = *wpos - 1 < 0 ? *wpos : *wpos - 1;
        print_lines(g_filebufs->data[cur_filebuf_id], *wpos, WINDOW_ROWS, screenbuf_linesizes);
        cpos->y = 0;
    }
    if(cpos->y >= WINDOW_ROWS) {
        *wpos = *wpos + 1 > cur_filebuf_size - WINDOW_ROWS ? *wpos : *wpos + 1;
        print_lines(g_filebufs->data[cur_filebuf_id], *wpos, WINDOW_ROWS, screenbuf_linesizes);
        cpos->y = WINDOW_ROWS - 1;
    }
    // check x
    if(cpos->x < 0) { // ig wont happen
        --cpos->y;
        if(cpos->y < 0 && *wpos != 0) x_needs_update = 1;
        else cpos->x = screenbuf_linesizes[cpos->y] - 1;
    }
    if(cpos->y >= 0 && cpos->x >= screenbuf_linesizes[cpos->y]) {
        if(what_button == KEY_UP || what_button == KEY_DOWN) {
            // last line doesn't have a newline
            cpos->x = screenbuf_linesizes[cpos->y] - (cpos->y <= WINDOW_ROWS - 1 || *wpos != cur_filebuf_size - WINDOW_ROWS);
        }
        else {
            ++cpos->y;
            if(cpos->y <= WINDOW_ROWS - 1 || *wpos != cur_filebuf_size - WINDOW_ROWS) cpos->x = 0;
            else cpos->x = screenbuf_linesizes[cpos->y - 1]; // last line doesn't have a newline
        }
    }
    // check y
    if(cpos->y < 0) {
        *wpos = *wpos - 1 < 0 ? *wpos : *wpos - 1;
        print_lines(g_filebufs->data[cur_filebuf_id], *wpos, WINDOW_ROWS, screenbuf_linesizes);
        cpos->y = 0;
    }
    if(cpos->y >= WINDOW_ROWS) {
        *wpos = *wpos + 1 > cur_filebuf_size - WINDOW_ROWS ? *wpos : *wpos + 1;
        print_lines(g_filebufs->data[cur_filebuf_id], *wpos, WINDOW_ROWS, screenbuf_linesizes);
        cpos->y = WINDOW_ROWS - 1;
    }

    if(x_needs_update) {
        cpos->x = screenbuf_linesizes[cpos->y] - 1;
    }

    move(cpos->y, cpos->x);
    refresh();
}

void app_input_handler() {
    int cur_filebuf_id = 0;
    size_t cur_filebuf_size = get_filesize_lines(g_filebufs->data[cur_filebuf_id]);

    int screenbuf_linesizes[WINDOW_ROWS]; // counting trailing newline
    Vec2 cursor_pos = (Vec2){0, 0};
    int window_pos = 0;

    print_lines(g_filebufs->data[0], 0, WINDOW_ROWS, screenbuf_linesizes); // TEMP SOLUTION MAYBE: print the first opened file contents
    move(0, 0);

    bool window_should_close = 0;
    while(!window_should_close){
        int ch = getch();
        switch(ch) {
        case KEY_UP:
            --cursor_pos.y;
            draw_cursor(KEY_UP, &cursor_pos, &window_pos, cur_filebuf_id, cur_filebuf_size, screenbuf_linesizes);
            break;
        case KEY_DOWN:
            ++cursor_pos.y;
            draw_cursor(KEY_DOWN, &cursor_pos, &window_pos, cur_filebuf_id, cur_filebuf_size, screenbuf_linesizes);
            break;
        case KEY_LEFT:
            --cursor_pos.x;
            draw_cursor(KEY_LEFT, &cursor_pos, &window_pos, cur_filebuf_id, cur_filebuf_size, screenbuf_linesizes);
            break;
        case KEY_RIGHT:
            ++cursor_pos.x;
            draw_cursor(KEY_RIGHT, &cursor_pos, &window_pos, cur_filebuf_id, cur_filebuf_size, screenbuf_linesizes);
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
    echo();
    keypad(stdscr, TRUE);
    startup_handler(argc, argv);

    // main 'loop'
    app_input_handler();
    // end of the main 'loop'

    exit_handler();

    endwin();
    return 0;
}