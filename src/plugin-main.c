/*
Plugin Name
Copyright (C) <Year> <Developer> <Email Address>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program. If not, see <https://www.gnu.org/licenses/>
*/

#define USE_LIBOBS_MAIN_THREADING

// OBS
#include <obs-module.h>
#include <util/platform.h>
#include <util/threading.h>
#include <obs-frontend-api.h>

// System
#include <pthread.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

// Platform-specific
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#endif


OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")





static pthread_t socket_thread;
static volatile bool keep_running = true;

struct switch_data {
	char name[512];
};


static void switch_scene(void *data)
{
	struct switch_data *sd = (struct switch_data *)data;
	obs_source_t *scene = obs_get_source_by_name(sd->name);
	if (scene) {
		obs_frontend_set_current_scene(scene);
		blog(LOG_INFO, "[CPlugin3] Switched to scene (main thread): %s", sd->name);
		obs_source_release(scene);
	} else {
		blog(LOG_WARNING, "[CPlugin] Scene not found: %s", sd->name);
	}
	bfree(sd);
}


// -- SOCKET FUNCTION --
static void *socket_listener(void *arg)
{
#ifdef _WIN32
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);
#endif

	#ifdef _WIN32
		SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
		if (sock == INVALID_SOCKET) {
			blog(LOG_ERROR, "[CPlugin] Failed to create socket");
			return NULL;
		}
	#else
		int sock = socket(AF_INET, SOCK_STREAM, 0);
		if (sock < 0) {
			blog(LOG_ERROR, "[CPlugin] Failed to create socket");
			return NULL;
		}
	#endif
	

	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_port = htons(12345);
	inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);

	if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
		blog(LOG_ERROR, "[CPlugin] Failed to connect to server");
#ifdef _WIN32
		closesocket(sock);
		WSACleanup();
#else
		close(sock);
#endif
		return NULL;
	}

	blog(LOG_INFO, "[CPlugin3] Connected to server");

	char buffer[1024];

	while (keep_running) {
		int bytes = recv(sock, buffer, sizeof(buffer) - 1, 0);
		if (bytes <= 0)
			break;

		buffer[bytes] = '\0';
		blog(LOG_INFO, "[CPlugin2] Received: %s", buffer);

		if (strncmp(buffer, "SCENE:", 6) == 0) {
			const char *scene_name = buffer + 6;

			struct switch_data *sd = bzalloc(sizeof(struct switch_data));
			strncpy(sd->name, scene_name, sizeof(sd->name) - 1);

			switch_scene(sd);

			//  obs_frontend_invoke(switch_scene, sd);
		}

	}

#ifdef _WIN32
	closesocket(sock);
	WSACleanup();
#else
	close(sock);
#endif

	blog(LOG_INFO, "[CPlugin] Socket listener stopped");
	return NULL;
}

// -- LOAD / UNLOAD --
bool obs_module_load(void)
{
	keep_running = true;
	if (pthread_create(&socket_thread, NULL, socket_listener, NULL) != 0) {
		blog(LOG_ERROR, "[CPlugin] Failed to create socket thread");
		return false;
	}

	blog(LOG_INFO, "[CPlugin] Module loaded");
	return true;
}

void obs_module_unload(void)
{
	keep_running = false;
	pthread_join(socket_thread, NULL);
	blog(LOG_INFO, "[CPlugin] Module unloaded");
}





const char *obs_module_description(void)
{
	return "OBS plugin written in C that connects to a TCP socket server";
}