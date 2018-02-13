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
	"fmt"
	"io"
	"log"
	"net/http"
	"os"
	"os/exec"
	"time"

	"github.com/gorilla/mux"
	mp3 "github.com/hajimehoshi/go-mp3"
	"github.com/hajimehoshi/oto"
	rpio "github.com/stianeikeland/go-rpio"
)

// Zuul is the Abiquo Gatekeeper that will take care of opening the door and welcoming
// all the people that comes into the office
type Zuul struct {
	WelcomeFile string
	DoorPin     int
}

// Init initializes the Zuul API and configures the HTTP routes for its commands
func (z *Zuul) Init() (*mux.Router, error) {
	if _, err := os.Stat(z.WelcomeFile); err != nil {
		return nil, fmt.Errorf("could not open welcome audio file: %v", err)
	}
	// if err := rpio.Open(); err != nil {
	// 	return nil, fmt.Errorf("error initializing GPIO system: %v", err)
	// }

	router := mux.NewRouter()
	router.HandleFunc("/puerta", z.Puerta)
	router.HandleFunc("/palante", z.PlayWelcomeFile)
	router.HandleFunc("/dimelo", z.Say).Methods("POST")
	return router, nil
}

// Puerta handles the requests to open the door and configures the GPIO pin accordingly
func (z *Zuul) Puerta(w http.ResponseWriter, r *http.Request) {
	log.Println("Received door open request")
	pin := rpio.Pin(z.DoorPin)
	pin.Low()
	time.Sleep(500 * time.Millisecond)
	pin.High()
}

// PlayWelcomeFile plays the welcome file in the speakers connected to the Raspberry Pi
func (z *Zuul) PlayWelcomeFile(w http.ResponseWriter, r *http.Request) {
	log.Println("Received welcome audio play request")
	f, err := os.Open(z.WelcomeFile)
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
func (z *Zuul) Say(w http.ResponseWriter, r *http.Request) {
	text := r.PostFormValue("text")
	log.Printf("Received text so speak: %v", text)
	cmd := exec.Command("espeak", "-ves+f4", "-s150", fmt.Sprintf("\"%v\"", text))
	if err := cmd.Run(); err != nil {
		http.Error(w, err.Error(), 500)
	}
}
