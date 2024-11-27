#include <windows.h>
#include <cmath>
#include "icb_gui.h"

// Global variables
ICBYTES screenMatrix;
int F1;

// Constants
const int NUM_PHILOSOPHERS = 5;

// Philosopher states
enum State { THINKING, HUNGRY, EATING, STARVED };
State philosopherStates[NUM_PHILOSOPHERS];

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
void PickUpChopsticks(int id, bool isSemaphoreMode,int* hungryTime) {
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
                if (hungryTime >= 3000) {
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
        Sleep(100);
        philosopherStates[id] = HUNGRY;
        Sleep(100);

        // Pick up chopsticks (use binary semaphores)
        WaitForSingleObject(chopsticks[id], 100);
        WaitForSingleObject(chopsticks[(id + 1) % NUM_PHILOSOPHERS], 100);

        if (chopstickAvailable[id] == true && chopstickAvailable[id + 1] == true) {

            PickUpChopsticks(id, isSemaphoreMode,&hungryTime);
            Sleep(100);

            ReleaseSemaphore(chopsticks[id], 1, NULL);
            ReleaseSemaphore(chopsticks[(id + 1) % NUM_PHILOSOPHERS], 1, NULL);

            PutDownChopsticks(id);
        }

        philosopherStates[id] = THINKING;
        Sleep(100);
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
        if (philosopherStates[i] == THINKING) color = 0x0000FF; // Blue
        else if (philosopherStates[i] == HUNGRY) color = 0xFF0000; // Red
        else if (philosopherStates[i] == EATING) color = 0x00FF00; // Green
        else if (philosopherStates[i] == STARVED) color = 0x000000; // Black

        FillEllipse(matrix, x, y, 8 * constant, 8 * constant, color);

        char label[4];
        PrintNumbertoScreen(label, "PL", i);
        Impress12x20(matrix, x + 5 * constant, y + 7 * constant, label, 0xFFFFFF);
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
    ICG_Button(5, 5, 150, 25, "Start NonSemaphore", StartNonSemaphore);
    ICG_Button(300, 5, 150, 25, "Start Semaphore", StartWithSemaphore);
    CreateImage(screenMatrix, 500, 500, ICB_UINT);
}