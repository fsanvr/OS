#include "mainwindow.h"

#include <QApplication>

#include <X11/Xlib.h>
#include <X11/keysym.h>

void DisableGlobalShortcuts() {
    Display *display = XOpenDisplay(nullptr);
    if (!display) return;

    Window rootWindow = DefaultRootWindow(display);

    KeyCode tab = XKeysymToKeycode(display, XK_Tab);
    KeyCode altKeyLeft = XKeysymToKeycode(display, XK_Alt_L);
    KeyCode altKeyRight = XKeysymToKeycode(display, XK_Alt_R);

    XGrabKey(display, tab, Mod1Mask, rootWindow, True, GrabModeAsync, GrabModeAsync);
    XGrabKey(display, altKeyLeft, Mod1Mask, rootWindow, True, GrabModeAsync, GrabModeAsync);
    XGrabKey(display, altKeyRight, Mod1Mask, rootWindow, True, GrabModeAsync, GrabModeAsync);

    XCloseDisplay(display);
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    DisableGlobalShortcuts();

    MainWindow w;
    w.showFullScreen();

    return a.exec();
}
