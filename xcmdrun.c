/*
  ISC License
  Copyright 2022 Nathan Phillips.
  Permission to use, copy, modify, and/or distribute this software for any 
  purpose with or without fee is hereby granted, provided that the above 
  copyright notice and this permission notice appear in all copies.
  
  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES 
  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF 
  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR 
  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES 
  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN 
  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF 
  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define INPUT_BUFFER_LENGTH 1024
#define BUFFER_LENGTH 128

void stop(Display *display, Window window, GC gc) {
  XFreeGC(display, gc);
  XDestroyWindow(display, window);
  XCloseDisplay(display);
  exit(0);
}

void draw(
  Display *display, 
  Window window, 
  GC gc, 
  char *inputBuffer
) {
  XClearWindow(display, window);
  XDrawString(
    display,
    window,
    gc,
    5,
    15,
    inputBuffer,
    strlen(inputBuffer)
  );
}

int main() {
  Display *display = XOpenDisplay(NULL);
  if(display == NULL) {
    fprintf(stderr, "Can't open display.\n");
    exit(1);
  }

  int screen = DefaultScreen(display);
  Window window = XCreateSimpleWindow(
    display,
    RootWindow(display, screen),
    10,
    10,
    660,
    100,
    1,
    BlackPixel(display, screen),
    WhitePixel(display, screen)
  );

  XStoreName(display, window, "xcmdrun");
  GC gc = XCreateGC(display, window, 0, 0);
  XSelectInput(display, window, ExposureMask | KeyPressMask);
  XMapWindow(display, window);
  Atom WM_DELETE_WINDOW = XInternAtom(display, "WM_DELETE_WINDOW", False);
  XSetWMProtocols(display, window, &WM_DELETE_WINDOW, 1);
  XEvent event;
  char inputBuffer[INPUT_BUFFER_LENGTH] = { '\0' };
  for(;;) {
    XNextEvent(display, &event);
    char buffer[BUFFER_LENGTH] = { '\0' };
    switch(event.type) {
      case Expose:
        draw(display, window, gc, inputBuffer);
        break;

      case ClientMessage:
        if((unsigned int)(event.xclient.data.l[0]) == WM_DELETE_WINDOW) {
          stop(display, window, gc);
        }
        
        break;

      default:
        break;
    }

    if(event.type == KeyPress) {
      KeySym keysym;
      XLookupString(
        &event.xkey,
        buffer,
        sizeof(buffer),
        &keysym,
        NULL
      );

      switch(keysym) {
        case XK_Escape:
          stop(display, window, gc);
          break;

        case XK_Return:
          system(inputBuffer);
          stop(display, window, gc);
          break;

        case XK_BackSpace:
          if(strlen(inputBuffer) <= 0) {
              break;
          }
          
          inputBuffer[strlen(inputBuffer) - 1] = '\0';
          draw(display, window, gc, inputBuffer);
          break;
        
        default:
          if(
            strlen(inputBuffer) + 
            strlen(buffer) >= 
            INPUT_BUFFER_LENGTH &&
            strlen(buffer) <= 1
          ) {
            break;
          }
          
          strcat(inputBuffer, buffer);
          draw(display, window, gc, inputBuffer);
          break;
      }
    }
  }

  stop(display, window, gc);
}
