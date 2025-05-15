# Target library
lib := libuthread.a
objs := queue.o context.o uthread.o sem.o preempt.o
CCFLAGS := -Wall -Wextra -Werror -MMD

all: $(lib) #libuthread.a is the target

%.o: %.c
	gcc $(CCFLAGS) -c $< -o $@

$(lib): $(objs)
	ar rcs $(lib) $(objs)

clean:
	rm -f $(objs) $(lib) *.d *.x

## From what I understand, a target library only wants object files and they're part of the library

#Source: https://tldp.org/HOWTO/Program-Library-HOWTO/static-libraries.html
