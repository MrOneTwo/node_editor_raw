---
tags: ["UI", "UX", "opengl"]
---

# Raw Node editor

Node editor UX exploration. By *raw* I mean that it is basically an OpenGL app written from scratch. Nuklear GUI lib is used for statistics, debug, etc.

![](thumb.jpg)


## Current state

17-07-2019 - working on a better architecture where platform layer is separate from the program itself.
At the moment I'm working on implementing the basic IO functions the way Handmade Hero does it.

## Dependencies

- SDL2
- glew
- cglm
- dirent_win

## Init project

### GLEW

Go into `auto` folder and do `make`. After that solution from `glew\build\vc15`
should build fine.
