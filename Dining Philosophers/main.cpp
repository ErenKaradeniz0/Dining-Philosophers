﻿#include <windows.h>
#include <cmath>
#include "icb_gui.h"

struct SpagetthiStruct
{
    int x;
    int y;
    int State;
};

enum State { THINKING, HUNGRY, EATING, STARVED };

struct Philosopher {
    int id;
    int x, y;
    State state;
    bool hasImage;
    ICBYTES CurrentImage;

    Philosopher() : id(-1), x(0), y(0), state(THINKING), hasImage(false) {}

    void SetId(int philosopher_id)
    {
        id = philosopher_id;
        hasImage = true;
        state = THINKING;
    }

};

struct Chopstic {
    int x;
    int y;
    int centerX;
    int centerY;
    int width;
    int height;
    int Color;
    bool Available;
};

// Global variables
ICBYTES screenMatrix, PhilosophersBMP, PhilosophersBMPX3;
ICBYTES Spaghetti;

int F1;
// Constants
const int NUM_PHILOSOPHERS = 5;
Philosopher philosophers[NUM_PHILOSOPHERS];
SpagetthiStruct SpaghettiPlate[NUM_PHILOSOPHERS];
Chopstic chopsticks[NUM_PHILOSOPHERS];

// Semaphore handles for chopsticks
HANDLE chopsticksHandle[NUM_PHILOSOPHERS];
HANDLE threads[NUM_PHILOSOPHERS + 1]; // +1 for DrawThread
// Semaphore mode flag
bool isSemaphoreMode = false;

// Coordinates
ICBYTES Coordinates{
    {6, 6, 75, 96},     // Red Front
    {87, 6, 75, 96},    // Blue Front
    {168, 6, 75, 96},   // Green Front
    {249, 6, 75, 96},   // Brown Front

    {27, 213, 45, 90},     // Red Left
    {78, 213, 45, 90},    // Blue Left
    {129, 213, 45, 90},   // Green Left
    {183, 213, 45, 90},   // Brown Left

    {18, 114, 51, 90},     // Red Right
    {72, 114, 51, 90},    // Blue Right
    {126, 114, 51, 90},   // Green Right
    {180, 114, 51, 90},   // Brown Right
};

// Mathematical variables
const int centerX = 230, centerY = 220, radius = 150;
const double PI = 3.141592653589793;
const double philosopherAngles[NUM_PHILOSOPHERS] = { 270, 340, 50, 125, 200 };
const double chopstickAngles[NUM_PHILOSOPHERS] = { 240,300, 10, 85,170 };
int constant = 5;

// Function declarations
void PhilosopherNonSemaphore(int id);
void PhilosopherSemaphore(int id);
void DrawDiningPhilosophers(ICBYTES& matrix);
void StartNonSemaphore();
void StartWithSemaphore();
DWORD WINAPI DrawThread(LPVOID lpParam);
void PrintNumbertoScreen(char* label, const char* base, int num);
// Utility functions for chopstick management   
void PickUpChopsticks(int id, bool isSemaphoreMode, int& hungryTime);
void PutDownChopsticks(int id);

// f for fast, s for slow
int SetSpeed(char speed) {
    // Lambda
    switch (speed) {
    case 'f':
        return 300; // Fast
    case 's':
        return 1000; // Slow
    default:
        return 1000; // Default
    }
    return 1000;
}

int sleepDuration = SetSpeed('s'); //s or f

bool CheckDeadLock() {
    int ct = 0;
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        if(philosophers[i].state == STARVED)
            ct++;
    }
    if (ct == NUM_PHILOSOPHERS-1) {
        return true;
    }
    return false;
}

//Copy Spagetti Sprite from SpagettiState 
void SpaghettiState(int c) {
    switch (c)
    {
    case 0: { Copy(PhilosophersBMPX3, 270, 138, 30, 30, Spaghetti); break; }    //  Spaghetti %100
    case 1: { Copy(PhilosophersBMPX3, 270, 177, 30, 30, Spaghetti); break; }    //  Spaghetti %66
    case 2: { Copy(PhilosophersBMPX3, 270, 216, 30, 30, Spaghetti); break; }    //  Spaghetti %33
    case 3: { Copy(PhilosophersBMPX3, 270, 255, 30, 30, Spaghetti); break; }     // Spaghetti %0
    default:
        break;
    }
}

// Print Spagetti
void SpaghettiPrint(int x, int y) {
    PasteNon0(Spaghetti, x, y, screenMatrix);
}

// Prepare sprites for print
void PreparePhilosopher(Philosopher& philosopher) {
    if (philosopher.id == 0) { //front
        if (philosopher.state == THINKING) {
            Copy(PhilosophersBMPX3, Coordinates.I(1, 2), Coordinates.I(2, 2),
                Coordinates.I(3, 2), Coordinates.I(4, 2),
                philosopher.CurrentImage);
        }
        else if (philosopher.state == HUNGRY) {
            Copy(PhilosophersBMPX3, Coordinates.I(1, 1), Coordinates.I(2, 1),
                Coordinates.I(3, 1), Coordinates.I(4, 1),
                philosopher.CurrentImage);
        }
        else if (philosopher.state == EATING) {
            Copy(PhilosophersBMPX3, Coordinates.I(1, 3), Coordinates.I(2, 3),
                Coordinates.I(3, 3), Coordinates.I(4, 3),
                philosopher.CurrentImage);

        }
        else if (philosopher.state == STARVED) {
            Copy(PhilosophersBMPX3, Coordinates.I(1, 4), Coordinates.I(2, 4),
                Coordinates.I(3, 4), Coordinates.I(4, 4),
                philosopher.CurrentImage);
        }
    }
    else if (philosopher.id == 1 || philosopher.id == 2) {
        if (philosopher.state == THINKING) {
            Copy(PhilosophersBMPX3, Coordinates.I(1, 6), Coordinates.I(2, 6),
                Coordinates.I(3, 6), Coordinates.I(4, 6),
                philosopher.CurrentImage);
        }
        else if (philosopher.state == HUNGRY) {
            Copy(PhilosophersBMPX3, Coordinates.I(1, 5), Coordinates.I(2, 5),
                Coordinates.I(3, 5), Coordinates.I(4, 5),
                philosopher.CurrentImage);
        }
        else if (philosopher.state == EATING) {
            Copy(PhilosophersBMPX3, Coordinates.I(1, 7), Coordinates.I(2, 7),
                Coordinates.I(3, 7), Coordinates.I(4, 7),
                philosopher.CurrentImage);

        }
        else if (philosopher.state == STARVED) {
            Copy(PhilosophersBMPX3, Coordinates.I(1, 8), Coordinates.I(2, 8),
                Coordinates.I(3, 8), Coordinates.I(4, 8),
                philosopher.CurrentImage);
        }
    }
    else if (philosopher.id == 3 || philosopher.id == 4) {
        if (philosopher.state == THINKING) {
            Copy(PhilosophersBMPX3, Coordinates.I(1, 10), Coordinates.I(2, 10),
                Coordinates.I(3, 10), Coordinates.I(4, 10),
                philosopher.CurrentImage);
        }
        else if (philosopher.state == HUNGRY) {
            Copy(PhilosophersBMPX3, Coordinates.I(1, 9), Coordinates.I(2, 9),
                Coordinates.I(3, 9), Coordinates.I(4, 9),
                philosopher.CurrentImage);
        }
        else if (philosopher.state == EATING) {
            Copy(PhilosophersBMPX3, Coordinates.I(1, 11), Coordinates.I(2, 11),
                Coordinates.I(3, 11), Coordinates.I(4, 11),
                philosopher.CurrentImage);

        }
        else if (philosopher.state == STARVED) {
            Copy(PhilosophersBMPX3, Coordinates.I(1, 12), Coordinates.I(2, 12),
                Coordinates.I(3, 12), Coordinates.I(4, 12),
                philosopher.CurrentImage);
        }
    }

}

// Philosophers print function on screen
void PrintPhilosophers() {
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        if (philosophers[i].hasImage)
            PasteNon0(philosophers[i].CurrentImage, philosophers[i].x, philosophers[i].y, screenMatrix);
    }
}
void PhilosopherChangeState(int id, State newState) {
    philosophers[id].state = newState;
    PreparePhilosopher(philosophers[id]);
}
// Chopstick handling for non-semaphore mode
void PickUpChopsticks(int id, bool isSemaphoreMode, int& hungryTime) {
    int left = id;
    int right = (id + 1) % NUM_PHILOSOPHERS;
    if (chopsticks[left].Available && chopsticks[right].Available) {
        chopsticks[left].Available = false;
        chopsticks[right].Available = false;
        PhilosopherChangeState(id, EATING);
        hungryTime = 0;
    }
    else if (chopsticks[left].Available && !isSemaphoreMode) {
        chopsticks[left].Available = false;
    }
    else if (chopsticks[right].Available && !isSemaphoreMode) {
        chopsticks[right].Available = false;
    }
}

void PutDownChopsticks(int id) {
    int left = id;
    int right = (id + 1) % NUM_PHILOSOPHERS;
    chopsticks[left].Available = true;
    chopsticks[right].Available = true;
    PhilosopherChangeState(id, THINKING);
}

// Philosopher thread function (non-semaphore mode)
void PhilosopherNonSemaphore(int id) {
    int hungryTime = 0;
    PhilosopherChangeState(id, THINKING);
    while (philosophers[id].state != STARVED) {
        PreparePhilosopher(philosophers[id]);
        Sleep(sleepDuration);
        PhilosopherChangeState(id, HUNGRY);

        PickUpChopsticks(id, isSemaphoreMode, hungryTime);
        if (philosophers[id].state == EATING) {
            for (int i = 0; i < 4; i++) {
                SpaghettiPlate[id].State = i;
                Sleep(sleepDuration / 4);
            }
            hungryTime = 0;
            PutDownChopsticks(id);
            Sleep(sleepDuration);
        }
        else if (philosophers[id].state == HUNGRY) {
            // Açlık süresini artır
            Sleep(100);
            hungryTime += 100;
            if (hungryTime >= sleepDuration) {
                PhilosopherChangeState(id, STARVED);

            }
        }
    }
}

// Philosopher thread function (semaphore mode)
void PhilosopherSemaphore(int id) {
    int hungryTime = 0;
    int left = id;
    int right = (id + 1) % NUM_PHILOSOPHERS;

    while (philosophers[id].state != STARVED) {
        PreparePhilosopher(philosophers[id]);
        philosophers[id].state = HUNGRY;
        Sleep(sleepDuration);
        hungryTime += 100;

        if (hungryTime >= 3 * sleepDuration) {
            PhilosopherChangeState(id, STARVED);
            break;
        }

        // Pick up chopsticks (use binary semaphores)
        WaitForSingleObject(chopsticksHandle[left], sleepDuration);
        WaitForSingleObject(chopsticksHandle[right], sleepDuration);

        if (chopsticks[left].Available && chopsticks[right].Available) {
            PickUpChopsticks(id, isSemaphoreMode, hungryTime);

            for (int i = 0; i < 4; i++) {
                SpaghettiPlate[id].State = i;
                Sleep(sleepDuration / 2);
            }

            ReleaseSemaphore(chopsticksHandle[left], 1, NULL);
            ReleaseSemaphore(chopsticksHandle[right], 1, NULL);

            PutDownChopsticks(id);
            philosophers[id].state = THINKING;
            hungryTime = 0;
            Sleep(2 * sleepDuration);
        }
        else {
            Sleep(100);
        }
    }
}

// Calculate Coordinates of the dining philosophers and chopsticks
void CalculateCoordinate() {

    // Philosopher
    int ph_x, ph_y;

    // SpaghettiPlate
    int sp_x, sp_y;

    // Chopstick
    int ch_centerX, ch_centerY, ch_Width, ch_Height, ch_rectX, ch_rectY;

    for (int id = 0; id < NUM_PHILOSOPHERS; ++id) {
        // Philosopher
        ph_x = centerX + (radius + 10) * cos(philosopherAngles[id] * PI / 180);
        ph_y = centerY + (radius + 10) * sin(philosopherAngles[id] * PI / 180);

        // SpaghettiPlate
        sp_x = centerX + 20 + (radius - 75) * cos(philosopherAngles[id] * PI / 180);
        sp_y = centerY + 30 + (radius - 75) * sin(philosopherAngles[id] * PI / 180);

        ch_centerX = centerX + (radius - (15 * constant)) * cos(chopstickAngles[id] * PI / 180);
        ch_centerY = centerY + (radius - (15 * constant)) * sin(chopstickAngles[id] * PI / 180);
        ch_Width = constant;
        ch_Height = 6 * constant;

        // Chopstick
        ch_rectX = ch_centerX - (ch_Width / 2) + 40;
        ch_rectY = ch_centerY - (ch_Height / 2) + 50;

        philosophers[id].x = ph_x;
        philosophers[id].y = ph_y;
        philosophers[id].state = THINKING;

        SpaghettiPlate[id] = { sp_x, sp_y, 3 };

        chopsticks[id] = { ch_rectX, ch_rectY, ch_centerX, ch_centerY, ch_Width, ch_Height, 0x00FF00, true };

    }
}

// Drawing the dining philosophers and chopsticks
void DrawDiningPhilosophers(ICBYTES& matrix) {
    char label[4];
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {

        // Draw philosophers
        PrintPhilosophers();
        PrintNumbertoScreen(label, "PL", i + 1);
        Impress12x20(matrix, philosophers[i].x + 5 * constant, philosophers[i].y - 5 * constant, label, 0xFFFFFF);

        // DrawSpaghetti
        SpaghettiState(SpaghettiPlate[i].State);
        SpaghettiPrint(SpaghettiPlate[i].x, SpaghettiPlate[i].y);

        // Set the color of chopstick based on availability
        chopsticks[i].Color = chopsticks[i].Available ? 0x00FF00 : 0xFF0000; // Green if available, red if not

        // Draw chopsticks
        FillRect(matrix, chopsticks[i].x, chopsticks[i].y, chopsticks[i].width, chopsticks[i].height, chopsticks[i].Color);
        PrintNumbertoScreen(label, "CH", i + 1);
        Impress12x20(matrix, chopsticks[i].centerX, chopsticks[i].centerY + 40, label, 0xFFFFFF);

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
    //CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)PrintMacaroni, NULL, 0, NULL);

    screenMatrix = 0; // Clear the screen

    //Draw Table
    FillEllipse(screenMatrix, 165, 165, 100, 100, 0xA1662F);

    ICBYTES test;
    //Copy(PhilosophersBMPX3, 4, 4 ,75, 93, test); //FRONT RED
    //test

    PasteNon0(test, 1, 1, screenMatrix); // Paste on screen

    while (true) {
        if (CheckDeadLock()) {
            PlaySound("Sound/ChineseDOIT.wav", NULL, SND_ASYNC);
        }
        DrawDiningPhilosophers(screenMatrix);
        DisplayImage(F1, screenMatrix);
        Sleep(10);
    }
}

void KillThreads() {
    for (int i = 0; i < NUM_PHILOSOPHERS + 1; ++i) {
        if (threads[i] != NULL) {
            TerminateThread(threads[i], 0);
            CloseHandle(threads[i]);
            threads[i] = NULL;
        }
    }
}

void* MusicControllerThread(LPVOID lpParam) {
    // ASYNC
    PlaySound("Sound/ChineseGong.wav", NULL, SND_ASYNC);
    return nullptr;
}

// Start non-semaphore mode
void StartNonSemaphore() {
    KillThreads(); // Kill previous threads
    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)MusicControllerThread, NULL, 0, NULL);
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        philosophers[i].SetId(i);
        chopsticks[i].Available = true;
        SpaghettiPlate[i].State = 3;
    }
    isSemaphoreMode = false;
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        threads[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)PhilosopherNonSemaphore, (LPVOID)i, 0, NULL);
    }
    threads[NUM_PHILOSOPHERS] = CreateThread(NULL, 0, DrawThread, NULL, 0, NULL);
}

// Start semaphore mode
void StartWithSemaphore() {
    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)MusicControllerThread, NULL, 0, NULL);
    KillThreads(); // Kill previous threads
    for (int i = 0; i < NUM_PHILOSOPHERS; ++i) {
        philosophers[i].SetId(i);
        chopsticks[i].Available = true;
        SpaghettiPlate[i].State = 3;
        chopsticksHandle[i] = CreateSemaphore(NULL, 1, 1, NULL);
    }
    isSemaphoreMode = true;
    for (int i = 0; i < NUM_PHILOSOPHERS; ++i) {
        threads[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)PhilosopherSemaphore, (LPVOID)i, 0, NULL);
    }
    threads[NUM_PHILOSOPHERS] = CreateThread(NULL, 0, DrawThread, NULL, 0, NULL);
}

// GUI setup
void ICGUI_Create() {
    ICG_MWTitle("Dining Philosophers");
    ICG_MWSize(540, 600);
}

void ICGUI_main() {
    F1 = ICG_FrameMedium(5, 40, 1, 1);
    CalculateCoordinate();  

    //test
    ReadImage("Assests/sprites.bmp", PhilosophersBMP);
    MagnifyX3(PhilosophersBMP, PhilosophersBMPX3);

    ICG_Button(5, 5, 150, 25, "Start NonSemaphore", StartNonSemaphore);
    ICG_Button(300, 5, 150, 25, "Start Semaphore", StartWithSemaphore);
    CreateImage(screenMatrix, 500, 500, ICB_UINT);
}