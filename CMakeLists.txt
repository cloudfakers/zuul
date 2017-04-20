cmake_minimum_required(VERSION 2.6)
project(Zuul C)
add_executable(zuul zuul.c)
target_link_libraries(zuul
    circus
    pthread
    wiringPi
    mpg123
    ao)
install(TARGETS zuul DESTINATION bin)
