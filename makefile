compile:
	mpicc -cc=gcc-6 prim.c parser.c -o proj
run1: compile
	mpiexec -f nodes -n 5 ./proj samples/input.txt
run2: compile
	mpiexec -f nodes -n 5 ./proj samples/input2.txt
run3: compile
	mpiexec -f nodes -n 5 ./proj samples/input3.txt
clean:
	rm -rf proj
