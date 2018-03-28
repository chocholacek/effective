CXX = g++-7
DEBUG = -g
CXXFLAGS = -std=c++1z -Wall -Wextra -pedantic -pthread -g -O2 -Ibricks $(DEBUG)

BRICKS != ls bricks/brick-*
HT != ls hashtable_sources/*
MTX != ls matrix_sources/*
CST != ls charset_sources/*

hash_table: $(HT)
	$(CXX) $(CXXFLAGS) -o hash_table hashtable_sources/main.cpp

matrix: $(MTX)
	$(CXX) $(CXXFLAGS) -o matrix matrix_sources/main.cpp

charset: $(CST)
	$(CXX) $(CXXFLAGS) -o charset charset_sources/main.cpp

all:
	(make hash_table && ./hash_table | gnuplot > hash_table.pdf)
	(make matrix && ./matrix | gnuplot > matrix_multiplication.pdf)
	(make charset && ./charset | gnuplot > charset.pdf)
	(rm hash_table && rm matrix && rm charset)

clear:
	rm *.pdf *.log callgrind.out.*

clean:
	for F in `ls hw*`; do if ! [ -d $F ] && [ -x $F ]; then rm $F; fi; done

