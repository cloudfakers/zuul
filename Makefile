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
	@echo "Fetching dependencies..."
	@go get -v github.com/tools/godep
	@go get -v github.com/golang/lint/golint
	@dep ensure -v

.PHONY: deps build clean install uninstall
