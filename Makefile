CC      = gcc
CFLAGS  = -Wall -Wextra -g -Iinclude

SRCS    = src/main.c \
          src/utils.c \
          src/patient.c \
          src/doctor.c \
          src/appointment.c \
          src/billing.c \
          src/report.c

OBJS    = $(patsubst src/%.c, obj/%.o, $(SRCS))
TARGET  = hospital

.PHONY: all clean

all: obj $(TARGET)

obj:
	if not exist obj mkdir obj

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

obj/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	if exist obj rmdir /s /q obj
	if exist $(TARGET).exe del /q $(TARGET).exe
	if exist $(TARGET) del /q $(TARGET)
