CFLAGS = -Wall -Werror

# Define object files based on source files
OBJECTS = client.o

# Executable name
TARGET = client

# Dependency rule (implicit rule): compiles client.c to client.o if client.c is newer
client.o: client.c mosquitto.h
		$(CC) $(CFLAGS) -c client.c -o client.o

# Build rule: links object files to create the executable
all: $(TARGET)

# Link rule: creates the executable (client) from object files
$(TARGET): $(OBJECTS)
		$(CC) $(CFLAGS) -o $(TARGET) $^

# Clean rule: removes the executable and object files
clean:
		rm -f $(TARGET) $(OBJECTS)

# Phony target for usage message
.PHONY: usage

# Usage rule: displays usage instructions
usage:
		@echo ""
		@echo "Usage :"
		@echo "    On the server computer, start the server:"
		@echo "       ./server port"
		@echo "    On the client computer, start the client:"
		@echo "       ./client ipserver portserver"
		@echo ""

# Implicit rule for mosquitto.h (assuming it's in a standard include path)
