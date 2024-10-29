all: process_memory psum

process_memory : process_memory.c
	gcc -o process_memory process_memory.c
	clear

psum : parallel_sum.o array_random.o
	gcc parallel_sum.o array_random.o -lsum -Lstatic -pthread -o psum

parallel_sum.o : parallel_sum.c
	gcc -o parallel_sum.o parallel_sum.c -c

array_random.o : array_random.c
	gcc array_random.c -o array_random.o -c

clean :
	rm process_memory parallel_sum.o array_random.o psum
