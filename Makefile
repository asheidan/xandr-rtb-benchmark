CFLAGS := -g -Wall -Wextra

IMPLEMENTATIONS = async_protocol

TARGET := benchmark

DATASET_CONNECTIONS = 1 10 128 256 512 1024 2048 3072 4096 5120 6144 7168 8192
default: $(TARGET)

datafiles: $(patsubst %,data/%/data.txt,$(IMPLEMENTATIONS))

$(TARGET): build/main.o
	$(LINK.o) $(OUTPUT_OPTION) $^

build/%.o: src/%.c | build
	$(COMPILE.c) $(OUTPUT_OPTION) $<

build:
	mkdir -p $@

# We might need to remove the pattern expansion here for the dependencies
# (adding .py) if we have implementations in multiple languages.
data/%/data.txt: implementations/%.py $(TARGET) testbench.sh Makefile
	test -e $(@D) || mkdir -p $(@D)
	./testbench.sh $(TARGET) $< $(DATASET_CONNECTIONS) > $@

clean:
	rm -r build
.PHONY: clean
