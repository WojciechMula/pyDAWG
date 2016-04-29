.SUFFIXES:
.PHONY: all build

test: build
	python3 unittests.py

build:
	python3 setup.py build_ext --inplace

clean:
	rm -f *.so
