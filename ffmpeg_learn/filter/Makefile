src = $(wildcard *.c)
obj = $(src:.c=.o)
dep = $(obj:.o=.d)  # one dependency file for each source

#LDFLAGS =  -L../lib -lavformat -lavfilter -lavcodec -lswresample -lswscale -lavutil   -lx264 -ldl -lm -lpthread -lz
LDFLAGS =  -L../lib -lavfilter -lpostproc -lavformat   -lavcodec -lx264  -lswresample -lswscale -lavutil   -ldl -lm -lpthread -lz
CFLAGS = -std=c99 -Wall -g -O0 -c -I./  -I/usr/include/glib-2.0/ -I../include/

test: $(obj)
	$(CC) -o $@ $^ $(LDFLAGS)

-include $(dep)   # include all dep files in the makefile

# rule to generate a dep file by using the C preprocessor
# (see man cpp for details on the -MM and -MT options)
%.d: %.c
	@$(CPP) $(CFLAGS) $< -MM -MT $(@:.d=.o) >$@

.PHONY: clean
clean:
	rm -f $(obj) $(dep) test



