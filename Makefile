all:
	@echo ""

clean:
	rm -rf http11/__pycache__
	rm -f http11/*_cffi_*.so

ragel:
	ragel -C -G2 http11/c/http11.rl
