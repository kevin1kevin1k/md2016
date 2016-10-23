.PHONY: all clean run

CFLAGS += -O2
LDFLAGS += -lm

TARGET = train

all: $(TARGET)

clean:
	$(RM) $(TARGET)

run: $(TARGET)
	./$(TARGET) test1 &
	./$(TARGET) test2 &
