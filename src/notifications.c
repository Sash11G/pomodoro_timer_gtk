#include <windows.h>
#include <stdio.h>

// Define a structure to store window handles
typedef struct {
    HWND windows[256];  // Array to store window handles
    int count;          // Count of windows
} WindowData;

// Callback function that will be called for each open window
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    char windowTitle[256];

    // Check if the window is a top-level window and visible
    if (IsWindowVisible(hwnd) && GetWindow(hwnd, GW_OWNER) == NULL) {
        // Get the window title
        if (GetWindowTextA(hwnd, windowTitle, sizeof(windowTitle))) {
            // Store the handle of the window
            WindowData* data = (WindowData*)lParam;
            data->windows[data->count++] = hwnd;

            // Print windows with titles
            if (strlen(windowTitle) > 0) {
                printf("Main Application Window with Title: %s\n", windowTitle);
            } else {
                printf("Main Application Window with No Title (HWND: %p)\n", hwnd);
            }
        }
    }
    return TRUE;  // Continue enumeration
}

// Function to disable all top-level windows
void disable_windows(WindowData* data) {
    // Enumerate all top-level windows and store their handles
    EnumWindows(EnumWindowsProc, (LPARAM)data);

    // Disable all found windows to prevent interaction
    for (int i = 0; i < data->count; i++) {
        EnableWindow(data->windows[i], FALSE);
    }

    // Disable the taskbar
    HWND taskBar = FindWindow("Shell_TrayWnd", NULL);
    if (taskBar != NULL) {
        EnableWindow(taskBar, FALSE);
    }
}

// Function to re-enable all top-level windows
void enable_windows(WindowData* data) {
    // Re-enable all windows after the message box is closed
    for (int i = 0; i < data->count; i++) {
        EnableWindow(data->windows[i], TRUE);
    }

    // Re-enable the taskbar
    HWND taskBar = FindWindow("Shell_TrayWnd", NULL);
    if (taskBar != NULL) {
        EnableWindow(taskBar, TRUE);
    }
}

// Function implementation for notifying the user
void notify_user(const char* message) {
    WindowData data = { .count = 0 };

    // Disable windows and taskbar
    disable_windows(&data);

    // Show the message box that stays on top and blocks interaction
    MessageBox(NULL, message, "System Lockdown", MB_OK | MB_ICONINFORMATION | MB_TOPMOST | MB_SYSTEMMODAL);

    // Re-enable windows and taskbar after the message box is closed
    enable_windows(&data);
}
