CFLAGS := -g -Wall -Wextra

TARGET := benchmark

DATASET_CONNECTIONS = 1 10 1024 2048 3072 4096 5120 6144 7168 8192

$(TARGET): build/main.o
	$(LINK.o) $(OUTPUT_OPTION) $^

build/%.o: src/%.c | build
	$(COMPILE.c) $(OUTPUT_OPTION) $<

build:
	mkdir -p $@

data/%/data.txt: implementations/%.py $(TARGET) testbench.sh
	test -e $(@D) || mkdir -p $(@D)
	./testbench.sh $(TARGET) $< $(DATASET_CONNECTIONS)

clean:
	rm -r build
.PHONY: clean
