#include <windows.h>
#include <cmath>
#include "icb_gui.h"
ICBYTES screenMatrix;
int F1;

// Constants
const int NUM_PHILOSOPHERS = 5;
// Philosopher states
enum State { THINKING, HUNGRY, EATING, STARVED };
State philosopherStates[NUM_PHILOSOPHERS];

// Semaphore handles for chopsticks
HANDLE chopsticks[NUM_PHILOSOPHERS];

// Semaphore mode flag
bool isSemaphoreMode = false;

// Function declarations
void PhilosopherNonSemaphore(int id);
void PhilosopherSemaphore(int id);
void DrawDiningPhilosophers(ICBYTES& matrix);
void StartNonSemaphore();
void StartWithSemaphore();

DWORD WINAPI DrawThread(LPVOID lpParam);

// Drawing utility
void PrintNumbertoScreen(char* label, const char* base, int num);

// Philosopher logic without semaphores (deadlock may occur here)

void PhilosopherNonSemaphore(int id) {
    int left = id;  // Left chopstick
    int right = (id + 1) % NUM_PHILOSOPHERS;  // Right chopstick

    int hungryTime = 0;  // Tracks how long philosopher has been hungry

    while (true) {
        // Think for a while
        philosopherStates[id] = THINKING;
        Sleep(1000);  // Thinking for 1 second

        // Get hungry
        philosopherStates[id] = HUNGRY;
        Sleep(500);  // Feeling hungry for 0.5 seconds

        // Try picking up chopsticks
        while (true) {
            if (philosopherStates[left] != EATING && philosopherStates[right] != EATING) {
                // If both chopsticks are free, pick them up and start eating
                philosopherStates[id] = EATING;
                Sleep(1000);  // Eat for 1 second
                philosopherStates[id] = THINKING;  // Done eating, go back to thinking
                break;  // Exit the loop and continue with the next cycle
            }
            else {
                // If at least one chopstick is being used, philosopher stays hungry
                philosopherStates[id] = HUNGRY;
                Sleep(100);  // Retry after short delay

                // Increase hungry time
                hungryTime += 100;
                if (hungryTime >= 1000) {  // If philosopher is hungry for 1 seconds
                    philosopherStates[id] = STARVED;  // Starved condition
                    break;  // Exit the loop and move to the next cycle
                }
            }
        }

        // If philosopher is starved, break out of the loop (they will stop trying to eat)
        if (philosopherStates[id] == STARVED) {
            break;
        }
    }
}


// Philosopher logic with semaphores (using Windows API semaphores)
void PhilosopherSemaphore(int id) {
    int left = id;               // Left chopstick
    int right = (id + 1) % NUM_PHILOSOPHERS; // Right chopstick

    while (1) {
        // Think
        philosopherStates[id] = THINKING;
        Sleep(1000); // Sleep for 1 second

        // Get hungry
        philosopherStates[id] = HUNGRY;
        Sleep(500); // Sleep for 0.5 seconds

        // Pick up chopsticks (use binary semaphores)
        WaitForSingleObject(chopsticks[left], INFINITE);
        WaitForSingleObject(chopsticks[right], INFINITE);

        // Eat
        philosopherStates[id] = EATING;
        Sleep(1000); // Sleep for 1 second

        // Put down chopsticks
        ReleaseSemaphore(chopsticks[left], 1, NULL);
        ReleaseSemaphore(chopsticks[right], 1, NULL);
    }
}

// Drawing function
void DrawDiningPhilosophers(ICBYTES& matrix) {
    const int centerX = 230;
    const int centerY = 220;
    const int radius = 150;

    const double PI = 3.141592653589793;
    const double philosopherAngles[5] = { 50, 130, 200, 270, 340 };
    const double chopstickAngles[5] = { 20, 80, 160, 235, 318 };
    int constant = 5;

    // Draw philosophers
    for (int i = 0; i < NUM_PHILOSOPHERS; ++i) {
        int x = centerX + radius * cos(philosopherAngles[i] * PI / 180);
        int y = centerY + radius * sin(philosopherAngles[i] * PI / 180);
        FillEllipse(matrix, x, y, 8 * constant, 8 * constant, 0x0000FF);

        // Color based on state
        int color = 0xFFFFFF; // Default white
        if (philosopherStates[i] == THINKING)
            color = 0xFFFF00; // Yellow
        else if (philosopherStates[i] == HUNGRY)
            color = 0xFF0000; // Red
        else if (philosopherStates[i] == EATING)
            color = 0x00FF00; // Green        
        else if (philosopherStates[i] == STARVED)
            color = 0x000000; // Black

        char label[4];
        PrintNumbertoScreen(label, "PL", i);
        Impress12x20(matrix, x + 5 * constant, y + 7 * constant, label, color);
    }

    // Draw chopsticks
    for (int i = 0; i < NUM_PHILOSOPHERS; ++i) {
        int x = centerX + (radius - (2 * constant)) * cos(chopstickAngles[i] * PI / 180);
        int y = centerY + (radius - (2 * constant)) * sin(chopstickAngles[i] * PI / 180);
        FillEllipse(matrix, x, y, 4 * constant, 4 * constant, 0x00FF00);

        char label[4];
        PrintNumbertoScreen(label, "CH", i);
        Impress12x20(matrix, x + 1 * constant, y + 2 * constant, label, 0xFFFFFF);
    }
}

// Utility to print a label
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
        DrawDiningPhilosophers(screenMatrix); // Draw philosophers
        DisplayImage(F1, screenMatrix); // Send to the screen
        Sleep(30); // Sleep for 30 milliseconds
    }
}

// Start functions
void StartNonSemaphore() {
    isSemaphoreMode = false;
    for (int i = 0; i < NUM_PHILOSOPHERS; ++i) {
        // Initialize binary semaphores for each chopstick with a value of 1
        chopsticks[i] = CreateSemaphore(NULL, 1, 1, NULL);
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)PhilosopherNonSemaphore, (LPVOID)i, 0, NULL); // Using CreateThread
    }
    CreateThread(NULL, 0, DrawThread, NULL, 0, NULL); // Start drawing thread
}

void StartWithSemaphore() {
    isSemaphoreMode = true;
    for (int i = 0; i < NUM_PHILOSOPHERS; ++i) {
        // Initialize binary semaphores for each chopstick with a value of 1
        chopsticks[i] = CreateSemaphore(NULL, 1, 1, NULL);
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)PhilosopherSemaphore, (LPVOID)i, 0, NULL); // Using CreateThread
    }
    CreateThread(NULL, 0, DrawThread, NULL, 0, NULL); // Start drawing thread
}

void ICGUI_Create() {
    ICG_MWTitle("Dining Philosophers");
    ICG_MWSize(540, 600);
}

void ICGUI_main() {
    F1 = ICG_FrameMedium(5, 40, 1, 1);
    ICG_Button(5, 5, 150, 25, "Start NonSemaphore", StartNonSemaphore);
    ICG_Button(300, 5, 150, 25, "Start Semaphore ", StartWithSemaphore);
    CreateImage(screenMatrix, 500, 500, ICB_UINT);
};
