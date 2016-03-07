COMPILER = g++
ARTIFACT = oauth

SOURCE_FILES = main.cpp machine.cpp local_webserver.cpp exit.cpp
OBJECT_FILES = $(SOURCE_FILES:.cpp=.o)

INCLUDE_PATHS = -I/mingw64/include
LIBS_PATHS = -L/mingw64/lib
LINK_WITH = -lcurl -lws2_32
# -lidn -lrtmp -lssh2 -lssl -lcrypto -lssl -lcrypto -lgdi32 -lwldap32 -lz -lws2_32

CFLAGS = 
ARTIFACT_CFLAGS = $(INCLUDE_PATHS) $(LIBS_PATHS) $(LINK_WITH)

all: $(ARTIFACT)

$(ARTIFACT): $(OBJECT_FILES)
	$(COMPILER) $(OBJECT_FILES) $(ARTIFACT_CFLAGS) -o $(ARTIFACT)

%.o: %.cpp
	$(COMPILER) $< $(CFLAGS) -c -o $@ 

clean:
	rm -rf *.o $(ARTIFACT)