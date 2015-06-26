CC = g++
objects = main.cpp scanner.cpp parser.cpp  public.cpp symtab.cpp
parser : $(objects)
	g++ main.cpp scanner.cpp parser.cpp  public.cpp symtab.cpp -std=c++0x -o2 -o parser 2> result/gcc_error
parser.cpp : parser.y
	bison -o parser.cpp parser.y
scanner.cpp : scanner.l
	flex -o scanner.cpp scanner.l
clean:
	rm parser