.PHONY: all clean run

CFLAGS += -O2

TARGET = train

all: $(TARGET)

clean:
	$(RM) $(TARGET)

run: $(TARGET)
	./$(TARGET) valid
