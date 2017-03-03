/*
 * Copyright (C) 2017 Ignasi Barrera
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define _POSIX_C_SOURCE 200112L

#define GPIO_PIN 9

/* We just connected the wires this way... Abiquo style! */
#define GPIO_ON 0
#define GPIO_OFF 1

#include <stdio.h>
#include <stdlib.h>
#include <circus/irc.h>
#include <wiringPi.h>

/* Disconnect if the nick is in use */
void on_nick_in_use(ErrorEvent* event) {
    printf("Nick %s is already in use\n", event->params[1]);
    irc_quit("Bye");
    irc_disconnect();
    exit(EXIT_FAILURE);
}

/* IRC callback to open the door */
void open_door(MessageEvent* event) {
    irc_message(event->to, "Vamos esa puertaaaaaa!!!!");
    irc_message(event->to, "apm guapi");
    irc_message(event->to, "afuego http://zuul.bcn.abiquo.com:8080/?action=stream");
    
    digitalWrite(GPIO_PIN, GPIO_ON);
    delay(500);
    digitalWrite(GPIO_PIN, GPIO_OFF);
}

/* Initialize the GPIO system */
void init_gpio() {
    if (wiringPiSetup() == -1) {
        printf("Could not setup the GPIO library");
        exit(EXIT_FAILURE);
    }
    pinMode(GPIO_PIN, OUTPUT);
    digitalWrite(GPIO_PIN, GPIO_OFF);
}

int main(int argc, char **argv) {
    char *irc_server, *irc_port, *irc_nick, *irc_channel, *irc_channel_pass = NULL;

    if (argc < 4) {
        printf("Usage: %s <irc server> <irc port> <nickname> <channel>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    init_gpio();

    irc_server = argv[1];
    irc_port = argv[2];
    irc_nick = argv[3];
    irc_channel = argv[4];
    if (argc > 4) {
        irc_channel_pass = argv[5];
    }

    irc_bind_event(ERR_NICKNAMEINUSE, (Callback) on_nick_in_use);
    irc_bind_command("puerta!", (Callback) open_door);

    irc_connect(irc_server, irc_port);
    irc_login(irc_nick, "Zuul", "Abiquo Gatekeeper");

    if (irc_channel_pass != NULL) {
        irc_join_pass(irc_channel, irc_channel_pass);
    } else {
        irc_join(irc_channel);
    }

    irc_listen();

    irc_quit("Bye");
    irc_disconnect();

    return 0;
}
