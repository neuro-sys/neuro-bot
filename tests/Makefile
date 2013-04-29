
test_utils_run:
	$(info *** Running unit tests *** )
	./test_utils

test_utils: ../utils.o utils_test.o
	$(CC) ../utils.o utils_test.o -o $@ 
