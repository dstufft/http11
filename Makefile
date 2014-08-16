all:
	@echo ""

clean:
	rm -rf src/http11/c/__pycache__
	rm -f src/http11/*_cffi_*.so

ragel:
	ragel -C -G2 src/http11/c/http11.rl
