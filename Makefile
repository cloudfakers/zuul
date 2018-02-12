BIN=zuul

all: build

build:
	@echo "Building Zuul..."
	@go build -o $(BIN)

	@echo "Running static code analysis..."
	@go vet

	@echo "Running Go style checks..."
	@golint

install:
	@echo "Installing to $(GOPATH)/bin/$(BIN)..."
	@go install

clean:
	@echo "Cleaning local builds..."
	@go clean

uninstall:
	@echo "Uninstalling from $(GOPATH)/bin..."
	@go clean -i -v

deps:
	go get github.com/gorilla/mux
	go get github.com/stianeikeland/go-rpio
	go get github.com/hajimehoshi/oto
	go get github.com/hajimehoshi/go-mp3

.PHONY: deps build
