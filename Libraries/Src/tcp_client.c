#include "tcp_client.h"

#include "lwip/api.h"

/* Private define ------------------------------------------------------------*/
#define TCP_CLIENT_THREAD_PRIO  ( tskIDLE_PRIORITY + 4 )

/* Private function prototypes -----------------------------------------------*/
static void tcp_client_thread(void *arg);

/* Public functions ----------------------------------------------------------*/
void tcp_client_init(void) {
	sys_thread_new("tcp_client_thread", tcp_client_thread, NULL,
	DEFAULT_THREAD_STACKSIZE, TCP_CLIENT_THREAD_PRIO);
}

/* Private functions ---------------------------------------------------------*/
static void tcp_client_thread(void *arg) {
	struct netconn *conn/*, *newconn*/;
	err_t err, connect_err;
	struct netbuf *buf;
	void *data;
	u16_t len;
	ip_addr_t local_ip, remote_ip;

	LWIP_UNUSED_ARG(arg);

	IP4_ADDR(&local_ip, 192, 168, 1, 49);
	IP4_ADDR(&remote_ip, 192, 168, 1, 107); //todo patikrinti ar tikrai 107

	/* Create a new connection identifier. */
	conn = netconn_new(NETCONN_TCP);

	if (conn != NULL) {
		/* Bind connection to a known port number */
		err = netconn_bind(conn, &local_ip, 55151);

		if (err == ERR_OK) {
			/* Tell connection to go into listening mode */
//			netconn_listen(conn);
			while (1) {
				/* Grab new connection. */
				connect_err = netconn_connect(conn, &remote_ip, 55152); //todo paziuret portai ar geri

				/* Process the new connection. */
				if (connect_err == ERR_OK) {

					while (netconn_recv(conn, &buf) == ERR_OK) {
						do {
							netbuf_data(buf, &data, &len);
							netconn_write(conn, data, len, NETCONN_COPY);

						} while (netbuf_next(buf) >= 0);

						netbuf_delete(buf);
					}

					/* Close connection and discard connection identifier. */
					netconn_close(conn);
					netconn_delete(conn);
				}
			}
		} else {
			netconn_delete(conn);
		}
	}
}
