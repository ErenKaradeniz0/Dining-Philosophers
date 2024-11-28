#include <windows.h>
#include <cmath>
#include "icb_gui.h"

// Global variables
ICBYTES screenMatrix, PhilosophersBMP, PhilosophersBMPX3;
ICBYTES PhilosopherBlue, PhilosopherRed, PhilosopherGreen, PhilosopherBrown;
ICBYTES Spaghetti;

//test
ICBYTES PhilosophersBMP2, PhilosophersBMP2X3; 

// f for fast, s for slow
int SetSpeed(char speed) {
    // Lambda
    switch (speed) {
    case 'f':
        return 300; // Fast
    case 's':
        return 2000; // Slow
    default:
        return 1000; // Default
    }
    return 1000;
}

int sleepDuration = SetSpeed('s'); //s or f

struct SpagetthiStruct
{
    int x;
    int y;
    int State;
    //Again Push
};

enum State { THINKING, HUNGRY, EATING, STARVED };

struct Philosopher {
    int x;
    int y;
    State State;
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

int F1;

// Constants
const int NUM_PHILOSOPHERS = 5;

// Philosopher states

Philosopher  philosophers[NUM_PHILOSOPHERS];

SpagetthiStruct SpaghettiPlate[NUM_PHILOSOPHERS];

Chopstic chopsticks[NUM_PHILOSOPHERS];

// Semaphore handles for chopsticks
HANDLE chopsticksHandle[NUM_PHILOSOPHERS];

// Semaphore mode flag
bool isSemaphoreMode = false;

//coordinates
ICBYTES Coordinates{
    {6, 6, 75, 96},     // Red Front
    {87, 6, 75, 96},    // Blue
    {168, 6, 75, 96},   // Green
    {249, 6, 75, 96},   // Brown
};

// Mathematical variables
const int centerX = 230, centerY = 220, radius = 150;
const double PI = 3.141592653589793;
const double philosopherAngles[NUM_PHILOSOPHERS] = { 270, 340, 50, 125, 200 };
const double chopstickAngles[NUM_PHILOSOPHERS] = { 240,300, 10, 85,170 };
int constant = 5;

void SpaghettiState(int c) {
    switch (c)
    {
    case 0: { Copy(PhilosophersBMPX3, 204, 105, 36, 30, Spaghetti); break; } // Macaroni %100
    case 1: { Copy(PhilosophersBMPX3, 165, 105, 30, 30, Spaghetti); break; }// Macaroni %66
    case 2: { Copy(PhilosophersBMPX3, 126, 105, 30, 30, Spaghetti); break; }// Macaroni %33
    case 3: { Copy(PhilosophersBMPX3, 87, 105, 30, 30, Spaghetti); break; }// Macaroni %0
    default:
        break;
    }
}

void SpaghettiPrint(int x,int y) {
    PasteNon0(Spaghetti, x, y, screenMatrix);
}

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
void PickUpChopsticks(int id, bool isSemaphoreMode, int& hungryTime) {
    int left = id;
    int right = (id + 1) % NUM_PHILOSOPHERS;
    if (chopsticks[left].Available && chopsticks[right].Available) {
        chopsticks[left].Available = false;
        chopsticks[right].Available = false;
        philosophers[id].State = EATING;
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
    philosophers[id].State = THINKING;
}

// Philosopher thread function (non-semaphore mode)
void PhilosopherNonSemaphore(int id) {
    int hungryTime = 0;
    philosophers[id].State = THINKING;
    while (philosophers[id].State != STARVED) {
        Sleep(sleepDuration);
        philosophers[id].State = HUNGRY;

        while (philosophers[id].State == HUNGRY) {
            PickUpChopsticks(id, isSemaphoreMode, hungryTime);
            if (philosophers[id].State == EATING) {
                for (int i = 0; i < 4; i++) {
                    SpaghettiPlate[id].State = i;
                    Sleep(sleepDuration / 4);
                }
                hungryTime = 0;
                PutDownChopsticks(id);
                Sleep(sleepDuration);
                break;
            }
            else {
                Sleep(100);
                hungryTime += 100;
                if (hungryTime >= 5000) {
                    philosophers[id].State = STARVED;
                    break;
                }
            }
        }
    }
}

// Philosopher thread function (semaphore mode)
void PhilosopherSemaphore(int id) {
    int hungryTime = 0;
    int left = id;
    int right = (id + 1) % NUM_PHILOSOPHERS;
    philosophers[id].State = THINKING;

    while (philosophers[id].State != STARVED) {
        Sleep(sleepDuration);
        philosophers[id].State = HUNGRY;
        Sleep(sleepDuration);

        hungryTime += 100;
        if (hungryTime >= 5000) {
            philosophers[id].State = STARVED;
            break;
        }
        // Pick up chopsticks (use binary semaphores)
        WaitForSingleObject(chopsticksHandle[left], sleepDuration);
        WaitForSingleObject(chopsticksHandle[right], sleepDuration);
        while (philosophers[id].State == HUNGRY) {
            if (chopsticks[left].Available == true && chopsticks[right].Available == true) {

                

                PickUpChopsticks(id, isSemaphoreMode, hungryTime);

                for (int i = 0; i < 4; i++) {
                    SpaghettiPlate[id].State = i;
                    Sleep(sleepDuration / 2);
                } 

                //Sleep(2*sleepDuration); --> for döngüsü sağlıyor

                /*Eğer nonSemafor un yeme hızı ile aynı çalışmasını istiyorsan
                    forun içindekini sleepDuration ı 2 yerine 4 e böl
                    aşağıdaki sleep i aç ama kat sayı çarpanı verme --> sleepDuration kadar beklesin*/




                ReleaseSemaphore(chopsticksHandle[left], 1, NULL);
                ReleaseSemaphore(chopsticksHandle[right], 1, NULL);

                PutDownChopsticks(id);
                Sleep(2 * sleepDuration);
                philosophers[id].State = THINKING;
            }
        }
    }
}


// Drawing the dining philosophers and chopsticks
void CalculateCoordinate() {
    
    // Philosopher
    int ph_x, ph_y;

    // SpaghettiPlate
    int sp_x, sp_y;

    // Chopstick
    int ch_centerX, ch_centerY, ch_Width, ch_Height, ch_rectX, ch_rectY;

    for (int i = 0; i < NUM_PHILOSOPHERS; ++i) {
        // Philosopher
        ph_x = centerX + radius * cos(philosopherAngles[i] * PI / 180);
        ph_y = centerY + radius * sin(philosopherAngles[i] * PI / 180);

        // SpaghettiPlate
        sp_x = centerX + 20 + (radius - 75) * cos(philosopherAngles[i] * PI / 180);
        sp_y = centerY + 30 + (radius - 75) * sin(philosopherAngles[i] * PI / 180);

        ch_centerX = centerX + (radius - (15 * constant)) * cos(chopstickAngles[i] * PI / 180);
        ch_centerY = centerY + (radius - (15 * constant)) * sin(chopstickAngles[i] * PI / 180);
        ch_Width = constant;
        ch_Height = 6 * constant;
        
        // Chopstick
        ch_rectX = ch_centerX - (ch_Width / 2) + 40;
        ch_rectY = ch_centerY - (ch_Height / 2) + 50;

        philosophers[i] = { ph_x, ph_y, THINKING };

        SpaghettiPlate[i] = { sp_x, sp_y, 3 }; 

        chopsticks[i] = { ch_rectX, ch_rectY, ch_centerX, ch_centerY, ch_Width, ch_Height, 0x00FF00, true };

    }
}

// Drawing the dining philosophers and chopsticks
void DrawDiningPhilosophers(ICBYTES& matrix) {
    char label[4];
    for (int i = 0; i < NUM_PHILOSOPHERS; ++i) {

        // Draw philosophers
        if (philosophers[i].State == THINKING) PrintBluePhilosophers(philosophers[i].x, philosophers[i].y);
        else if (philosophers[i].State == HUNGRY) PrintRedPhilosophers(philosophers[i].x, philosophers[i].y);
        else if (philosophers[i].State == EATING) PrintGreenPhilosophers(philosophers[i].x, philosophers[i].y);
        else if (philosophers[i].State == STARVED) PrintBrownPhilosophers(philosophers[i].x, philosophers[i].y);
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

    //test
    ICBYTES test;
    Copy(PhilosophersBMPX3, 4, 4 ,75, 93, test); //FRONT RED
    PasteNon0(test, 0, 0, screenMatrix); // Paste on screen

    while (true) {
        DrawDiningPhilosophers(screenMatrix);
        DisplayImage(F1, screenMatrix);
        Sleep(10); // Refresh every 30 ms
    }
}

// Start non-semaphore mode
void StartNonSemaphore() {
    for (int i = 0; i < NUM_PHILOSOPHERS; ++i) {
        chopsticks[i].Available = true;
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
        chopsticks[i].Available = true;
        chopsticksHandle[i] = CreateSemaphore(NULL, 1, 1, NULL);
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
    CalculateCoordinate();

    //test
    ReadImage("Assests/PLEren.bmp", PhilosophersBMP2);
    MagnifyX3(PhilosophersBMP2, PhilosophersBMP2X3);

    ReadImage("Assests/sprites.bmp", PhilosophersBMP);
    MagnifyX3(PhilosophersBMP, PhilosophersBMPX3);
    PreparePhilosophersBMP();
    ICG_Button(5, 5, 150, 25, "Start NonSemaphore", StartNonSemaphore);
    ICG_Button(300, 5, 150, 25, "Start Semaphore", StartWithSemaphore);
    CreateImage(screenMatrix, 500, 500, ICB_UINT);
}