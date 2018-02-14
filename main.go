/*
Copyright (C) 2017 Ignasi Barrera

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

package main

import (
	"context"
	"flag"
	"fmt"
	"log"
	"net/http"
	"os"
	"os/signal"
	"syscall"

	rpio "github.com/stianeikeland/go-rpio"
)

func main() {
	var port int
	zuul := &Zuul{PinNumber: 3}

	flag.IntVar(&port, "port", 7777, "Listen port")
	flag.StringVar(&zuul.WelcomeFile, "welcome-audio-file", "audio/palante.mp3", "Welcome Audio file")
	flag.Parse()

	router, err := zuul.Init()
	if err != nil {
		log.Fatal(err)
	}
	defer rpio.Close()

	address := fmt.Sprintf("0.0.0.0:%v", port)
	server := &http.Server{Addr: address, Handler: router}

	log.Printf("Starting Zuul server at %v...\n", address)
	go func() { log.Fatal(server.ListenAndServe()) }()

	signalChan := make(chan os.Signal, 1)
	signal.Notify(signalChan, syscall.SIGINT, syscall.SIGTERM)
	<-signalChan

	log.Printf("Shutdown signal received. Shutting down gracefully...")
	server.Shutdown(context.Background())
}
