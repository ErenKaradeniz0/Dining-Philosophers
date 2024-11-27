﻿#include <windows.h>
#include <cmath>
#include "icb_gui.h"

// Global variables
ICBYTES screenMatrix, PhilosophersBMP, PhilosophersBMPX3;
ICBYTES PhilosopherBlue, PhilosopherRed, PhilosopherGreen, PhilosopherBrown;
int F1;

// Constants
const int NUM_PHILOSOPHERS = 5;

// Philosopher states
enum State { THINKING, HUNGRY, EATING, STARVED };
State philosopherStates[NUM_PHILOSOPHERS];


//coordinates
ICBYTES Coordinates{
    {6, 6, 75, 96},     // Red Front
    {87, 6, 75, 96},    // Blue
    {168, 6, 75, 96},   // Green
    {249, 6, 75, 96},   // Brown
};

//print blue on scene
void PreparePhilosophersBMP() {
    Copy(PhilosophersBMPX3, Coordinates.I(1, 1), Coordinates.I(2, 1),
        Coordinates.I(3, 1), Coordinates.I(4, 1),
        PhilosopherRed);

    Copy(PhilosophersBMPX3, Coordinates.I(1, 2), Coordinates.I(2, 2),
        Coordinates.I(3, 2), Coordinates.I(4, 2),
        PhilosopherBlue);

    Copy(PhilosophersBMPX3, Coordinates.I(1, 3), Coordinates.I(2, 3),
        Coordinates.I(3, 3), Coordinates.I(4, 3),
        PhilosopherGreen);

    Copy(PhilosophersBMPX3, Coordinates.I(1, 4), Coordinates.I(2, 4),
        Coordinates.I(3, 4), Coordinates.I(4, 4),
        PhilosopherBrown);
}
void PrintBluePhilosophers(int x, int y) {
    PasteNon0(PhilosopherBlue, x, y, screenMatrix); // Paste on screen
}
//print red on scene
void PrintRedPhilosophers(int x, int y) {
    PasteNon0(PhilosopherRed, x, y, screenMatrix); // Paste on screen
}

void PrintGreenPhilosophers(int x, int y) {
    PasteNon0(PhilosopherGreen, x, y, screenMatrix); // Paste on screen
}

void PrintBrownPhilosophers(int x, int y) {
    PasteNon0(PhilosopherBrown, x, y, screenMatrix); // Paste on screen
}
// Semaphore handles for chopsticks
HANDLE chopsticks[NUM_PHILOSOPHERS];
bool chopstickAvailable[NUM_PHILOSOPHERS];

// Semaphore mode flag
bool isSemaphoreMode = false;

// Function declarations
void PhilosopherNonSemaphore(int id);
void PhilosopherSemaphore(int id);
void DrawDiningPhilosophers(ICBYTES& matrix);
void StartNonSemaphore();
void StartWithSemaphore();
DWORD WINAPI DrawThread(LPVOID lpParam);
void PrintNumbertoScreen(char* label, const char* base, int num);

// Utility functions for chopstick management   
void PickUpChopsticks(int id, bool isSemaphoreMode);
void PutDownChopsticks(int id);

// Chopstick handling for non-semaphore mode
void PickUpChopsticks(int id, bool isSemaphoreMode, int* hungryTime) {
    int left = id;
    int right = (id + 1) % NUM_PHILOSOPHERS;
    if (chopstickAvailable[left] && chopstickAvailable[right]) {
        chopstickAvailable[left] = false;
        chopstickAvailable[right] = false;
        philosopherStates[id] = EATING;
        *hungryTime = 0;
    }
    else if (chopstickAvailable[left] && !isSemaphoreMode) {
        chopstickAvailable[left] = false;
    }
    else if (chopstickAvailable[right] && !isSemaphoreMode) {
        chopstickAvailable[right] = false;
    }
}

void PutDownChopsticks(int id) {
    int left = id;
    int right = (id + 1) % NUM_PHILOSOPHERS;
    chopstickAvailable[left] = true;
    chopstickAvailable[right] = true;
    philosopherStates[id] = THINKING;
}

// Philosopher thread function (non-semaphore mode)
void PhilosopherNonSemaphore(int id) {
    int hungryTime = 0;
    philosopherStates[id] = THINKING;

    while (philosopherStates[id] != STARVED) {
        Sleep(750);
        philosopherStates[id] = HUNGRY;
        Sleep(750);

        while (philosopherStates[id] == HUNGRY) {
            PickUpChopsticks(id, isSemaphoreMode, &hungryTime);
            if (philosopherStates[id] == EATING) {
                hungryTime = 0;
                Sleep(750);
                PutDownChopsticks(id);
                Sleep(750);
                break;
            }
            else {
                Sleep(100);
                hungryTime += 100;
                if (hungryTime >= 5000) {
                    philosopherStates[id] = STARVED;
                    break;
                }
            }
        }
    }
}

// Philosopher thread function (semaphore mode)
void PhilosopherSemaphore(int id) {
    int hungryTime = 0;
    philosopherStates[id] = THINKING;

    while (philosopherStates[id] != STARVED) {
        Sleep(750);
        philosopherStates[id] = HUNGRY;
        Sleep(750);

        // Pick up chopsticks (use binary semaphores)
        WaitForSingleObject(chopsticks[id], 100);
        WaitForSingleObject(chopsticks[(id + 1) % NUM_PHILOSOPHERS], 100);
        while (philosopherStates[id] == HUNGRY) {
            if (chopstickAvailable[id] == true && chopstickAvailable[id + 1] == true) {

                PickUpChopsticks(id, isSemaphoreMode, &hungryTime);
                Sleep(750);

                ReleaseSemaphore(chopsticks[id], 1, NULL);
                ReleaseSemaphore(chopsticks[(id + 1) % NUM_PHILOSOPHERS], 1, NULL);

                PutDownChopsticks(id);
                Sleep(750);
                philosopherStates[id] = THINKING;
            }
            else {
                Sleep(100);
                hungryTime += 100;
                if (hungryTime >= 5000) {
                    philosopherStates[id] = STARVED;
                    break;
                }
            }
        }
    }
}

// Drawing the dining philosophers and chopsticks
void DrawDiningPhilosophers(ICBYTES& matrix) {
    const int centerX = 230, centerY = 220, radius = 150;
    const double PI = 3.141592653589793;
    const double philosopherAngles[NUM_PHILOSOPHERS] = { 50, 130, 200, 270, 340 };
    const double chopstickAngles[NUM_PHILOSOPHERS] = { 20, 80, 160, 235, 318 };
    int constant = 5;

    // Draw philosophers
    for (int i = 0; i < NUM_PHILOSOPHERS; ++i) {
        int x = centerX + radius * cos(philosopherAngles[i] * PI / 180);
        int y = centerY + radius * sin(philosopherAngles[i] * PI / 180);

        int color = 0xFFFFFF; // Default white
        if (philosopherStates[i] == THINKING) PrintBluePhilosophers(x, y);
        else if (philosopherStates[i] == HUNGRY) PrintRedPhilosophers(x, y);
        else if (philosopherStates[i] == EATING) PrintGreenPhilosophers(x, y);
        else if (philosopherStates[i] == STARVED) PrintBrownPhilosophers(x, y);

        char label[4];
        PrintNumbertoScreen(label, "PL", i);
        Impress12x20(matrix, x + 15 * constant, y + 7 * constant, label, 0xFFFFFF);
    }

    // Draw chopsticks
    for (int i = 0; i < NUM_PHILOSOPHERS; ++i) {
        int x = centerX + (radius - (2 * constant)) * cos(chopstickAngles[i] * PI / 180);
        int y = centerY + (radius - (2 * constant)) * sin(chopstickAngles[i] * PI / 180);

        int color = chopstickAvailable[i] ? 0x00FF00 : 0xFF0000; // Green if available, red if not
        FillEllipse(matrix, x, y, 4 * constant, 4 * constant, color);

        char label[4];
        PrintNumbertoScreen(label, "CH", i);
        Impress12x20(matrix, x + 1 * constant, y + 2 * constant, label, 0xFFFFFF);
    }
}

// Utility to print labels
void PrintNumbertoScreen(char* label, const char* base, int num) {
    label[0] = base[0];
    label[1] = base[1];
    label[2] = '0' + num;
    label[3] = '\0';
}

// Drawing thread
DWORD WINAPI DrawThread(LPVOID lpParam) {
    while (true) {
        screenMatrix = 0; // Clear the screen
        DrawDiningPhilosophers(screenMatrix);
        DisplayImage(F1, screenMatrix);
        Sleep(30); // Refresh every 30 ms
    }
}

// Start non-semaphore mode
void StartNonSemaphore() {
    for (int i = 0; i < NUM_PHILOSOPHERS; ++i) {
        chopstickAvailable[i] = true;
    }
    isSemaphoreMode = false;
    for (int i = 0; i < NUM_PHILOSOPHERS; ++i) {
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)PhilosopherNonSemaphore, (LPVOID)i, 0, NULL);
    }
    CreateThread(NULL, 0, DrawThread, NULL, 0, NULL);
}

// Start semaphore mode
void StartWithSemaphore() {
    for (int i = 0; i < NUM_PHILOSOPHERS; ++i) {
        chopstickAvailable[i] = true;
        chopsticks[i] = CreateSemaphore(NULL, 1, 1, NULL);
    }
    isSemaphoreMode = true;
    for (int i = 0; i < NUM_PHILOSOPHERS; ++i) {
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)PhilosopherSemaphore, (LPVOID)i, 0, NULL);
    }
    CreateThread(NULL, 0, DrawThread, NULL, 0, NULL);
}

// GUI setup
void ICGUI_Create() {
    ICG_MWTitle("Dining Philosophers");
    ICG_MWSize(540, 600);
}

void ICGUI_main() {
    F1 = ICG_FrameMedium(5, 40, 1, 1);
    ReadImage("Assests/sprites.bmp", PhilosophersBMP);
    MagnifyX3(PhilosophersBMP, PhilosophersBMPX3);
    PreparePhilosophersBMP();
    ICG_Button(5, 5, 150, 25, "Start NonSemaphore", StartNonSemaphore);
    ICG_Button(300, 5, 150, 25, "Start Semaphore", StartWithSemaphore);
    CreateImage(screenMatrix, 500, 500, ICB_UINT);
}