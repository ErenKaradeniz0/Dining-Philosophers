#include "icb_gui.h"
#include <cmath>
ICBYTES screenMatrix;

enum PhilosophersState {
    THINKING,
    HUNGRY,
    EATING
};

int F1;

void ICGUI_Create() {
    ICG_MWTitle("Dining Philosophers");
    ICG_MWSize(540, 600);
}

void PrintNumbertoScreen(char* label, const char* prefix, int number) {
    label[0] = prefix[0];          // First character of the prefix
    label[1] = prefix[1];          // Second character of the prefix
    label[2] = '0' + number;       // Convert number to string and append
    label[3] = '\0';               // Null terminator
}

void DrawDiningPhilosophers(ICBYTES& matrix) {
    // Center coordinates of the circle
    const int centerX = 220; // X-axis center
    const int centerY = 220; // Y-axis center
    const int radius = 150;  // Circle radius (distance from center)

    // Angles to determine the positions of philosophers and chopsticks on the circle
    const double PI = 3.141592653589793;
    const double philosopherAngles[5] = { 270, 340, 50, 130, 200 }; // Angles in degrees (positions of philosophers)
    const double chopstickAngles[5] = { 315, 20, 80, 155, 235 };   // Angles in degrees (positions of chopsticks)
    int constant = 5;

    // Place philosophers on the circle
    for (int i = 0; i < 5; ++i) {
        int x = centerX + radius * cos(philosopherAngles[i] * PI / 180);
        int y = centerY + radius * sin(philosopherAngles[i] * PI / 180);
        FillEllipse(matrix, x, y, 8 * constant, 8 * constant, 0x0000FF); // Philosophers (blue)
        // Generate philosopher label
        char philosopherLabel[4];
        PrintNumbertoScreen(philosopherLabel, "PL", i);
        Impress12x20(matrix, x + 5 * constant, y + 7 * constant, philosopherLabel, 0xFFFFFF); // Print ID
    }

    // Place chopsticks on the circle
    for (int i = 0; i < 5; ++i) {
        int x = centerX + (radius - (4 * constant)) * cos(chopstickAngles[i] * PI / 180);
        int y = centerY + (radius - (4 * constant)) * sin(chopstickAngles[i] * PI / 180);
        FillEllipse(matrix, x, y, 4 * constant, 4 * constant, 0x00FF00); // Chopsticks (green)
        // Generate chopstick label
        char chopstickLabel[4];
        PrintNumbertoScreen(chopstickLabel, "CH", i);

        Impress12x20(matrix, x + 1 * constant, y + 2 * constant, chopstickLabel, 0xFFFFFF); // Print ID
    }
}

// Drawing thread
void DrawThread() {
    while (true) {
        screenMatrix = 0; // Clear the screen
        DrawDiningPhilosophers(screenMatrix); // Draw Dining Philosophers
        DisplayImage(F1, screenMatrix); // Send drawing to the screen
        Sleep(30); // Refresh rate
    }
}

void StartNonSemaphore() {
    SetFocus(ICG_GetMainWindow());
    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)DrawThread, NULL, 0, NULL);
}

void StartWithSemaphore() {
    SetFocus(ICG_GetMainWindow());
    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)DrawThread, NULL, 0, NULL);
}

void ICGUI_main() {
    F1 = ICG_FrameMedium(5, 40, 1, 1);
    ICG_Button(5, 5, 150, 25, "Start NonSemaphore", StartNonSemaphore);
    ICG_Button(300, 5, 150, 25, "Start Semaphore ", StartWithSemaphore);
    CreateImage(screenMatrix, 500, 500, ICB_UINT);
};