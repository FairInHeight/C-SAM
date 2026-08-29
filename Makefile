CXX = g++
CXXFLAGS = -std=c++23 -Wall -Wextra -pedantic

TARGET = csam

SRC = compiler/src/main.cpp

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)