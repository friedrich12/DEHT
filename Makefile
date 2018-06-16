INC =-I/home/professor/Documents/DEHT/include

default:    main

main:   test.cpp

	g++ $(INC) *.cpp -std=c++17 -o deht

clean:
	rm -r deht