all: sample2D

sample2D: Sample_GL3_2D.cpp glad.c
	g++ -o sample2D Sample_GL3_2D.cpp glad.c -lGL -lglfw -lftgl -lSOIL -lGLEW -ldl -I/usr/local/include -I/usr/include/freetype2 -L/usr/local/lib

clean:
	rm sample2D
