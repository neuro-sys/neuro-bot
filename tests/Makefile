
TESTS	= test_utils test_config test_curl_wrap

all: $(TESTS)
	$(info *** Running unit tests *** )
	@$(foreach var, $(TESTS), ./$(var);)

test_utils: ../utils.o utils_test.o
	$(CC) ../utils.o utils_test.o -o $@ 

test_config: ../config.o config_test.o
	$(CC) ../config.o config_test.o -o $@
	cp ../neurobot.conf .

test_curl_wrap: ../curl_wrap.o curl_wrap_test.o
	$(CC) ../curl_wrap.o curl_wrap_test.c -o $@ $(shell pkg-config libcurl --cflags --libs)

clean:
	rm -fv *.o $(TESTS) neurobot.conf
