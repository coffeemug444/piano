LIBS_ = sfml-graphics sfml-audio sfml-window sfml-system
LIBS = $(patsubst %,-l%,$(LIBS_))

main: main.cpp
	g++ -g -o $@ $^ $(LIBS)

.PHONY: clean
clean:
	rm -f main