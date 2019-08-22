# MAINSOURCE代表含有main入口函数的cpp文件，因为含有测试代码，
# 所以要为多个目标编译，这里把Makefile写的通用了一点，
# 以后加东西Makefile不用做多少改动
MAINSOURCE := Main.cpp tests/HTTPClient.cpp
# MAINOBJS := $(patsubst %.cpp,%.o,$(MAINSOURCE))
SOURCE  := $(wildcard net/*.cpp base/*.cpp tests/*.cpp)
override SOURCE := $(filter-out $(MAINSOURCE),$(SOURCE))
OBJS    := $(patsubst %.cpp,%.o,$(SOURCE))

TARGET  := SoL
CC      := g++
LIBS    := -lpthread
INCLUDE:= -I./usr/local/lib
CFLAGS  := -std=c++11 -g -Wall -O3 -D_PTHREADS
CXXFLAGS:= $(CFLAGS)

# Test object
SUBTARGET1 := HTTPClient

.PHONY : objs clean veryclean rebuild all tests debug
all : $(TARGET) $(SUBTARGET1)
objs : $(OBJS)
rebuild: veryclean all

tests : $(SUBTARGET1)
clean :
	find . -name '*.o' | xargs rm -f
veryclean :
	find . -name '*.o' | xargs rm -f
	find . -name $(TARGET) | xargs rm -f
	find . -name $(SUBTARGET1) | xargs rm -f
debug:
	@echo $(SOURCE)

$(TARGET) : $(OBJS) Main.o
	$(CC) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)
# $@代表目标，这里是$(TARGET)

$(SUBTARGET1) : $(OBJS) tests/HTTPClient.o
	$(CC) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)
