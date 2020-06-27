COMPILER=gcc
FLAGS=-Wall -g


all: server.out sender.out listener.out simplesender.out

server.out:
	$(COMPILER) $(FLAGS) server.c -o server.out

sender.out:
	$(COMPILER) $(FLAGS) sender.c -o sender.out

simplesender.out:
	$(COMPILER) $(FLAGS) simplesender.c -o simplesender.out

listener.out:
	$(COMPILER) $(FLAGS) receiver.c -o receiver.out

.PHONY: clean run

clean:
	rm -f server.out sender.out receiver.out

test: server.out sender.out listener.out
	./test.sh
