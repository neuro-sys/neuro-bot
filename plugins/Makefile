CFLAGS = -Wall -g -O0

ifneq ($(OS), Windows_NT)
	CFLAGS		 	+= -fPIC
endif

all: plugin_python_manager.so plugin_git.so plugin_title.so plugin_wiki.so plugin_youtube.so \
	plugin_test.so plugin_mesai.so plugin_java_manager.so

plugin_python_manager.so: plugin_python_manager.c
	$(CC) plugin_python_manager.c $(CFLAGS) --shared -o$@

plugin_git.so: plugin_git.c
	$(CC) plugin_git.c utils/curl_wrapper.c utils/json.c $(CFLAGS) `pkg-config libcurl --cflags --libs` --shared -o$@

plugin_title.so: plugin_title.c
	$(CC) plugin_title.c utils/curl_wrapper.c $(CFLAGS) `pkg-config libcurl libxml-2.0 --cflags --libs` --shared -o$@

plugin_wiki.so: plugin_wiki.c
	$(CC) plugin_wiki.c utils/curl_wrapper.c utils/json.c $(CFLAGS) `pkg-config libcurl --cflags --libs` --shared -o$@

plugin_youtube.so: plugin_youtube.c
	$(CC) plugin_youtube.c utils/curl_wrapper.c utils/json.c $(CFLAGS) `pkg-config libcurl --cflags --libs` --shared -o$@

plugin_test.so: plugin_test.c
	$(CC) plugin_test.c $(CFLAGS) --shared -o$@

plugin_mesai.so: plugin_mesai.c
	$(CC) plugin_mesai.c $(CFLAGS) --shared -o$@

GCJ_LD_PATH="/usr/lib/gcj-*/"
plugin_java_manager.so: plugin_java_manager.c
ifeq ("$(wildcard $(GCJ_LD_PATH))", "")
	$(CC) plugin_java_manager.c -L$(shell ls -d /usr/lib/gcj-*/) -ljvm --shared -o$@
else
	@echo 'Java plugin cannot be built, skipping. GCJ libraries are needed.'
endif

clean:
	rm -fv *.o *.so

