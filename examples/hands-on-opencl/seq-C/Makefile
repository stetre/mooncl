
default: build

build:
	$(CC) -o _matmul -lm matmul.c matrix_lib.c wtime.c
	$(CC) -o _pi -lm pi.c wtime.c

clean:
	@-rm -f _*
