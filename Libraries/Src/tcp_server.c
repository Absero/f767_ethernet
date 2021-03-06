#include "tcp_server.h"

#include "lwip/api.h"

/* Private define ------------------------------------------------------------*/
#define TCP_SERVER_THREAD_PRIO  ( tskIDLE_PRIORITY + 4 )

/* Private function prototypes -----------------------------------------------*/
static void tcp_server_thread(void *arg);

/* Public functions ----------------------------------------------------------*/
void tcp_server_init(void) {
	sys_thread_new("tcp_server_thread", tcp_server_thread, NULL,
	DEFAULT_THREAD_STACKSIZE, TCP_SERVER_THREAD_PRIO);
}

/* Private functions ---------------------------------------------------------*/
static void tcp_server_thread(void *arg) {
	struct netconn *conn, *newconn;
	err_t err, accept_err;
	struct netbuf *buf;
	void *data;
	u16_t len;

	LWIP_UNUSED_ARG(arg);

	/* Create a new connection identifier. */
	conn = netconn_new(NETCONN_TCP);

	if (conn != NULL) {
		/* Bind connection to a known port number */
		err = netconn_bind(conn, NULL, 7);

		if (err == ERR_OK) {
			/* Tell connection to go into listening mode */
			netconn_listen(conn);

			while (1) {
				/* Grab new connection. */
				accept_err = netconn_accept(conn, &newconn);

				/* Process the new connection. */
				if (accept_err == ERR_OK) {

					while (netconn_recv(newconn, &buf) == ERR_OK) {
						do {
							netbuf_data(buf, &data, &len);
							netconn_write(newconn, data, len, NETCONN_COPY);

						} while (netbuf_next(buf) >= 0);

						netbuf_delete(buf);
					}

					/* Close connection and discard connection identifier. */
					netconn_close(newconn);
					netconn_delete(newconn);
				}
			}
		} else {
			netconn_delete(newconn);
		}
	}
}
