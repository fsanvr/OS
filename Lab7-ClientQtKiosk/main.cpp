#include "mainwindow.h"

#include <QApplication>

#include <X11/Xlib.h>
#include <X11/keysym.h>

void DisableGlobalShortcuts() {
    Display *display = XOpenDisplay(nullptr);
    if (!display) return;

    Window root = DefaultRootWindow(display);

    KeyCode keyAltTab = XKeysymToKeycode(display, XK_Tab);
    KeyCode keyCtrlAltT = XKeysymToKeycode(display, XK_T);

    XGrabKey(display, keyAltTab, Mod1Mask, root, True, GrabModeAsync, GrabModeAsync);
    XGrabKey(display, keyCtrlAltT, ControlMask | Mod1Mask, root, True, GrabModeAsync, GrabModeAsync);

    XCloseDisplay(display);
}

void DisableAltTab() {
    Display* display = XOpenDisplay(nullptr);
    if (!display) return;

    Window rootWindow = DefaultRootWindow(display);

    KeyCode altTab = XKeysymToKeycode(display, XK_Tab);
    XGrabKey(display, altTab, Mod1Mask, rootWindow, True, GrabModeAsync, GrabModeAsync);

    XCloseDisplay(display);
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    DisableGlobalShortcuts();
    DisableAltTab();

    MainWindow w;
    w.show();
    return a.exec();
}
