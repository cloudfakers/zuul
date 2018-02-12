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
	"io"
	"log"
	"net/http"
	"os"
	"os/exec"
	"os/signal"
	"syscall"
	"time"

	"github.com/gorilla/mux"
	mp3 "github.com/hajimehoshi/go-mp3"
	"github.com/hajimehoshi/oto"
	rpio "github.com/stianeikeland/go-rpio"
)

// DoorPin is the BCM number for the GPIO pin connected to the door
const DoorPin = 3

// WelcomeFile is the file to play when the welcome command is invoked
var WelcomeFile string

// Port is the port where the Zuul webserver listens
var Port int

// ConfigureRoutes sets up the routes of the Zuul API
func ConfigureRoutes() *mux.Router {
	router := mux.NewRouter()
	router.HandleFunc("/puerta", Puerta)
	router.HandleFunc("/palante", PlayWelcomeFile)
	router.HandleFunc("/dimelo", Say).Methods("POST")
	return router
}

// Puerta handles the requests to open the door and configures the GPIO pin accordingly
func Puerta(w http.ResponseWriter, r *http.Request) {
	pin := rpio.Pin(DoorPin)
	pin.Low()
	time.Sleep(500 * time.Millisecond)
	pin.High()
}

// PlayWelcomeFile plays the welcome file in the speakers connected to the Raspberry Pi
func PlayWelcomeFile(w http.ResponseWriter, r *http.Request) {
	f, err := os.Open(WelcomeFile)
	if err != nil {
		http.Error(w, err.Error(), 500)
	}
	defer f.Close()

	d, err := mp3.NewDecoder(f)
	if err != nil {
		http.Error(w, err.Error(), 500)
	}
	defer d.Close()

	p, err := oto.NewPlayer(d.SampleRate(), 2, 2, 8192)
	if err != nil {
		http.Error(w, err.Error(), 500)
	}
	defer p.Close()

	if _, err := io.Copy(p, d); err != nil {
		http.Error(w, err.Error(), 500)
	}
}

// Say plays the given audio text using the configured text to speech tool
func Say(w http.ResponseWriter, r *http.Request) {
	text := r.PostFormValue("text")
	cmd := exec.Command("espeak", "-ves+f4", "-s150", fmt.Sprintf("\"%v\"", text))
	if err := cmd.Run(); err != nil {
		http.Error(w, err.Error(), 500)
	}
}

func main() {
	flag.IntVar(&Port, "port", 7777, "Listen portt")
	flag.StringVar(&WelcomeFile, "welcome-audio-file", "audio/palante.mp3", "Welcome Audio file")
	flag.Parse()

	if _, err := os.Stat(WelcomeFile); err != nil {
		fmt.Printf("Could not open welcome audio file: %v\n", err)
		os.Exit(1)
	}

	// Initialize Raspberry GPIO
	if err := rpio.Open(); err != nil {
		fmt.Printf("Error initializing GPIO system: %v\n", err)
		os.Exit(1)
	}
	defer rpio.Close()

	address := fmt.Sprintf("0.0.0.0:%v", Port)
	router := ConfigureRoutes()
	server := &http.Server{Addr: address, Handler: router}

	log.Printf("Starting Zuul server at %v...\n", address)
	go func() { log.Fatal(server.ListenAndServe()) }()
	signalChan := make(chan os.Signal, 1)
	signal.Notify(signalChan, syscall.SIGINT, syscall.SIGTERM)
	<-signalChan

	log.Printf("Shutdown signal received. Shutting down gracefully...")
	server.Shutdown(context.Background())
}
