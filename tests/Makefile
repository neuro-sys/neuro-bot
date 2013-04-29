
TESTS	= test_utils

test_utils_run: test_utils
	$(info *** Running unit tests *** )
	./test_utils

test_utils: ../utils.o utils_test.o
	$(CC) ../utils.o utils_test.o -o $@ 

clean:
	rm -fv *.o $(TESTS)
