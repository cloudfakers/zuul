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

#include <stdio.h>
#include <stdlib.h>
#include <circus/irc.h>


/* Disconnect if the nick is in use */
void on_nick_in_use(ErrorEvent* event) {
    printf("Nick %s is already in use\n", event->params[1]);
    irc_quit("Bye");
    irc_disconnect();
    exit(EXIT_FAILURE);
}

/* IRC callback to open the door */
void open_door(MessageEvent* event) {
    char msg[30];
    snprintf(msg, 30, "Opening door!");
    irc_message(event->to, msg);
    /* TODO: Configure the GPIO pins to open the door */
}

int main(int argc, char **argv) {
    char *irc_server, *irc_port, *irc_nick, *irc_channel;

    if (argc < 4) {
        printf("Usage: %s <irc server> <irc port> <nickname> <channel>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    irc_server = argv[1];
    irc_port = argv[2];
    irc_nick = argv[3];
    irc_channel = argv[4];

    irc_bind_event(ERR_NICKNAMEINUSE, (Callback) on_nick_in_use);
    irc_bind_command("puerta!", (Callback) open_door);

    irc_connect(irc_server, irc_port);
    irc_login(irc_nick, "Zuul", "Abiquo gatekeeper");
    irc_join(irc_channel);
    irc_listen();

    irc_quit("Bye");
    irc_disconnect();

    return 0;
}
