// discover.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

/*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU Library General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program; if not, write to the Free Software
*  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <artnet/artnet.h>

int verbose = 0;
int nodes_found = 0;


void print_node_config(artnet_node_entry ne) {
	printf("--------- %d.%d.%d.%d -------------\n", ne->ip[0], ne->ip[1], ne->ip[2], ne->ip[3]);
	printf("Short Name:   %s\n", ne->shortname);
	printf("Long Name:    %s\n", ne->longname);
	printf("Node Report:  %s\n", ne->nodereport);
	printf("Subnet:       0x%02x\n", ne->sub);
	printf("Numb Ports:   %d\n", ne->numbports);
	printf("Input Addrs:  0x%02x, 0x%02x, 0x%02x, 0x%02x\n", ne->swin[0], ne->swin[1], ne->swin[2], ne->swin[3]);
	printf("Output Addrs: 0x%02x, 0x%02x, 0x%02x, 0x%02x\n", ne->swout[0], ne->swout[1], ne->swout[2], ne->swout[3]);
	printf("----------------------------------\n");
}

int reply_handler(artnet_node n, void *pp, void *d) {
	artnet_node_list nl = artnet_get_nl(n);

	if (nodes_found == artnet_nl_get_length(nl)) 
	{
		// this is not a new node, just a previously discovered one sending
		// another reply
		return 0;
	}
	else if (nodes_found == 0) 
	{
		// first node found
		nodes_found++;
		print_node_config(artnet_nl_first(nl));
	}
	else 
	{
		// new node
		nodes_found++;
		print_node_config(artnet_nl_next(nl));
	}

	return 0;
}

int quit(int error_code)
{
	int n;
	std::cout << "enter any key to quit: ";
	std::cin >> n;
	return error_code;
}

int main(int argc, char *argv[])
{
	artnet_node node;
	const char *ip_addr = NULL;
	int sd, maxsd, timeout = 2;
	fd_set rset;
	struct timeval tv;
	time_t start;

	// Simple parse
	for (int i = 0; i < argc; i++)
	{
		std::string sa(argv[i]);
		if (sa == "-a")
		{
			assert(i + 1 < argc);
			ip_addr = argv[i + 1];
			continue;
		}
		if (sa == "-v")
		{
			verbose = 1;
			continue;
		}
		if (sa == "-t")
		{
			assert(i + 1 < argc);
			timeout = atoi(argv[i + 1]);
			continue;
		}
	}

	// Create new node
	if ((node = artnet_new(ip_addr, verbose)) == NULL) 
	{
		printf("new failed %s\n", artnet_strerror());
		return quit(1);
	}

	// Set node name and type (server)
	artnet_set_short_name(node, "artnet-discovery");
	artnet_set_long_name(node, "ArtNet Discovery Node");
	artnet_set_node_type(node, ARTNET_SRV);

	// start node
	if (artnet_start(node) != ARTNET_EOK) 
	{
		printf("Failed to start: %s\n", artnet_strerror());
		return quit(1);
	}

	// set poll reply handler
	artnet_set_handler(node, ARTNET_REPLY_HANDLER, reply_handler, NULL);

	// get the socket descriptor
	sd = artnet_get_sd(node);

	// broadcast a poll request
	if (artnet_send_poll(node, NULL, ARTNET_TTM_DEFAULT) != ARTNET_EOK) 
	{
		printf("send poll failed\n");
		return quit(1);
	}

	start = time(NULL);
	
	// wait for timeout seconds before quitting
	while (time(NULL) - start < timeout) 
	{
		FD_ZERO(&rset);
		FD_SET(sd, &rset);

		tv.tv_usec = 0;
		tv.tv_sec = 1;
		maxsd = sd;

		switch (select(maxsd + 1, &rset, NULL, NULL, &tv)) {
		case 0:
			// timeout
			break;
		case -1:
			printf("select error\n");
			break;
		default:
			artnet_read(node, 0);
			break;
		}
	}
    return quit(0);
}

