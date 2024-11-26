#include "icb_gui.h"
#include <cmath>

ICBYTES screenMatrix;
int F1;

#define PHILOSOPHER_LIMIT 5
#define CHOPSTICK_LIMIT 5

enum PhilosophersState {
    THINKING,
    HUNGRY,
    EATING
};

struct Philosopher {
    PhilosophersState state;
    int chopstick; 
    int id;
    int x;
    int y;
};

int PhilosopherID;

int ChopstickCount = CHOPSTICK_LIMIT;
int PhilosophersCoordinate[PHILOSOPHER_LIMIT][2];
int ChopsticksCoordinate[CHOPSTICK_LIMIT][2];

const int centerX = 220; // Y-axis center
const int centerY = 220; // Y-axis center
const int radius = 150;  // Circle radius (distance from center)

// Angles to determine the positions of philosophers and chopsticks on the circle
const double PI = 3.141592653589793;
const double philosopherAngles[5] = { 270, 340, 50, 130, 200 }; // Angles in degrees (positions of philosophers)
const double chopstickAngles[5] = { 315, 20, 80, 155, 235 };   // Angles in degrees (positions of chopsticks)
int constant = 5;

char philosopherLabel[4];
char chopstickLabel[4];

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

void CalculateCoordinatePhilosophers() {
    for (int i = 0; i < 5; ++i) {
        int x = centerX + radius * cos(philosopherAngles[i] * PI / 180);
        int y = centerY + radius * sin(philosopherAngles[i] * PI / 180);
        PhilosophersCoordinate[i][0] = x;
        PhilosophersCoordinate[i][1] = y;
        FillEllipse(screenMatrix, x, y, 8 * constant, 8 * constant, 0x0000FF); // Philosophers (blue)
        // Generate philosopher label
        PrintNumbertoScreen(philosopherLabel, "PL", i);
        Impress12x20(screenMatrix, x + 5 * constant, y + 7 * constant, philosopherLabel, 0xFFFFFF); // Print ID
    }
}

void CalculateCoordinateChopsticks() {
    for (int i = 0; i < 5; ++i) {
        int x = centerX + (radius - (4 * constant)) * cos(chopstickAngles[i] * PI / 180);
        int y = centerY + (radius - (4 * constant)) * sin(chopstickAngles[i] * PI / 180);
        FillEllipse(screenMatrix, x, y, 4 * constant, 4 * constant, 0x00FF00); // Chopsticks (green)
        // Generate chopstick label
        char chopstickLabel[4];
        ChopsticksCoordinate[i][0] = x;
        ChopsticksCoordinate[i][1] = y;
        PrintNumbertoScreen(chopstickLabel, "CH", i);
        Impress12x20(screenMatrix, x + 1 * constant, y + 2 * constant, chopstickLabel, 0xFFFFFF); // Print ID
    }
}


void DrawDefaultImages() {
    for (int i = 0; i < 5; ++i) {
       
        FillEllipse(screenMatrix, PhilosophersCoordinate[i][0], PhilosophersCoordinate[i][1], 8 * constant, 8 * constant, 0x0000FF); // Philosophers (blue)
        // Generate philosopher label
        PrintNumbertoScreen(philosopherLabel, "PL", i);
        Impress12x20(screenMatrix, PhilosophersCoordinate[i][0] + 5 * constant, PhilosophersCoordinate[i][1] + 7 * constant, philosopherLabel, 0xFFFFFF); // Print ID

       
        FillEllipse(screenMatrix, ChopsticksCoordinate[i][0], ChopsticksCoordinate[i][1], 4 * constant, 4 * constant, 0x00FF00); // Chopsticks (green)
        // Generate chopstick label
        PrintNumbertoScreen(chopstickLabel, "CH", i);
        Impress12x20(screenMatrix, ChopsticksCoordinate[i][0] + 1 * constant, ChopsticksCoordinate[i][1] + 2 * constant, chopstickLabel, 0xFFFFFF); // Print ID

    }

}

// Drawing thread
void DrawThread() {
    screenMatrix = 0; // Clear the screen
    DrawDefaultImages();
    //DrawDiningPhilosophers(screenMatrix); // Draw Dining Philosophers
    while (true) {
        DisplayImage(F1, screenMatrix); // Send drawing to the screen
        Sleep(30); // Refresh rate
    }
}



void *PhilosopherThread(LPVOID vv) {

    int* tmp = (int*)vv;
    int id;
    id = *tmp;

    Philosopher p = Philosopher{ THINKING, 0, id, PhilosophersCoordinate[id][0], PhilosophersCoordinate[id][1] } ;
   

    Sleep(1000); 

    //Philosopher p = { THINKING, 0  };

    p.state = HUNGRY; //HUNGRRY

    while(1){
        switch (p.state)
        {
            case HUNGRY:{
                FillEllipse(screenMatrix, p.x, p.y, 8 * constant, 8 * constant, 0xFF0000); // Philosophers (red)
                PrintNumbertoScreen(philosopherLabel, "PL", p.id);
                Impress12x20(screenMatrix, p.x + 5 * constant, p.y + 7 * constant, philosopherLabel, 0xFFFFFF);
                break;
            }
            case EATING: {
                //FillEllipse(screenMatrix, p.x, p.y, 8 * constant, 8 * constant, 0xFFFF00); // Philosophers (yellow)
                Sleep(1000); //Eating Animation;
                p.state = THINKING;
                break;
            }
            default: {
                //FillEllipse(screenMatrix, p.x, p.y, 8 * constant, 8 * constant, 0x0000FF); // Philosophers (yellow)
                ChopstickCount += 2;
                p.chopstick = 0;
                //return ;
            }
        }
        p.chopstick++;
        ChopstickCount--;
        if (p.chopstick == 2) {
            p.state = HUNGRY;
        }
        Sleep(1000);
    }


}

void StartNonSemaphore(void) {

    //Philosopher philosopher[5];

    int philosopher[5];
    DWORD ThreadID;
    
    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)DrawThread, NULL, 0, NULL);

    Sleep(1000);

    for (int i = 0; i < PHILOSOPHER_LIMIT; i++) {

        //Philosopher  p = { THINKING, 0, id, PhilosophersCoordinate[id][0], PhilosophersCoordinate[id][1] };
        //philosopher[i] = p;

        philosopher[5];

        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)PhilosopherThread, &philosopher[i], 0, &ThreadID);
    }
    
    
 
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

    CalculateCoordinatePhilosophers();
    CalculateCoordinateChopsticks();

};
