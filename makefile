
SRCS := $(wildcard *.cpp)
OBJS := $(patsubst %.cpp, %.o, $(SRCS))

BIN_PATH:=./

TARGET := $(BIN_PATH)/libfatwriter.so

all: $(TARGET)

$(TARGET): $(OBJS)
	g++ -std=c++0x -Wall -g -pthread -shared -o $@ $^
	@echo Finished building target: $@
	

%.o: %.cpp
	g++ -std=c++0x -Wall -g -pthread -c -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o $@ $^
	

clean:
	-rm -f *.o
	-rm -f *.d
