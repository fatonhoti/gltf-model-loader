# Compiler and flags (missing -Werror)
CXX = g++
CXXFLAGS = -O3 -g -std=c++20 -Wall -pedantic -I ./include
LDFLAGS = -lglfw -lGL

# source files
SRC_DIR = ./src
SRCS = $(wildcard $(SRC_DIR)/*.cpp) $(wildcard $(SRC_DIR)/*.c)

# object files
OBJS = $(SRCS:.cpp=.o)

# executable
TARGET = main.out

all: $(TARGET)

# link objs
$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET)  $(LDFLAGS)

# compile source files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.c
	$(CXX) $(CXXFLAGS) -c $< -o $@

# clean up after yourself!
clean:
	rm -f $(OBJS) $(TARGET)

# Run the executable
run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run