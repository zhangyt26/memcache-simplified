#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

#include "hashmap.h"

#define DEFAULT_PORT 11211
#define HEADER_LENGTH 24

struct packet {
	unsigned char magic;
	unsigned char opcode;
	unsigned char key_length[2];
	unsigned char extras_length;
	unsigned char data_type;
	unsigned char reserverd_or_status[2];
	unsigned char total_body_length[4];
	unsigned char opaque[4];
	unsigned char cas[8];
};


void *client_thread(void *arg) {
	int conn_socket;
	int err;

	memcpy(&conn_socket, arg, sizeof(int));
	// conn_socket = * ((int *) arg);
	// pthread_detach(pthread_self());

	for ( ; ; ) {
		struct packet *recv_packet = malloc(HEADER_LENGTH);
		bzero(recv_packet, HEADER_LENGTH);
		err = read(conn_socket, (void *)recv_packet, HEADER_LENGTH);
		if (err == 0) {
			printf("TCP FIN! Close and return thread\n");
			close(conn_socket);
			return NULL;
		}

		if (recv_packet->opcode == 0) {
			int key_length = recv_packet->key_length[1] | recv_packet->key_length[0] << 8;
			char *recv_key;
			char *value;
			struct packet send_packet;

			recv_key = malloc(key_length+1);
			bzero(recv_key, key_length);
			err = read(conn_socket, recv_key, key_length);
			recv_key[key_length] = '\0';
			printf("it is a get, key is %s\n", recv_key);
			value = map_get(recv_key);

			bzero(&send_packet, sizeof(struct packet));
			send_packet.magic = 0x81;
			send_packet.opcode = 0x00;
			if (value != NULL) {
				// found a value
				printf("found a value: %s\n", value);
				int value_length = strlen(value);
				int body_length = value_length + 4;
				char send_buffer[sizeof(struct packet) + value_length + 4];
				char extras[4] = {0x00, 0x00, 0x00, 0x00};

				bzero(send_buffer, sizeof(struct packet) + value_length);
				send_packet.reserverd_or_status[0] = 0x00;
				send_packet.reserverd_or_status[1] = 0x00;
				send_packet.extras_length = 4 & 0xFF;
				send_packet.total_body_length[3] = body_length & 0xFF;
				send_packet.total_body_length[2] = body_length >> 8 & 0xFF;
				send_packet.total_body_length[1] = body_length >> 16 & 0xFF;
				send_packet.total_body_length[0] = body_length >> 24 & 0xFF;
				// send_packet.cas[7] = 1 & 0xFF;
				// send_packet.total_body_length = (char *)&value_length;
				memcpy(send_buffer, &send_packet, HEADER_LENGTH);
				memcpy(send_buffer+HEADER_LENGTH, extras, 4);
				memcpy(send_buffer+HEADER_LENGTH+4, value, value_length);
				write(conn_socket, (void *)send_buffer, sizeof(send_buffer));

			}
			else {
				// not found
				printf("not found!\n");
				char send_buffer[sizeof(struct packet) + 9];

				bzero(send_buffer, sizeof(struct packet) + 9);
				send_packet.reserverd_or_status[0] = 0x00;
				send_packet.reserverd_or_status[1] = 0x01;
				send_packet.total_body_length[3] = 9 & 0xFF;
				memcpy(send_buffer, &send_packet, HEADER_LENGTH);
				memcpy(send_buffer+HEADER_LENGTH, "Not found", 9);
				write(conn_socket, (void *)send_buffer, sizeof(send_buffer));
			}

			free(recv_key);

		}
		else if (recv_packet->opcode == 1) {

			struct packet send_packet;
			int key_length = recv_packet->key_length[1] | recv_packet->key_length[0] << 8;
			int extras_length = recv_packet->extras_length;
			int body_length = recv_packet->total_body_length[3] | recv_packet->total_body_length[2] << 8
							| recv_packet->total_body_length[1] << 16 | recv_packet->total_body_length[0] << 24;
			int value_length = body_length-extras_length-key_length;
			char *key = malloc(key_length+1);
			char *extra = malloc(extras_length+1);
			char *value = malloc(value_length+1);

			bzero(key, key_length);
			bzero(extra, extras_length);
			bzero(value, value_length);
			key[key_length] = '\0';
			value[value_length] = '\0';

			err = read(conn_socket, extra, extras_length);
			err = read(conn_socket, key, key_length);
			err = read(conn_socket, value, value_length);
			printf("it is a set: key: %s, value: %s\n", key, value);
			map_set(key, value);

			bzero(&send_packet, sizeof(struct packet));
			send_packet.magic = 0x81;
			send_packet.opcode = 0x01;
			send_packet.reserverd_or_status[0] = 0x00;
			send_packet.reserverd_or_status[1] = 0x00;
			write(conn_socket, (void *)&send_packet, sizeof(send_packet));
			free(key);
			free(extra);
			free(value);
		}
		free(recv_packet);
	}

}

int main(int argc, char **argv) {
	
	int listen_socket, conn_socket;
	struct sockaddr_in servaddr;
	int err;


	// map_set("key1", "value");
	// map_set("key2", "value");
	// // map_print();
	// map_set("key3", "value");
	// // map_print();
	// map_set("key2", "xfa");
	// map_print();

	// printf("get %s\n", map_get("key2"));
	// return 1;

	listen_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_socket < 0) {
		printf("error creating socket!\n");
	}

	bzero (&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(DEFAULT_PORT);

	err = bind(listen_socket, (struct sockaddr *) &servaddr, sizeof(servaddr));
	if (err < 0) {
		printf("error binding socket!\n");
	}

	err = listen(listen_socket, 5);

	for ( ; ; ) {
		struct sockaddr_in cliaddr;
		pthread_t thread_id;
		socklen_t clilen = sizeof(cliaddr);
		printf("waiting for connection\n");
		conn_socket = accept(listen_socket, (struct sockaddr *) &cliaddr, &clilen);
		printf("accept socket conn\n");

		err = pthread_create(&thread_id, NULL, &client_thread, &conn_socket);
		if (err < 0) {
			printf("error creating thread\n");
		}
	}
}