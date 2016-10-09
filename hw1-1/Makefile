T = hw1

all: $(T)

%: %.cpp
	@g++ -O2 -Wall $< -o $@

run: $(T)
	@./$T

rm:
	@rm -f $(T)