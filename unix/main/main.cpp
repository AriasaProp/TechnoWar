#include <X11/Xlib.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <iostream>

int main() {
    Display* display = XOpenDisplay(nullptr);
    if (!display) {
        std::cerr << "Failed to open X display" << std::endl;
        return -1;
    }

    int screen = DefaultScreen(display);
    Window root = RootWindow(display, screen);

    GLint attributes[] = {GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None};
    XVisualInfo* visual = glXChooseVisual(display, screen, attributes);
    if (!visual) {
        std::cerr << "No appropriate visual found" << std::endl;
        return -1;
    }

    Colormap colormap = XCreateColormap(display, root, visual->visual, AllocNone);
    XSetWindowAttributes swa;
    swa.colormap = colormap;
    swa.event_mask = ExposureMask | KeyPressMask;

    Window window = XCreateWindow(
        display, root, 0, 0, 800, 600, 0, visual->depth, InputOutput, visual->visual,
        CWColormap | CWEventMask, &swa);

    XMapWindow(display, window);

    GLXContext glc = glXCreateContext(display, visual, nullptr, GL_TRUE);
    glXMakeCurrent(display, window, glc);

    // Render loop
    XEvent xev;
    do {
        XNextEvent(display, &xev);

        if (xev.type == Expose) {
            glClear(GL_COLOR_BUFFER_BIT);
            glXSwapBuffers(display, window);
        }

    } while (xev.type == KeyPress);

    // Cleanup
    glXMakeCurrent(display, None, nullptr);
    glXDestroyContext(display, glc);
    XDestroyWindow(display, window);
    XCloseDisplay(display);
    return 0;
}