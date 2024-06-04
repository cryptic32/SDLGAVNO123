#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib, "SDL2_mixer.lib")
#include <SDL.h>
#include <SDL_ttf.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <SDL_mixer.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define FONT_SIZE 36
#define CLOCK_RADIUS 150
#define CLOCK_THICKNESS 30
#define CLOCK_X (WINDOW_WIDTH / 2)
#define CLOCK_Y (WINDOW_HEIGHT / 2 - 50)
#define HOUR_HAND_LENGTH (CLOCK_RADIUS * 0.5)
#define MINUTE_HAND_LENGTH (CLOCK_RADIUS * 0.7)
#define SECOND_HAND_LENGTH (CLOCK_RADIUS * 0.8)
#define TIME_TEXT_Y (CLOCK_Y + CLOCK_RADIUS + 20)
#define DATE_TEXT_Y (TIME_TEXT_Y + FONT_SIZE + 10)
#define ALARM_TEXT_Y (DATE_TEXT_Y + FONT_SIZE + 10)


// Укажите пути к вашим шрифтам
const char* FONT_PATHS[] = {
    "G:/cpp/SDLGAVNO123/font1.ttf",
    "G:/cpp/SDLGAVNO123/font2.ttf",
    "G:/cpp/SDLGAVNO123/font3.ttf",
    "G:/cpp/SDLGAVNO123/font4.ttf",
    "G:/cpp/SDLGAVNO123/font5.ttf",
};
const char* FONT_NAMES[] = {
    "Font 1",
    "Font 2",
    "Font 3",
    "Font 4",
    "Font 5"
};

const SDL_Color FONT_COLORS[] = {
    {255, 255, 255, 255}, // Белый
    {255, 0, 0, 255},     // Красный
    {0, 255, 0, 255},     // Зеленый
    {0, 0, 255, 255},     // Синий
    {255, 255, 0, 255}    // Желтый
};
const char* TICK_SOUND_PATH = "G:/cpp/SDLGAVNO123/tick.wav";
const char* ALARM_SOUND_PATH = "G:/cpp/SDLGAVNO123/alarm.wav";

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        printf("Failed to initialize SDL: %s\n", SDL_GetError());
        return 1;
    }

    if (TTF_Init() != 0) {
        printf("Failed to initialize SDL_ttf: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("Failed to initialize SDL_mixer: %s\n", Mix_GetError());
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    printf("Hotkeys:\n\n");
    printf("Basic:\n");
    printf("Change font - TAB\n");
    printf("Manage text color - C\n");
    printf("Convert into 24h format - SPACE\n");
    printf("Alarm settings - a\n");
    printf("Manage alarm time - Arrows\n");
    printf("Turn on/off the alarm - SPACE\n");

    Mix_Chunk* tickSound = Mix_LoadWAV(TICK_SOUND_PATH);
    if (!tickSound) {
        printf("Failed to load tick sound: %s\n", Mix_GetError());
        Mix_CloseAudio();
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    Mix_Chunk* alarmSound = Mix_LoadWAV(ALARM_SOUND_PATH);
    if (!alarmSound) {
        printf("Failed to load alarm sound: %s\n", Mix_GetError());
        Mix_FreeChunk(tickSound);
        Mix_CloseAudio();
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Настольные часы",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Failed to create window: %s\n", SDL_GetError());
        Mix_FreeChunk(tickSound);
        Mix_FreeChunk(alarmSound);
        Mix_CloseAudio();
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Failed to create renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        Mix_FreeChunk(tickSound);
        Mix_FreeChunk(alarmSound);
        Mix_CloseAudio();
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    int currentFontIndex = 0;
    int currentColorIndex = 0;
    TTF_Font* font = TTF_OpenFont(FONT_PATHS[currentFontIndex], FONT_SIZE);
    if (!font) {
        printf("Failed to load font: %s\n", TTF_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        Mix_FreeChunk(tickSound);
        Mix_FreeChunk(alarmSound);
        Mix_CloseAudio();

        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    int timeFormat = 24; // 12-часовой формат времени
    bool showMenu = false;
    bool showColorMenu = false;
    bool showAlarmMenu = false;
    int input = 0;
    bool alarmTriggered = false;
    int alarmHour = 0;
    int alarmMinute = 0;
    bool running = true;

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_SPACE) {
                    timeFormat = (timeFormat == 12) ? 24 : 12;
                }
                if (event.key.keysym.sym == SDLK_TAB) {
                    showMenu = !showMenu;
                }
                if (event.key.keysym.sym == SDLK_c) {
                    showColorMenu = !showColorMenu;
                }
                if (event.key.keysym.sym == SDLK_a) {
                    showAlarmMenu = !showAlarmMenu;
                }
                if (event.key.keysym.sym >= SDLK_1 && event.key.keysym.sym <= SDLK_5) {
                    currentFontIndex = event.key.keysym.sym - SDLK_1;
                    TTF_CloseFont(font);
                    font = TTF_OpenFont(FONT_PATHS[currentFontIndex], FONT_SIZE);
                    if (!font) {
                        printf("Failed to load font: %s\n", TTF_GetError());
                        SDL_DestroyRenderer(renderer);
                        SDL_DestroyWindow(window);
                        Mix_FreeChunk(tickSound);
                        Mix_FreeChunk(alarmSound);
                        Mix_CloseAudio();
                        TTF_Quit();
                        SDL_Quit();
                        return 1;
                    }
                }
                if (event.key.keysym.sym == SDLK_r) {
                    currentColorIndex = 1;
                }
                if (event.key.keysym.sym == SDLK_g) {
                    currentColorIndex = 2;
                }
                if (event.key.keysym.sym == SDLK_b) {
                    currentColorIndex = 3;
                }
                if (event.key.keysym.sym == SDLK_y) {
                    currentColorIndex = 4;
                }
                if (event.key.keysym.sym == SDLK_w) {
                    currentColorIndex = 0;
                }
                if (showAlarmMenu) {
                    if (event.key.keysym.sym == SDLK_UP) {
                        alarmHour = (alarmHour + 1) % 24;
                    }
                    if (event.key.keysym.sym == SDLK_DOWN) {
                        alarmHour = (alarmHour - 1 + 24) % 24;
                    }
                    if (event.key.keysym.sym == SDLK_RIGHT) {
                        alarmMinute = (alarmMinute + 1) % 60;
                    }
                    if (event.key.keysym.sym == SDLK_LEFT) {
                        alarmMinute = (alarmMinute - 1 + 60) % 60;
                    }
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        time_t currentTime = time(NULL);
        struct tm* localTime = localtime(&currentTime);

        int hours = localTime->tm_hour;
        int minutes = localTime->tm_min;
        int seconds = localTime->tm_sec;
        int month = localTime->tm_mon + 1;
        int day = localTime->tm_mday;
        int year = localTime->tm_year + 1900;

        if (timeFormat == 12) {
            if (hours > 12) {
                hours -= 12;
            }
            if (hours == 0) {
                hours = 12;
            }
        }

        char timeString[9];
        char dateString[11];
        char alarmString[9];
        sprintf(timeString, "%02d:%02d:%02d", hours, minutes, seconds);
        sprintf(dateString, "%02d.%02d.%d", day, month, year);
        sprintf(alarmString, "%02d:%02d", alarmHour, alarmMinute);

        SDL_Color currentColor = FONT_COLORS[currentColorIndex];

        // Рисуем циферблат часов
        SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255); // Серый цвет
        SDL_Rect clockRect = {
            CLOCK_X - CLOCK_RADIUS,
            CLOCK_Y - CLOCK_RADIUS,
            CLOCK_RADIUS * 2,
            CLOCK_RADIUS * 2
        };
        SDL_RenderFillRect(renderer, &clockRect);

        SDL_SetRenderDrawColor(renderer, 64, 64, 64, 255); // Темно-серый цвет
        SDL_RenderDrawRect(renderer, &clockRect);

        // Рисуем отметки на циферблате
        for (int i = 0; i < 12; i++) {
            double angle = i * M_PI / 6.0;
            int x1 = CLOCK_X + (int)(CLOCK_RADIUS * 0.9 * cos(angle));
            int y1 = CLOCK_Y + (int)(CLOCK_RADIUS * 0.9 * sin(angle));
            int x2 = CLOCK_X + (int)(CLOCK_RADIUS * 0.95 * cos(angle));
            int y2 = CLOCK_Y + (int)(CLOCK_RADIUS * 0.95 * sin(angle));
            SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
        }

        // Рисуем часовую, минутную и секундную стрелки
        double hourAngle = (hours % 12) * M_PI / 6.0 + (minutes * M_PI / (6.0 * 60.0));
        double minuteAngle = (minutes * M_PI / 30.0) + (seconds * M_PI / (30.0 * 60.0));
        double secondAngle = (seconds * M_PI / 30.0);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // Белый цвет
        SDL_RenderDrawLine(renderer,
            CLOCK_X,
            CLOCK_Y,
            CLOCK_X + (int)(HOUR_HAND_LENGTH * cos(hourAngle)),
            CLOCK_Y + (int)(HOUR_HAND_LENGTH * sin(hourAngle)));

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // Белый цвет
        SDL_RenderDrawLine(renderer,
            CLOCK_X,
            CLOCK_Y,
            CLOCK_X + (int)(MINUTE_HAND_LENGTH * cos(minuteAngle)),
            CLOCK_Y + (int)(MINUTE_HAND_LENGTH * sin(minuteAngle)));

        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Красный цвет
        SDL_RenderDrawLine(renderer,
            CLOCK_X,
            CLOCK_Y,
            CLOCK_X + (int)(SECOND_HAND_LENGTH * cos(secondAngle)),
            CLOCK_Y + (int)(SECOND_HAND_LENGTH * sin(secondAngle)));

        SDL_Surface* timeTextSurface = TTF_RenderText_Solid(font, timeString, currentColor);
        SDL_Texture* timeTextTexture = SDL_CreateTextureFromSurface(renderer, timeTextSurface);

        SDL_Surface* dateTextSurface = TTF_RenderText_Solid(font, dateString, currentColor);
        SDL_Texture* dateTextTexture = SDL_CreateTextureFromSurface(renderer, dateTextSurface);

        SDL_Surface* alarmTextSurface = TTF_RenderText_Solid(font, alarmString, currentColor);
        SDL_Texture* alarmTextTexture = SDL_CreateTextureFromSurface(renderer, alarmTextSurface);

        int timeTextWidth, timeTextHeight;
        SDL_QueryTexture(timeTextTexture, NULL, NULL, &timeTextWidth, &timeTextHeight);

        int dateTextWidth, dateTextHeight;
        SDL_QueryTexture(dateTextTexture, NULL, NULL, &dateTextWidth, &dateTextHeight);

        int alarmTextWidth, alarmTextHeight;
        SDL_QueryTexture(alarmTextTexture, NULL, NULL, &alarmTextWidth, &alarmTextHeight);

        SDL_Rect timeTextRect = {
            CLOCK_X - timeTextWidth / 2,
TIME_TEXT_Y,
            timeTextWidth,
            timeTextHeight
        };

        SDL_Rect dateTextRect = {
            CLOCK_X - dateTextWidth / 2,
            DATE_TEXT_Y,
            dateTextWidth,
            dateTextHeight
        };

        SDL_Rect alarmTextRect = {
            CLOCK_X - alarmTextWidth / 2,
            ALARM_TEXT_Y,
            alarmTextWidth,
            alarmTextHeight
        };

        SDL_RenderCopy(renderer, timeTextTexture, NULL, &timeTextRect);
        SDL_RenderCopy(renderer, dateTextTexture, NULL, &dateTextRect);
        SDL_RenderCopy(renderer, alarmTextTexture, NULL, &alarmTextRect);

        if (showMenu) {
            // Рисуем меню шрифтов
            SDL_Color menuColor = { 255, 255, 255, 255 };
            int menuItemHeight = 40;
            int menuY = (WINDOW_HEIGHT - (sizeof(FONT_NAMES) / sizeof(FONT_NAMES[0])) * menuItemHeight) / 2;
            int menuX = 10;
            int menuWidth = WINDOW_WIDTH - 20;

            SDL_Rect menuRect = {
                menuX,
                menuY,
                menuWidth,
                (sizeof(FONT_NAMES) / sizeof(FONT_NAMES[0])) * menuItemHeight
            };

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
            SDL_RenderFillRect(renderer, &menuRect);

            for (int i = 0; i < sizeof(FONT_NAMES) / sizeof(FONT_NAMES[0]); i++) {
                SDL_Rect menuItemRect = {
                    menuX,
                    menuY + i * menuItemHeight,
                    menuWidth,
                    menuItemHeight
                };

                if (i == currentFontIndex) {
                    SDL_SetRenderDrawColor(renderer, 64, 64, 64, 255);
                    SDL_RenderFillRect(renderer, &menuItemRect);
                }

                SDL_Surface* menuItemSurface = TTF_RenderText_Solid(font, FONT_NAMES[i], menuColor);
                SDL_Texture* menuItemTexture = SDL_CreateTextureFromSurface(renderer, menuItemSurface);

                int menuItemWidth, _menuItemHeight;
                SDL_QueryTexture(menuItemTexture, NULL, NULL, &menuItemWidth, &_menuItemHeight);

                SDL_Rect menuItemTextRect = {
                    (WINDOW_WIDTH - menuItemWidth) / 2,
                    menuY + i * menuItemHeight + (menuItemHeight - _menuItemHeight) / 2,
                    menuItemWidth,
                    _menuItemHeight
                };

                SDL_RenderCopy(renderer, menuItemTexture, NULL, &menuItemTextRect);

                SDL_DestroyTexture(menuItemTexture);
                SDL_FreeSurface(menuItemSurface);
            }
        }

        if (showColorMenu) {
            // Рисуем меню цветов
            SDL_Color colorMenuColor = { 255, 255, 255, 255 };
            int colorMenuItemHeight = 40;
            int colorMenuY = (WINDOW_HEIGHT - (sizeof(FONT_COLORS) / sizeof(FONT_COLORS[0])) * colorMenuItemHeight) / 2;
            int colorMenuX = 10;
            int colorMenuWidth = WINDOW_WIDTH - 20;

            SDL_Rect colorMenuRect = {
                colorMenuX,
                colorMenuY,
                colorMenuWidth,
                (sizeof(FONT_COLORS) / sizeof(FONT_COLORS[0])) * colorMenuItemHeight
            };

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
            SDL_RenderFillRect(renderer, &colorMenuRect);

            for (int i = 0; i < sizeof(FONT_COLORS) / sizeof(FONT_COLORS[0]); i++) {
                SDL_Rect colorMenuItemRect = {
                    colorMenuX,
                    colorMenuY + i * colorMenuItemHeight,
                    colorMenuWidth,
                    colorMenuItemHeight
                };

                if (i == currentColorIndex) {
                    SDL_SetRenderDrawColor(renderer, 64, 64, 64, 255);
                    SDL_RenderFillRect(renderer, &colorMenuItemRect);
                }

                SDL_Surface* colorMenuItemSurface = TTF_RenderText_Solid(font, i == 0 ? "White" : i == 1 ? "Red" : i == 2 ? "Green" : i == 3 ? "Blue" : "Yellow", FONT_COLORS[i]);
                SDL_Texture* colorMenuItemTexture = SDL_CreateTextureFromSurface(renderer, colorMenuItemSurface);
                int colorMenuItemWidth, _colorMenuItemHeight;
                SDL_QueryTexture(colorMenuItemTexture, NULL, NULL, &colorMenuItemWidth, &_colorMenuItemHeight);

                SDL_Rect colorMenuItemTextRect = {
                    (WINDOW_WIDTH - colorMenuItemWidth) / 2,
                    colorMenuY + i * colorMenuItemHeight + (colorMenuItemHeight - _colorMenuItemHeight) / 2,
                    colorMenuItemWidth,
                    _colorMenuItemHeight
                };

                SDL_RenderCopy(renderer, colorMenuItemTexture, NULL, &colorMenuItemTextRect);

                SDL_DestroyTexture(colorMenuItemTexture);
                SDL_FreeSurface(colorMenuItemSurface);
            }
        }
        // ... (код выше)

        if (showAlarmMenu) {
            // Рисуем меню будильника
            SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
            SDL_Rect alarmMenuRect = {
                CLOCK_X - 100,
                ALARM_TEXT_Y + 40,
                200,
                100
            };
            SDL_RenderFillRect(renderer, &alarmMenuRect);

            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            char alarmTimeString[9];
            sprintf(alarmTimeString, "%02d:%02d", alarmHour, alarmMinute);
            SDL_Surface* alarmMenuTextSurface = TTF_RenderText_Solid(font, alarmTimeString, currentColor);
            SDL_Texture* alarmMenuTextTexture = SDL_CreateTextureFromSurface(renderer, alarmMenuTextSurface);

            int alarmMenuTextWidth, alarmMenuTextHeight;
            SDL_QueryTexture(alarmMenuTextTexture, NULL, NULL, &alarmMenuTextWidth, &alarmMenuTextHeight);

            SDL_Rect alarmMenuTextRect = {
                CLOCK_X - alarmMenuTextWidth / 2,
                ALARM_TEXT_Y + 50,
                alarmMenuTextWidth,
                alarmMenuTextHeight
            };

            SDL_RenderCopy(renderer, alarmMenuTextTexture, NULL, &alarmMenuTextRect);
            SDL_DestroyTexture(alarmMenuTextTexture);
            SDL_FreeSurface(alarmMenuTextSurface);
        }

        SDL_RenderPresent(renderer);

        SDL_DestroyTexture(timeTextTexture);
        SDL_FreeSurface(timeTextSurface);
        SDL_DestroyTexture(dateTextTexture);
        SDL_FreeSurface(dateTextSurface);
        SDL_DestroyTexture(alarmTextTexture);
        SDL_FreeSurface(alarmTextSurface);

        // Воспроизведение звука тика каждую секунду
        if (seconds % 1 == 0) {
            Mix_PlayChannel(-1, tickSound, 0);
        }

        // Проверка на срабатывание будильника
        if (hours == alarmHour && minutes == alarmMinute && !alarmTriggered) {
            Mix_PlayChannel(-1, alarmSound, 0);
            alarmTriggered = true;
        }

        // Сброс будильника, если время изменилось
        if (hours != alarmHour || minutes != alarmMinute) {
            alarmTriggered = false;
        }

        SDL_Delay(1000);
    }

    // Очистка ресурсов
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    Mix_FreeChunk(tickSound);
    Mix_FreeChunk(alarmSound);
    Mix_CloseAudio();
    TTF_Quit();
    SDL_Quit();

    return 0;
}





