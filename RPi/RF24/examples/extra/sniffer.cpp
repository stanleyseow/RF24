/*
Copyright (C) 2014 Andrzej Bieniek <andyhelp@gmail.com> 

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.


Sniffing implementation based on
http://blog.cyberexplorer.me/2014/01/sniffing-and-decoding-nrf24l01-and.html
*/

#include <cstdlib>
#include <iostream>
#include <RF24/RF24.h>


#define RADIO_CE_PIN    (RPI_V2_GPIO_P1_22)
#define RADIO_CSN_PIN   (RPI_V2_GPIO_P1_24)
RF24 radio(RADIO_CE_PIN, RADIO_CSN_PIN, BCM2835_SPI_SPEED_8MHZ);


void setup(int channel, uint64_t rx_addr, int rx_addr_len, int payload_size, rf24_datarate_e speed) {
	radio.begin();
	radio.setDataRate(speed);
	radio.setAutoAck(false);
	radio.setChannel(channel);
	radio.disableCRC();
	radio.setPayloadSize(payload_size);
	radio.setAddressWidth(rx_addr_len);
	radio.openReadingPipe(1,rx_addr);
	radio.stopListening();
	radio.printDetails();
}

void run(int rx_addr_len) {
	radio.startListening();
	while(1) {
		delayMicroseconds(10);
		while (radio.available()) {
			char buf[rx_addr_len];

			radio.read(buf, rx_addr_len);
			for(int i=0; i<rx_addr_len; i++) {
				printf("%.2x", buf[i]);
			}
			printf("\n");
		}
	}
	radio.stopListening();
}


int main(int argc, char** argv) {
	int payload_size = 32;
	uint64_t rx_addr = 0xaa;
	rf24_datarate_e speed = RF24_1MBPS;
	int channel = 0;
	int rx_addr_len = 2;
	int c;

	while ((c = getopt (argc, argv, "a:c:l:s:p:h")) != -1) {
		switch (c) {
		case 'a':
			rx_addr = strtoll(optarg, NULL, 0);
			break;
		case 'c':
			channel = atoi(optarg);
			if (channel < 0 || channel > 128) {
				printf("error: channel %d out of range (allowed values from 0 to 128)\n", channel);
				abort();
			}
			break;
		case 'l':
			rx_addr_len = atoi(optarg);
			if (rx_addr_len < 2 || rx_addr_len > 5) {
				printf("error: addr lenght %d out of range (allowed values from 2 to 5\n", rx_addr_len);
			}
			break;
		case 's':
			switch(atoi(optarg)) {
			case 250:
				speed = RF24_250KBPS;
				break;
			case 1000:
				speed = RF24_1MBPS;
				break;
			case 2000:
				speed = RF24_2MBPS;
				break;
			default:
				printf("error: uknown speed %s (allowed values in kbps: 250, 1000, 2000)\n", optarg);
				abort();
				break;
			}
			break;
		case 'p':
			payload_size = atoi(optarg);
			if (payload_size < 1 || payload_size > 32) {
				printf("error: payload %d out of range (allowed values are from 1 to 32)\n", payload_size);
				abort();
			}
			break;
		case 'h':
		case '?':
		default:
			printf("%s [OPTIONS]\n\n", argv[0]);
			printf("  -a ADDRESS\t- hex address value, e.g.: -a 0x1234AB\n");
			printf("  -l ADDR_LEN\t- address length in bytes\n");
			printf("  -c CHANNEL\t- RF channel from 0 to 128 \n");
			printf("  -s SPEED\t- Modulation speed in kbps: 250, 1000, 2000, e.g. -s 1000\n");
			printf("  -p PAYLOAD_LEN\t- payload length in bytes, maximum is 32\n");
			abort ();
			break;
		}
	}
	if (rx_addr >> (rx_addr_len * 8)) {
		printf("error: address 0x%llx is longer than %d bytes\n", rx_addr, rx_addr_len);
		abort();
	}
	setup(channel, rx_addr, rx_addr_len, payload_size, speed);
	run(payload_size);
	return 0;
}
