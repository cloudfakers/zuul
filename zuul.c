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
#include <unistd.h>
#include <circus/irc.h>
#include <wiringPi.h>
#include <ao/ao.h>
#include <mpg123.h>

/* Audio file to play when the door is opened */
char* audio_file;

/* Disconnect if the nick is in use */
void on_nick_in_use(ErrorEvent* event) {
    printf("Nick %s is already in use\n", event->params[1]);
    irc_quit("Bye");
    irc_disconnect();
    exit(EXIT_FAILURE);
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

/* Plays the configured sound file */
void play_sound(MessageEvent* event) {
    mpg123_handle *mh;
    unsigned char *buffer;
    size_t buffer_size, done;
    int err, driver, channels, encoding;
    ao_device *dev;
    ao_sample_format format;
    long rate;
    
    /* initializations */
    ao_initialize();
    driver = ao_default_driver_id();
    mpg123_init();
    mh = mpg123_new(NULL, &err);
    buffer_size = mpg123_outblock(mh);
    buffer = (unsigned char*) malloc(buffer_size * sizeof(unsigned char));

    /* open the file and get the decoding format */
    mpg123_open(mh, audio_file);
    mpg123_getformat(mh, &rate, &channels, &encoding);

    /* set the output format and open the output device */
    format.bits = mpg123_encsize(encoding) * 8;
    format.rate = rate;
    format.channels = channels;
    format.byte_format = AO_FMT_NATIVE;
    format.matrix = 0;
    dev = ao_open_live(driver, &format, NULL);

    /* decode and play */
    while (mpg123_read(mh, buffer, buffer_size, &done) == MPG123_OK) {
        ao_play(dev, buffer, done);
    }

    /* clean up */
    free(buffer);
    ao_close(dev);
    mpg123_close(mh);
    mpg123_delete(mh);
    mpg123_exit();
    ao_shutdown();
}

/* IRC callback to open the door */
void open_door(MessageEvent* event) {
    irc_message(event->to, "Vamos esa puertaaaaaa!!!!");
    irc_message(event->to, "afuego http://zuul.bcn.abiquo.com:8080/?action=stream");
    
    digitalWrite(GPIO_PIN, GPIO_ON);
    delay(500);
    digitalWrite(GPIO_PIN, GPIO_OFF);

    delay(4000);
    play_sound(event);
}

int main(int argc, char **argv) {
    char irc_channel[20];
    char *irc_server, *irc_port, *irc_nick, *irc_channel_pass = NULL;

    if (argc < 6) {
        printf("Usage: %s <audio file> <irc server> <irc port> <nickname> <channel>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    init_gpio();

    audio_file = argv[1];
    if (access(audio_file, R_OK) == -1) {
        printf("Audio file '%s' does not exist\n", audio_file);
        exit(EXIT_FAILURE);
    }

    irc_server = argv[2];
    irc_port = argv[3];
    irc_nick = argv[4];
    snprintf(irc_channel, sizeof irc_channel, "#%s", argv[5]);
    if (argc > 5) {
        irc_channel_pass = argv[6];
    }

    irc_bind_event(ERR_NICKNAMEINUSE, (Callback) on_nick_in_use);
    irc_bind_command("puerta!", (Callback) open_door);
    irc_bind_command("palante!", (Callback) play_sound);

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
