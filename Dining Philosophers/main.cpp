#include"icb_gui.h"
ICBYTES screenMatrix;

int F1;

void ICGUI_Create() {
    ICG_MWTitle("Dining Philosophers");
    ICG_MWSize(540, 600);

}

// Drawing Thread
void DrawThread() {
    while (true) {
        screenMatrix = 0;
        //draw
        
        DisplayImage(F1, screenMatrix);
        Sleep(30);
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
}