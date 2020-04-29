compile:
	mpicc -cc=gcc-6 prim.c parser.c -o proj
run1: compile
	mpiexec -f nodes -n 20 ./proj input.txt
run2: compile
	mpiexec -f nodes -n 20 ./proj input2.txt
run3: compile
	mpiexec -f nodes -n 20 ./proj input3.txt
clean:
	rm -rf proj
