#pragma once

char* mqtt_prefix(char* pointer, const char* prefix, ...);
void mqtt_publish(const char* topic, const void* data, int length);

void mqtt_information();
void mqtt_loop();
void mqtt_setup(char* ip, int port);
