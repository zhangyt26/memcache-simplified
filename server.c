#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

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
	unsigned char *extra;
	unsigned char *key;
	unsigned char *value;
};

int main(int argc, char **argv) {
	
	int listen_socket, conn_socket;
	struct sockaddr_in servaddr;
	int err;

	printf("%lu\n", sizeof(struct packet));

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
		socklen_t clilen = sizeof(cliaddr);
		char *buffer = malloc(HEADER_LENGTH);
		struct packet recv_packet;

		bzero(buffer, HEADER_LENGTH);
		conn_socket = accept(listen_socket, (struct sockaddr *) &cliaddr, &clilen);
		printf("accept socket conn");
		err = read(conn_socket, buffer, HEADER_LENGTH);
		// printf("packet content %x\n", (char )*(buffer+1));

		memcpy(&recv_packet, buffer, sizeof(struct packet));

		if (recv_packet.opcode == 0) {
			int keyLength = recv_packet.key_length[1] | recv_packet.key_length[0] << 8;
			recv_packet.key = malloc(keyLength);
			read(conn_socket, recv_packet.key, keyLength);
			printf("it is get, key is %s\n", recv_packet.key);

		}
		else if (recv_packet.opcode == 1) {
			printf("it is a get\n");
		}
	}
}