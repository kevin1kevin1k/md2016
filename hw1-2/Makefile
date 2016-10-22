.PHONY: all clean run

CFLAGS += -O2

TARGET = train test

all: $(TARGET)

clean:
	$(RM) $(TARGET)

run: $(TARGET)
	./$(TARGET) valid
