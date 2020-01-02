CC = gcc


#生成.d文件也需要这个目录，所以单独设置一个变量
FF_HFILE_PATH=-I../../include/

#-m64 表示编译目标未64位 -g 增加调试信息
CFLAGS =  -Wall  -m64 -g  -std=c99 -Wall -g -O0 -c -I./  -I../util $(FF_HFILE_PATH)
LDFLAGS = -m64   -L../lib -l:libfaad.a -lm -lpthread

 

TARGET = test
src = $(wildcard *.c )   

obj = $(src:.c=.o)
dep = $(obj:.o=.d)  # one dependency file for each source


$(TARGET): $(obj)
	gcc -o $@ $^ $(LDFLAGS)

-include $(dep)   # include all dep files in the makefile

# rule to generate a dep file by using the C preprocessor
# (see man cpp for details on the -MM and -MT options)
%.d: %.c
	gcc $(make) $< -MM -MT $(@:.d=.o) >$@ $(FF_HFILE_PATH) -I../util
	

.PHONY: clean
clean:
	-rm  $(obj) 
	-rm  $(dep) 
	-rm  $(TARGET)