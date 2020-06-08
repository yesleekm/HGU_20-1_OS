TARGET = target.c

all:
	gcc $(TARGET) -o target -pthread -g 
	gcc ddchck.c -o ddchck -g
	gcc ddmon.c -shared -fPIC -o ddmon.so -ldl

clean:
	rm target ddchck ddmon.so
