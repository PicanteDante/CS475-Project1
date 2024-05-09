all: main

CXX = clang++
override CXXFLAGS += -g -Wmost -Werror

SRCS = $(shell find . -name '.ccls-cache' -type d -prune -o -type f -name '*.cpp' -print | sed -e 's/ /\\ /g')
HEADERS = $(shell find . -name '.ccls-cache' -type d -prune -o -type f -name '*.h' -print)

main: $(SRCS) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o "$@"

main-debug: $(SRCS) $(HEADERS)
	$(CXX) $(CXXFLAGS) -U_FORTIFY_SOURCE -O0 $(SRCS) -o "$@"

proj01:     	main.cpp
	g++     main.cpp   -o proj01  -lm  -fopenmp

proj03:		main.cpp
	g++  main.cpp    -o  proj03    -lm  -fopenmp

proj03-2:		main2.cpp
	g++  main2.cpp    -o  proj03-2    -lm  -fopenmp

clean:
	rm -f main main-debug