CFLAGS  = -g -O0
TESTS	= test_utils test_config test_curl_wrap

all: $(TESTS)
	$(info *** Running unit tests *** )
	@$(foreach var, $(TESTS), ./$(var);)

test_utils: ../modules/utils.o utils_test.o
	$(CC) ../modules/utils.o utils_test.o -o $@ $(CFLAGS)

test_config: ../config.o config_test.o
	$(CC) ../config.o config_test.o -o $@ $(CFLAGS)
	cp ../neurobot.conf .

test_curl_wrap: ../modules/curl_wrap.o curl_wrap_test.o
	$(CC) $(CFLAGS) ../modules/curl_wrap.o curl_wrap_test.c -o $@ $(shell pkg-config libcurl --cflags --libs) 

clean:
	rm -fv *.o $(TESTS) neurobot.conf
