#!/bin/sh
g++ glAnthropometric3DModel.cpp -I /usr/local/include/opencv/ -L /usr/local/lib -lcv -lhighgui -lcvaux -lX11 -lXi -lXmu -lglut -lGL -lGLU -lm -o glAnthropometric3DModel
