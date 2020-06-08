compile:
	UPCXX_GASNET_CONDUIT=udp upcxx -O prim.cpp parser.cpp -o program
run1: compile
	upcxx-run -n 4 -ssh-servers stud201,stud202,stud203,stud204,stud205,stud206,stud207,stud208,stud209,stud210,stud211,stud212,stud213,stud214,stud215,stud216 program input.txt
run2: compile
	upcxx-run -n 8 -ssh-servers stud201,stud202,stud203,stud204,stud205,stud206,stud207,stud208,stud209,stud210,stud211,stud212,stud213,stud214,stud215,stud216 program input2.txt
run3: compile
	upcxx-run -n 4 -ssh-servers stud201,stud202,stud203,stud204,stud205,stud206,stud207,stud208,stud209,stud210,stud211,stud212,stud213,stud214,stud215,stud216 program input3.txt
run4: compile
	upcxx-run -n 16 -ssh-servers stud201,stud202,stud203,stud204,stud205,stud206,stud207,stud208,stud209,stud210,stud211,stud212,stud213,stud214,stud215,stud216 program large.txt
clean:
	rm -rf program