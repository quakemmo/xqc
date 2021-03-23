/*	DESCRIPTION

	Ping the server.
*/
#include <fcntl.h>

#ifdef _WIN32
#   include <winsock2.h>
#   include <ws2tcpip.h>
#   if WINVER < 0x501
#       ifdef __MINGW32__
            // wspiapi.h isn't available on MinGW, so if it's
            // present it's because the end user has added it
            // and we should look for it in our tree
#           include "wspiapi.h"
#       else
#           include <wspiapi.h>
#       endif
#   else
#       include <ws2spi.h>
#   endif
#else
#	include <netinet/in.h>
#	include <netdb.h>
#	include <sys/types.h>
#	include <sys/socket.h>
#	include <sys/resource.h>
#endif

#include "../cg_local.h"

void xq_ping(void) {
	static time_t master_ping_timer = 0;
	time_t xq_time;
	char key[XQ_SESSION_LEN+1], buf[20];

	xq_time = time(NULL);
	if ((xq_time - master_ping_timer) >= 2) {
		master_ping_timer = xq_time;
	} else {
		return;
	}

    trap_Cvar_VariableStringBuffer("cl_crypto_key", key, sizeof(key));
	if (strlen(key) == 0) {
		return;
	}
    trap_Cvar_VariableStringBuffer("cl_crypto_keyid", buf, sizeof(buf));
	uint64_t keyid = S64(buf);


	if (strlen(xqst->ui_charname) < 1) {
		return;
	}

	char name[XQ_MAX_CHAR_NAME+1];
	memset(name, 0, sizeof(name));
	Q_strncpyz(name, xqst->ui_charname, sizeof(name));
	name[0] = tolower(name[0]);


	#define blocksize 16
	#define numblocks 70
	#define payload_max (blocksize*2)

	char payload[payload_max+1];
	int i;
	for (i = 0;  i < payload_max;  i++) {
		payload[i] = random() * 256;
	}
	payload[i] = 0;
	#ifdef _WIN32
		snprintf(payload, payload_max, "%I64u:%s", keyid, name);
	#else
		snprintf(payload, payload_max, "%lu:%s", keyid, name);
	#endif

	byte wbuf[numblocks*16+8+1];
	int len = numblocks*16+8;
	uint16_t payload_len = strlen(payload);

	// find a random offset to put actual info in
	byte offset = (int)(random() * (numblocks - 6));
	offset += 3;

	// put random stuff in the buffer
    for (i = 0;  i < len;  i++) {
		wbuf[i] = random() * 256;
	}

	// craft the data packet

	// 8 bytes: keyid
	memcpy(wbuf, &keyid, 8);


	// (encryption starts here) 1 byte: offset
	wbuf[8] = offset;

	// 2 bytes: payload length
	memcpy(wbuf+9, &payload_len, 2);

	// now put the payload in the offset
	memcpy(wbuf + 8 + (offset * blocksize), payload, payload_len); // encrypted data starts at wbuf + 8 (contains payload's offset, payload_len, payload, and random padding around)

	// encrypt
	trap_XQ_Encrypt(wbuf+8, len-8, key, strlen(key));


	// send it all

	#if 0
		xq_clog(COLOR_WHITE, "xq_ping: payload: [%s], payload_len: %i, keyid: %lu, (signed: %li), name: %s, offset: %i, total len sent: %i, encrypted len: %i",
			payload, payload_len, keyid, keyid, name, offset, len, len-8);
	#endif


	#ifdef _WIN32
		static SOCKADDR_IN si, clsi;
		static SOCKET sock;
	#else
		static struct sockaddr_in si, clsi;
		static int sock;
	#endif
	static int init_done = 0;
	if (init_done == 0) {
		init_done = 1;

		sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		memset(&si, 0, sizeof(si));

		
		char hostbuf[50];
		trap_Cvar_VariableStringBuffer("cl_currentServerAddress", hostbuf, sizeof(hostbuf));
		if (strchr(hostbuf, ':')) {
			*(strchr(hostbuf, ':')) = 0;
		}
		
		#ifdef _WIN32
			HOSTENT *h;
		#else
			struct hostent *h;
		#endif
		h = gethostbyname(hostbuf);
		if (h == NULL) {
			xq_clog(COLOR_RED, "xq_ping: gethostbyname on %s failed", hostbuf);
			return;
		}

		si.sin_family = AF_INET;
		si.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
		si.sin_port = htons((unsigned short)10942);
		memcpy(&si.sin_addr, h->h_addr_list[0], h->h_length);


		clsi.sin_family = AF_INET;
		clsi.sin_addr.s_addr = htonl(INADDR_ANY);
		clsi.sin_port = htons(0);
  
		int rc = bind(sock, (struct sockaddr *) &clsi, sizeof(clsi));
		if (rc != 0) {
			xq_clog(COLOR_RED, "Cannot bind master UDP socket.");
			return;
		}

#ifdef _WIN32
		// XXX need to set to nonblocking on win32
#else
		if (fcntl(sock, F_SETFL, O_NONBLOCK, 1) == -1) {
			xq_clog(COLOR_RED, "Cannot nonblock master UDP socket.");
			return;
		}
#endif
	}

#ifdef _WIN32
	sendto(sock, (void *)wbuf, len, 0, (struct sockaddr *)&si, sizeof(struct sockaddr));
#else
	sendto(sock, (void *)wbuf, len, MSG_DONTWAIT, (struct sockaddr *)&si, sizeof(struct sockaddr));
#endif
}
