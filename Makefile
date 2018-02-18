GOLINT := $(GOPATH)/bin/golint
DEP := $(GOPATH)/bin/dep

TARGET := bin
BINARY := zuul


build: vendor $(GOLINT)
	go build -o $(TARGET)/$(BINARY)
	go vet
	golint

install: build
	go install

clean:
	go clean
	rm -rf $(TARGET)

uninstall:
	go clean -i

$(GOLINT):
	go get -v github.com/golang/lint/golint

$(DEP):
	go get -v github.com/golang/dep/cmd/dep

vendor: $(DEP)
	dep ensure -v

linux:
	GOOS=linux GOARCH=amd64 go build -o $(TARGET)/$(BINARY)-linux-amd64

raspberry:
	GOOS=linux GOARCH=arm GOARM=7 go build -o $(TARGET)/$(BINARY)-armv7

.PHONY: build install clean uninstall linux raspberry
