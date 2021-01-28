#include "tcp_client.h"

#include "main.h"
#include "lwip/api.h"
#include "string.h"

/*
 * Notes:
 * 		Atjungus LANa vistiek sukasi, tik kaupia viska i buferi
 */

/* Private variables ---------------------------------------------------------*/
extern struct netif gnetif;

/* Private define ------------------------------------------------------------*/
#define TCP_CLIENT_THREAD_PRIO  ( tskIDLE_PRIORITY + 4 )

/* Private function prototypes -----------------------------------------------*/
static void tcp_client_thread(void *arg);
static void tcp_periodic_action(struct netconn*);

/* Public functions ----------------------------------------------------------*/
void tcp_client_init(void) {
	sys_thread_new("tcp_client_thread", tcp_client_thread, NULL,
	DEFAULT_THREAD_STACKSIZE, TCP_CLIENT_THREAD_PRIO);
}

/* Private functions ---------------------------------------------------------*/
static void tcp_client_thread(void *arg) {
	struct netconn *conn/*, *newconn*/;
	err_t err, connect_err;
	ip_addr_t local_ip, remote_ip;

	LWIP_UNUSED_ARG(arg);

	IP4_ADDR(&local_ip, 192, 168, 1, 49);
	IP4_ADDR(&remote_ip, 192, 168, 1, 107);

	/* Create a new connection identifier. */
	conn = netconn_new(NETCONN_TCP);
	if (conn != NULL) {
		/* Bind connection to a known port number */
		err = netconn_bind(conn, &local_ip, 55151);
		if (err == ERR_OK) {
			/* Tell connection to go into listening mode */
			while (1) {
				connect_err = netconn_connect(conn, &remote_ip, 55151);
				/*
				 * ERR_OK - viskas gerai, testi
				 * ERR_ISCONN - jau prisijungta
				 * ERR_ALREADY - galbut gali reiket (rodo kad jungiasi)
				 */
				if (connect_err == ERR_OK || connect_err == ERR_ISCONN) {
					// Prisijungta, daryti veiksmus
					tcp_periodic_action(conn);
				} else {
					// Perkurti sasaja is naujo, kad butu galima prisijungti
					netconn_close(conn);
					netconn_delete(conn);
					conn = netconn_new(NETCONN_TCP);
					vTaskDelay(100); // Tiesiog palaukti kad neperkurinet super daznai
				}
			}
		} else {
			netconn_delete(conn);
		}
	}
}

static void tcp_periodic_action(struct netconn *conn) {
	struct netbuf *buf;
	uint16_t msg_len = 18;

	buf = netbuf_new();
	netbuf_alloc(buf, msg_len);
	buf->p->payload = "Hello TCP world!\n\r";
	buf->p->len = msg_len;

	netconn_write(conn, "Hello TCP world!\n\r", msg_len, NETCONN_COPY);
	netbuf_delete(buf);

//					while (netconn_recv(conn, &buf) == ERR_OK) {
//						do {
//							netbuf_data(buf, &data, &len);
//							netconn_write(conn, data, len, NETCONN_COPY);
//
//						} while (netbuf_next(buf) >= 0);
//
//						netbuf_delete(buf);
//					}
//
//					/* Close connection and discard connection identifier. */
//					netconn_close(conn);

	vTaskDelay(1000);
}

