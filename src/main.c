#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <cjson/cJSON.h>
#include <curl/curl.h>
#include <ncurses.h>

#include "errors.h"
#include "api.h"

const char *logo = 
    " ____  _      _____ _   _ _  __    \n"
    "|  _ \\| |    |_   _| \\ | | |/ /  \n"
    "| |_) | |      | | |  \\| | ' /    \n"
    "|  _ <| |      | | | . ` |  <      \n"
    "| |_) | |____ _| |_| |\\  | . \\   \n"
    "|____/|______|_____|_| \\_|_|\\_\\ \n";


const char *api_routes  = "https://api.opendata.metlink.org.nz/v1/gtfs/routes\?stop_id=";    
const char *api_predict = "https://api.opendata.metlink.org.nz/v1/stop-predictions\?stop_id=";
typedef enum ScreenState {
    STATE_MENU,
    STATE_STOP_ID,
    STATE_ROUTE,
    STATE_EXIT
} ScreenState;

// Custom callback function to write the response data into a dynamic string

int main() {

    char *data = NULL;
    char *stop_preditctions = NULL;
    ScreenState screenstate = STATE_MENU;
    initscr();
    start_color();
    keypad(stdscr, true);
    
    if (has_colors() == false) {
        endwin();
        printf("Your terminal does not support color\n");
        exit(1);
    }

    init_pair(0, COLOR_BLACK, COLOR_RED);
    init_pair(1, COLOR_RED, COLOR_BLACK);

    char stop_id[5] = {0};

    size_t route_cap = 512;
    size_t route_length = 0;

    typedef struct Routes{
        char **routes;
        char **predictions;
    } Routes;

    Routes routes; 

    routes.routes = malloc(sizeof(char *) * route_cap);

    if(routes.routes == NULL) {
        fprintf(stderr, ERRORS_ALLOCATE);
        exit(1);
    }

    while (screenstate != STATE_EXIT) {
        clear();
        curs_set(0);

        switch (screenstate) {
            case STATE_MENU: {
                attron(COLOR_PAIR(1));
                printw( "%s\nPlease press enter to begin!" , logo);
                refresh();
                break;            
            }

            case STATE_STOP_ID: {
                mvprintw(1, 2, "Enter stop id: %s", stop_id);
                break;
            }

            case STATE_ROUTE: {
                for(size_t i = 0; i < route_length; i++){
                    printw("%s\n", routes.routes[i]);
                }
                printw("\n%s", stop_preditctions);
                break;
            }

            default: {
                break;
            }
        }

        const int key = getch();

        switch (key) {
            case 'x':
            case 'q': {
                screenstate = STATE_EXIT;
                break;
            }

            case '\n': {
                switch (screenstate) {
                    case STATE_MENU: {
                        screenstate = STATE_STOP_ID;
                        break;
                    }

                    case STATE_STOP_ID: {
                        if (strlen(stop_id) >= 4) {

                            data = get_stop_routes(stop_id, api_routes);
                            stop_preditctions = get_stop_routes(stop_id, api_predict);
                            cJSON *elem;
                            cJSON *name;
                            cJSON *root = cJSON_Parse(data);
                            int n = cJSON_GetArraySize(root);
                            
                            for (int i = 0; i<n; i++) {
                                elem = cJSON_GetArrayItem(root, i);
                                name = cJSON_GetObjectItem(elem, "route_short_name");


                                if (route_length >= route_cap) {
                                    while (route_length >= route_cap) {
                                        route_cap *= 2;
                                    }
                                    routes.routes = realloc(routes.routes, sizeof(char *) * route_cap);
                                    if (routes.routes == NULL) {
                                        fprintf(stderr, ERRORS_ALLOCATE);
                                        exit(1);
                                    }
                                }
                                routes.routes[route_length++] = name->valuestring;
                                
                                printf("%s : bus\n", name->valuestring);
                            }
                            screenstate = STATE_ROUTE;
                        }
                        break;
                    }

                    default: {
                        break;
                    }
                }
                break;
            }

            case '0'...'9': {
                if (screenstate == STATE_STOP_ID) {
                    const size_t stop_id_len = strlen(stop_id);
                    if (stop_id_len < 4) {
                        stop_id[stop_id_len] = key;
                    }
                }
                break;
            }

            case KEY_BACKSPACE: {
                if (screenstate == STATE_STOP_ID) {
                    const size_t stop_id_len = strlen(stop_id);
                    if (stop_id_len > 0) {
                        stop_id[stop_id_len - 1] = 0;
                    }
                }
                break;
            }
        }
    }

    endwin();

    free(stop_preditctions);
    free(routes.routes);
    free(data);
    return 0;

}
