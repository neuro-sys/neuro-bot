CFLAGS  = -g -O0
TESTS	= test_utils test_config test_curl_wrap test_irc_parser

all: $(TESTS)
	$(info *** Running unit tests *** )
	@$(foreach var, $(TESTS), ./$(var);)

curl_wrap.o: ../modules/curl_wrap.c
	$(CC) -c $< -o $@

utils.o: ../modules/utils.c
	$(CC) -c $< -o $@

test_utils: utils.o utils_test.o
	$(CC) utils.o utils_test.o -o $@ $(CFLAGS)

test_config: ../config.o config_test.o
	$(CC) ../config.o config_test.o -o $@ $(CFLAGS)
	cp ../neurobot.conf .

test_curl_wrap: curl_wrap.o curl_wrap_test.o
	$(CC) $(CFLAGS) curl_wrap.o curl_wrap_test.c -o $@ $(shell pkg-config libcurl --cflags --libs) 

irc_parser.o: ../irc_parser.c
	$(CC) -c $< -o $@

test_irc_parser: irc_parser.o irc_parser_test.o
	$(CC) $(CFLAGS) irc_parser.o irc_parser_test.c -o $@

clean:
	rm -fv *.o $(TESTS) neurobot.conf
