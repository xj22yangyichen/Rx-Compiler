.DEFAULT_GOAL := serve

MD_BOOK ?= $(HOME)/.cargo/bin/mdbook
HOST ?= 127.0.0.1
PORT ?= 3000

.PHONY: build serve clean

build:
	SPEC_RELATIVE=0 SPEC_DENY_WARNINGS=1 $(MD_BOOK) build

serve:
	SPEC_RELATIVE=0 $(MD_BOOK) serve --hostname $(HOST) --port $(PORT) --open

clean:
	rm -rf book
