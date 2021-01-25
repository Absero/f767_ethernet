#include "index.h"

#include <string.h>
#include "lwip/api.h"
#include "html_webpage.h"

/* Private define ------------------------------------------------------------*/
#define WEBSERVER_THREAD_PRIO    ( tskIDLE_PRIORITY + 4 )

/* Private function prototypes -----------------------------------------------*/
static void http_server_netconn_thread(void *arg);
static void http_server_serve(struct netconn *conn);

/* Public functions ----------------------------------------------------------*/
void http_server_netconn_init() {
	sys_thread_new("HTTP", http_server_netconn_thread, NULL,
	DEFAULT_THREAD_STACKSIZE, WEBSERVER_THREAD_PRIO);
}

/* Private functions ---------------------------------------------------------*/
static void http_server_netconn_thread(void *arg) {
	struct netconn *conn, *newconn;
	err_t err, accept_err;

	/* Create a new TCP connection handle */
	conn = netconn_new(NETCONN_TCP);

	if (conn != NULL) {
		/* Bind to port 80 (HTTP) with default IP address */
		err = netconn_bind(conn, NULL, 80);

		if (err == ERR_OK) {
			/* Put the connection into LISTEN state */
			netconn_listen(conn);

			while (1) {
				/* accept any icoming connection */
				accept_err = netconn_accept(conn, &newconn);
				if (accept_err == ERR_OK) {
					/* serve connection */
					http_server_serve(newconn);

					/* delete connection */
					netconn_delete(newconn);
				}
			}
		}
	}
}

static void http_server_serve(struct netconn *conn) {
	struct netbuf *inbuf;
	err_t recv_err;
	char *buf;
	u16_t buflen;

	/* Read the data from the port, blocking if nothing yet there.
	 We assume the request (the part we care about) is in one netbuf */
	recv_err = netconn_recv(conn, &inbuf);

	if (recv_err == ERR_OK) {
		if (netconn_err(conn) == ERR_OK) {
			netbuf_data(inbuf, (void**) &buf, &buflen);

			/* Is this an HTTP GET command? (only check the first 5 chars, since
			 there are other formats for GET, and we're keeping it very simple )*/
			if ((buflen >= 5) && (strncmp(buf, "GET /", 5) == 0)) {
				if (strncmp((char const*) buf, "GET /index.html", 15) == 0) {
					netconn_write(conn, (const unsigned char* ) index_html,
							index_html_len, NETCONN_NOCOPY);
				}
			}
		}
	}

	/* Close the connection (server closes in HTTP) */
	netconn_close(conn);

	/* Delete the buffer (netconn_recv gives us ownership,
	 so we have to make sure to deallocate the buffer) */
	netbuf_delete(inbuf);
}

