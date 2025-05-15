app: app.o scheduler.o
	gcc -g -Wall -Wextra -Werror -o app app.o scheduler.o
app.o: app.c scheduler.h
	gcc -g -Wall -Wextra -Werror -c app.c
scheduler.o: scheduler.c scheduler.h
	gcc -g -Wall -Wextra -Werror -c scheduler.c
clean:
	rm -f app app.o scheduler.o
run: app
	./app