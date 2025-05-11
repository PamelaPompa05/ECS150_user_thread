# Target library
lib := libuthread.a

all: $(lib) #libuthread.a is the target

queue.o: queue.c
	gcc -c queue.c -o queue.o

$(lib): queue.o
	ar rcs $(lib) queue.o

clean:
	rm -f queue.o $(lib)

## From what I understand, a target library only wants object files and they're part of the library

#Source: https://tldp.org/HOWTO/Program-Library-HOWTO/static-libraries.html
