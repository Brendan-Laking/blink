CC     = clang
LINK   = -ltinfo -lcurl -lncurses -lcjson
WARN   = -Wall -Wextra
FILES  = $(wildcard src/**.c)
TARGET = build/$(shell basename $(shell pwd))
OPT    ?= 0

.PHONY: $(TARGET)

$(TARGET): $(FILES)
	mkdir -p build 
	$(CC) -O$(OPT) $(LINK) $(WARN) -o $(TARGET) $(FILES) 

run: $(TARGET)
	./$(TARGET)
