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

    char *routes_raw = NULL;
    char *stop_preditctions_raw = NULL;
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



    typedef struct Routes{
        char **routes;
        char **predictions;
        size_t routes_cap;
        size_t predictions_cap;
        size_t routes_length;
        size_t predictions_length;
    } Routes;

    Routes routes = {
        .routes_cap         = 512,
        .predictions_cap    = 512,
        .routes             = malloc(sizeof(char *) * routes.routes_cap),
        .predictions        = malloc(sizeof(char *) * routes.predictions_cap),
        .routes_length      = 0,
        .predictions_length = 0
    };


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
                for(size_t i = 0; i < routes.routes_length; i++) {
                    printw("%s\n", routes.routes[i]);
                }
                break;

                for(size_t i =0; i < routes.predictions_length; i++) {
                    printw("%s\n", routes.predictions[i]);
                }
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
                            routes_raw = get_stop_routes(stop_id, api_routes);
                            stop_preditctions_raw = get_stop_routes(stop_id, api_predict);
                            cJSON *elem;
                            cJSON *name;
                            cJSON *root = cJSON_Parse(routes_raw);
                            int n = cJSON_GetArraySize(root);
                            
                            for (int i = 0; i<n; i++) {
                                elem = cJSON_GetArrayItem(root, i);
                                name = cJSON_GetObjectItem(elem, "route_short_name");


                                if (routes.routes_length  >= routes.routes_cap) {
                                    while (routes.routes_length >= routes.routes_cap) {
                                        routes.routes_cap *= 2;
                                    }
                                    routes.routes = realloc(routes.routes, sizeof(char *) * routes.routes_cap);
                                    if (routes.routes == NULL) {
                                        fprintf(stderr, ERRORS_ALLOCATE);
                                        exit(1);
                                    }
                                }
                                routes.routes[routes.routes_length++] = name->valuestring;
                                
                            }   

                            root = cJSON_Parse(stop_preditctions_raw);
                            cJSON *departures = cJSON_GetObjectItem(root, "departures");

                            n = cJSON_GetArraySize(departures);
                            printf("%d", n);

                            for (int i = 0; i<n; i++) {
                                cJSON *departure = cJSON_GetArrayItem(departures, i);
                                cJSON *arrival   = cJSON_GetObjectItem(departure, "arrival");
                                cJSON *expected  = cJSON_GetObjectItem(arrival, "expected");
                                
                                if (routes.predictions_length  >= routes.predictions_cap) {
                                    while (routes.predictions_length >= routes.predictions_cap) {
                                        routes.predictions_cap *= 2;
                                    }
                                    routes.predictions = realloc(routes.predictions, sizeof(char *) * routes.predictions_cap);
                                    if (routes.predictions == NULL) {
                                        fprintf(stderr, ERRORS_ALLOCATE);
                                        exit(1);
                                    }
                                }
                                routes.predictions[routes.predictions_length++] = expected->valuestring;

                                printf("%s : bus\n", expected->valuestring);
                            }   

                            exit(0);
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

    free(stop_preditctions_raw);
    free(routes.routes);
    free(routes_raw);
    return 0;

}
