
.PHONY: barrier_test
barrier_test:
	gcc -O0 -g -Wall barrier.c barrier_test.c -o barrier_test -lpthread
clean:
	rm -f barrier_test *_out.txt
	rm -rf student_handout

