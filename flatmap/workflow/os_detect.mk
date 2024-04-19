# Detect OS (https://stackoverflow.com/a/14777895)
ifeq ($(OS),Windows_NT)
	DETECTED_OS := Windows
else
	DETECTED_OS := $(shell (uname || echo Unknown) 2>/dev/null)
endif
