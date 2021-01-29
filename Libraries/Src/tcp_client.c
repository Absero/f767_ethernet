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
#define TCP_CLIENT_RECONNECT_PERIOD 100	//Kas kiek laiko bandyt vel prisijungt prie serverio

#define TCP_CLIENT_THREAD_PRIO  ( tskIDLE_PRIORITY + 4 )

/* Private function prototypes -----------------------------------------------*/
static void tcp_client_thread(void *arg);
static void tcp_periodic_action(struct netconn*);
static void tcp_echo(struct netconn *conn);
void callbackas(struct netconn*, enum netconn_evt, u16_t len);

/* Public functions ----------------------------------------------------------*/
void tcp_client_init(void) {
	sys_thread_new("tcp_client_thread", tcp_client_thread, NULL,
	DEFAULT_THREAD_STACKSIZE, TCP_CLIENT_THREAD_PRIO);
}

/* Private functions ---------------------------------------------------------*/
static void tcp_client_thread(void *arg) {
	struct netconn *conn;
	err_t connect_err;
	ip_addr_t local_ip, remote_ip;

	LWIP_UNUSED_ARG(arg);

	IP4_ADDR(&local_ip, 192, 168, 1, 49);
	IP4_ADDR(&remote_ip, 192, 168, 1, 107);

	/* Create a new connection identifier. */
	conn = netconn_new(NETCONN_TCP);
	conn = netconn_new_with_callback(NETCONN_TCP, callbackas);
	if (conn != NULL) {
		/* Tell connection to go into listening mode */
		while (1) {
			connect_err = netconn_connect(conn, &remote_ip, 55151);
			if (connect_err == ERR_OK || connect_err == ERR_ISCONN) {
				// Prisijungta, daryti veiksmus
//				tcp_periodic_action(conn);
				tcp_echo(conn);
			} else {
				netconn_delete(conn);
				conn = netconn_new(NETCONN_TCP);
				vTaskDelay(TCP_CLIENT_RECONNECT_PERIOD); // Tiesiog palaukti kad neperkurinet super daznai
			}
		}
	}
}

static void tcp_periodic_action(struct netconn *conn) {
	uint16_t msg_len = 18;

	netconn_write(conn, "Hello TCP world!\n\r", msg_len, NETCONN_COPY);

	vTaskDelay(1000);
}

static void tcp_echo(struct netconn *conn) {
	struct netbuf *buf;
	void *data;
	u16_t len;
	if (netconn_recv(conn, &buf) == ERR_OK) {
		do {
			netbuf_data(buf, &data, &len);
			netconn_write(conn, data, len, NETCONN_COPY);
		} while (netbuf_next(buf) >= 0);
		netbuf_delete(buf);
	} else {
		// Sprendimas, bet kad perkurineja visa ta netconn gali but negerai
		netconn_delete(conn);
		conn = netconn_new(NETCONN_TCP);
		vTaskDelay(10);
	}
}

void callbackas(struct netconn *conn, enum netconn_evt event, u16_t len) {

}
