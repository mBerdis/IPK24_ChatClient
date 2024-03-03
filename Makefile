CC = g++
CFLAGS = -Wall -std=c++20

SRCDIR = src
OBJDIR = obj

SOURCES := $(wildcard $(SRCDIR)/*.cpp)
OBJECTS := $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)
EXECUTABLE = ipk24chat-client

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.cpp
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm -f $(EXECUTABLE)
	@rm -rf $(OBJDIR)

.PHONY: all clean

runUdp: all
	./ipk24chat-client -t udp -s 172.23.0.1

run: all
	./ipk24chat-client -t tcp -s 127.0.0.1