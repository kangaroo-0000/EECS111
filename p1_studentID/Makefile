EXEC=p1_exec
CC=g++
CFLAGS=-std=c++98 -I.

${EXEC}: main.o p1_process.o p1_threads.o
	g++ -o ${EXEC} main.o p1_process.o p1_threads.o -I. -lpthread 

%.o: %.cpp
	$(CC) $(CFLAGS) -o $@ -c $<

.PHONY: test
test: ${EXEC}
	python3 autograder.py

.PHONY: clean
clean:
	rm -rf ./${EXEC}
	rm -rf ./*.o
	find ./output -type f ! -name "desktop.ini" -delete
