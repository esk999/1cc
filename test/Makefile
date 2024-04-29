FLAGS=-std=c11 -S -fverbose-asm -masm=intel
all: function_call/callee.s
function_call/callee.s: function_call/callee.c
	$(CC) $(@:.s=.c) -S -O0 -o $@ $(CFLAGS)

clean:
	rm -f */*.s