CFLAGS := -g -Wall -Wextra

benchmark: build/main.o
	$(LINK.o) $(OUTPUT_OPTION) $^

build/%.o: src/%.c | build
	$(COMPILE.c) $(OUTPUT_OPTION) $<

build:
	mkdir -p $@

clean:
	rm -r build
.PHONY: clean
