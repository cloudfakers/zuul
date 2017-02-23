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

/* For Raspberry Pi 2 and Pi 3, change to 0x3F000000 */
#define BCM2708_PERI_BASE 0x20000000
#define GPIO_BASE (BCM2708_PERI_BASE + 0x200000) /* GPIO controller */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <circus/irc.h>

#define PAGE_SIZE (4 * 1024)
#define BLOCK_SIZE (4 * 1024)

int  mem_fd;
void *gpio_map;
volatile unsigned *gpio; /* I/O access */ 

/* Always use INP_GPIO(x) before using OUT_GPIO(x) or SET_GPIO_ALT(x,y) */
#define INP_GPIO(g) *(gpio+((g)/10)) &= ~(7<<(((g)%10)*3))
#define OUT_GPIO(g) *(gpio+((g)/10)) |=  (1<<(((g)%10)*3))
#define SET_GPIO_ALT(g,a) *(gpio+(((g)/10))) |= (((a)<=3?(a)+4:(a)==4?3:2)<<(((g)%10)*3))
#define GPIO_SET *(gpio+7)  /* sets   bits which are 1 ignores bits which are 0 */
#define GPIO_CLR *(gpio+10) /* clears bits which are 1 ignores bits which are 0 */
#define GET_GPIO(g) (*(gpio+13)&(1<<g)) /* 0 if LOW, (1<<g) if HIGH */
#define GPIO_PULL *(gpio+37) /* Pull up/pull down */
#define GPIO_PULLCLK0 *(gpio+38) /* Pull up/pull down clock */

/* Set up a memory regions to access GPIO */
void setup_io() {
    /* open /dev/mem */
    if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) {
        printf("can't open /dev/mem \n");
        exit(EXIT_FAILURE);
    }

    /* mmap GPIO */
    gpio_map = mmap(
        NULL,             /* Any adddress in our space will do */
        BLOCK_SIZE,       /* Map length */
        PROT_READ|PROT_WRITE, /* Enable reading & writting to mapped memory */
        MAP_SHARED,       /* Shared with other processes */
        mem_fd,           /* File to map */
        GPIO_BASE         /* Offset to GPIO peripheral */
    );

    close(mem_fd); /* No need to keep mem_fd open after mmap */

    if (gpio_map == MAP_FAILED) {
        printf("mmap error %d\n", (int)gpio_map); /* errno also set! */
        exit(EXIT_FAILURE);
    }

    /* Always use volatile pointer! */
    gpio = (volatile unsigned *)gpio_map;
}

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
