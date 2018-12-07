src = $(wildcard *.c)
obj = $(src:.c=.o)
bin = test

CFLAGS = -pedantic -Wall -g -I../src
LDFLAGS = ../libanim.a $(libgl) -lm

ifeq ($(shell uname -s), Darwin)
	libgl = -framework OpenGL -framework GLUT
else
	libgl = -lGL -lGLU -lglut
endif

$(bin): $(obj) ../libanim.a
	$(CC) -o $@ $(obj) $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(obj) $(bin)
