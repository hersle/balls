all:
	cc balls.c -o balls -lm -lglfw -lGL -lGLU

clean:
	rm balls
