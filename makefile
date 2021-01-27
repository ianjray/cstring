CC       = clang
SANITIZE = -g -O0 -fsanitize=address -fsanitize=undefined-trap -fsanitize-undefined-trap-on-error
COVERAGE = -fprofile-instr-generate -fcoverage-mapping
OPTS     = $(SANITIZE) $(COVERAGE) -Weverything -Wno-padded -Wno-poison-system-directories

cstring : cstring.c
	$(CC) $(OPTS) -DUNITTEST_CSTRING $^ -o $@

.PHONY : clean
clean :
	rm -rf cstring cstring.dSYM cstring.profdata cstring.profraw

.PHONY : coverage
coverage : cstring.profdata
	xcrun llvm-cov show ./cstring -instr-profile=$<

%.profdata : %.profraw
	xcrun llvm-profdata merge -sparse $< -o $@

%.profraw : %
	LLVM_PROFILE_FILE=$@ ./$<
