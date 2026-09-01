CXX = g++
CXXFLAGS = -std=c++23 -Wall -Wextra -pedantic -Icompiler/include

TARGET_BASE = csam

# GNU make on Windows commonly exposes OS=Windows_NT.
# Keep the same target name on POSIX systems while using the native
# executable suffix and clean command on Windows.
ifeq ($(OS),Windows_NT)
    EXE_SUFFIX = .exe
    RM = del /Q /F
else
    EXE_SUFFIX =
    RM = rm -f
endif

TARGET = $(TARGET_BASE)$(EXE_SUFFIX)

SRC = $(wildcard compiler/src/*.cpp)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

clean:
	$(RM) $(TARGET)
