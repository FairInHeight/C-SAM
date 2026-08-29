CXX = g++
CXXFLAGS = -std=c++23 -Wall -Wextra -pedantic -Icompiler/include

TARGET = csam

SRC = $(wildcard compiler/src/*.cpp)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)