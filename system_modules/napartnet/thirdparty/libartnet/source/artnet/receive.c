/*
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * receive.c
 * Handles the receiving of datagrams
 * Copyright (C) 2004-2007 Simon Newton
 */

#include "private.h"

uint8_t _make_addr(uint8_t subnet, uint8_t addr);
void check_merge_timeouts(node n, int port);
void merge(node n, int port, int length, uint8_t *latest);

/*
 * Checks if the callback is defined, if so call it passing the packet and
 * the user supplied data.
 * If the callbacks return a non-zero result, further processing is canceled.
 */
int check_callback(node n, artnet_packet p, callback_t callback) {
  if (callback.fh != NULL)
    return callback.fh(n, p, callback.data);

  return 0;
}


/*
 * Handle an artpoll packet
 */
int handle_poll(node n, artnet_packet p) {
  // run callback if defined
  if (check_callback(n, p, n->callbacks.poll))
    return ARTNET_EOK;

  if (n->state.node_type != ARTNET_RAW) {
    //if we're told to unicast further replies
    if (p->data.ap.ttm & TTM_REPLY_MASK) {
      n->state.reply_addr = p->from;
    } else {
      n->state.reply_addr.s_addr = n->state.bcast_addr.s_addr;
    }

    // if we are told to send updates when node conditions change
    if (p->data.ap.ttm & TTM_BEHAVIOUR_MASK) {
      n->state.send_apr_on_change = TRUE;
    } else {
      n->state.send_apr_on_change = FALSE;
    }

    return artnet_tx_poll_reply(n, TRUE);

  }
  return ARTNET_EOK;
}

/*
 * handle an art poll reply
 */
void handle_reply(node n, artnet_packet p) {
  // update the node list
  artnet_nl_update(&n->node_list, p);

  // run callback if defined
  if (check_callback(n, p, n->callbacks.reply))
    return;
}


/*
 * handle a art dmx packet
 */
void handle_dmx(node n, artnet_packet p) {
  int i, data_length;
  output_port_t *port;
  in_addr_t ipA, ipB;

  // run callback if defined
  if (check_callback(n, p, n->callbacks.dmx))
    return;

  data_length = (int) bytes_to_short(p->data.admx.lengthHi,
                                     p->data.admx.length);
  data_length = min(data_length, ARTNET_DMX_LENGTH);

  // find matching output ports
  for (i = 0; i < ARTNET_MAX_PORTS; i++) {
    // if the addr matches and this port is enabled
    if (p->data.admx.universe == n->ports.out[i].port_addr &&
        n->ports.out[i].port_enabled) {

      port = &n->ports.out[i];
      ipA = port->ipA.s_addr;
      ipB = port->ipB.s_addr;

      // ok packet matches this port
      n->ports.out[i].port_status = n->ports.out[i].port_status | PORT_STATUS_ACT_MASK;

      /**
       * 9 cases for merging depending on what the stored ips are.
       * here's the truth table
       *
       *
       * \   ipA   #           #            #             #
       *  ------   #   empty   #            #             #
       *   ipB  \  #   ( 0 )   #    p.from  #   ! p.from  #
       * ##################################################
       *           # new node  # continued  # start       #
       *  empty    # first     #  trans-    #  merge      #
       *   (0)     #   packet  #   mission  #             #
       * ##################################################
       *           #continued  #            # cont        #
       *  p.from   # trans-    # invalid!   #  merge      #
       *           #  mission  #            #             #
       * ##################################################
       *           # start     # cont       #             #
       * ! p.from  #  merge    #   merge    # discard     #
       *           #           #            #             #
       * ##################################################
       *
       * The merge exits when:
       *   o ACCancel command is received in an ArtAddress packet
       *       (this is done in handle_address )
       *   o no data is recv'ed from one source in 10 seconds
       *
       */

      check_merge_timeouts(n,i);

      if (ipA == 0 && ipB == 0) {
        // first packet recv on this port
        port->ipA.s_addr = p->from.s_addr;
        port->timeA = time(NULL);

        memcpy(&port->dataA, &p->data.admx.data, data_length);
        port->length = data_length;
        memcpy(&port->data, &p->data.admx.data, data_length);
      }
      else if (ipA == p->from.s_addr && ipB == 0) {
        //continued transmission from the same ip (source A)

        port->timeA = time(NULL);
        memcpy(&port->dataA, &p->data.admx.data, data_length);
        port->length = data_length;
        memcpy(&port->data, &p->data.admx.data, data_length);
      }
      else if (ipA == 0 && ipB == p->from.s_addr) {
        //continued transmission from the same ip (source B)

        port->timeB = time(NULL);
        memcpy(&port->dataB, &p->data.admx.data, data_length);
        port->length = data_length;
        memcpy(&port->data, &p->data.admx.data, data_length);
      }
      else if (ipA != p->from.s_addr  && ipB == 0) {
        // new source, start the merge
        port->ipB.s_addr = p->from.s_addr;
        port->timeB = time(NULL);
        memcpy(&port->dataB, &p->data.admx.data,data_length);
        port->length = data_length;

        // merge, newest data is port B
        merge(n,i,data_length, port->dataB);

        // send reply if needed

      }
      else if (ipA == 0 && ipB == p->from.s_addr) {
        // new source, start the merge
        port->ipA.s_addr = p->from.s_addr;
        port->timeB = time(NULL);
        memcpy(&port->dataB, &p->data.admx.data,data_length);
        port->length = data_length;

        // merge, newest data is portA
        merge(n,i,data_length, port->dataA);

        // send reply if needed
      }
      else if (ipA == p->from.s_addr && ipB != p->from.s_addr) {
        // continue merge
        port->timeA = time(NULL);
        memcpy(&port->dataA, &p->data.admx.data,data_length);
        port->length = data_length;

        // merge, newest data is portA
        merge(n,i,data_length, port->dataA);

      }
      else if (ipA != p->from.s_addr && ipB == p->from.s_addr) {
        // continue merge
        port->timeB = time(NULL);
        memcpy(&port->dataB, &p->data.admx.data,data_length);
        port->length = data_length;

        // merge newest data is portB
        merge(n,i,data_length, port->dataB);

      }
      else if (ipA == p->from.s_addr && ipB == p->from.s_addr) {
//        err_warn("In handle_dmx, source matches both buffers, this shouldn't be happening!\n");

      }
      else if (ipA != p->from.s_addr && ipB != p->from.s_addr) {
//        err_warn("In handle_dmx, more than two sources, discarding data\n");

      }
      else {
//        err_warn("In handle_dmx, no cases matched, this shouldn't happen!\n");

      }

      // do the dmx callback here
      if (n->callbacks.dmx_c.fh != NULL)
        n->callbacks.dmx_c.fh(n,i, n->callbacks.dmx_c.data);
    }
  }
  return;
}


/**
 * handle art address packet.
 * This can reprogram certain nodes settings such as short/long name, port
 * addresses, subnet address etc.
 *
 */
int handle_address(node n, artnet_packet p) {
  int i, old_subnet;
  int addr[ARTNET_MAX_PORTS];
  int ret;

  if (check_callback(n, p, n->callbacks.address))
    return ARTNET_EOK;

  // servers (and raw nodes) don't respond to address packets
  if (n->state.node_type == ARTNET_SRV || n->state.node_type == ARTNET_RAW)
    return ARTNET_EOK;

  // reprogram shortname if required
  if (p->data.addr.shortname[0] != PROGRAM_DEFAULTS &&
      p->data.addr.shortname[0] != PROGRAM_NO_CHANGE) {
    memcpy(&n->state.short_name, &p->data.addr.shortname, ARTNET_SHORT_NAME_LENGTH);
    n->state.report_code = ARTNET_RCSHNAMEOK;
  }
  // reprogram long name if required
  if (p->data.addr.longname[0] != PROGRAM_DEFAULTS &&
      p->data.addr.longname[0] != PROGRAM_NO_CHANGE) {
    memcpy(&n->state.long_name, &p->data.addr.longname, ARTNET_LONG_NAME_LENGTH);
    n->state.report_code = ARTNET_RCLONAMEOK;
  }

  // first of all store existing port addresses
  // then we can work out if they change
  for (i=0; i< ARTNET_MAX_PORTS; i++) {
    addr[i] = n->ports.in[i].port_addr;
  }

  // program subnet
  old_subnet = p->data.addr.subnet;
  if (p->data.addr.subnet == PROGRAM_DEFAULTS) {
    // reset to defaults
    n->state.subnet = n->state.default_subnet;
    n->state.subnet_net_ctl = FALSE;

  } else if (p->data.addr.subnet & PROGRAM_CHANGE_MASK) {
    n->state.subnet = p->data.addr.subnet & ~PROGRAM_CHANGE_MASK;
    n->state.subnet_net_ctl = TRUE;
  }

  // check if subnet has actually changed
  if (old_subnet != n->state.subnet) {
    // if it does we need to change all port addresses
    for(i=0; i< ARTNET_MAX_PORTS; i++) {
      n->ports.in[i].port_addr = _make_addr(n->state.subnet, n->ports.in[i].port_addr);
      n->ports.out[i].port_addr = _make_addr(n->state.subnet, n->ports.out[i].port_addr);
    }
  }

  // program swins
  for (i =0; i < ARTNET_MAX_PORTS; i++) {
    if (p->data.addr.swin[i] == PROGRAM_NO_CHANGE)  {
      continue;
    } else if (p->data.addr.swin[i] == PROGRAM_DEFAULTS) {
      // reset to defaults
      n->ports.in[i].port_addr = _make_addr(n->state.subnet, n->ports.in[i].port_default_addr);
      n->ports.in[i].port_net_ctl = FALSE;

    } else if ( p->data.addr.swin[i] & PROGRAM_CHANGE_MASK) {
      n->ports.in[i].port_addr = _make_addr(n->state.subnet, p->data.addr.swin[i]);
      n->ports.in[i].port_net_ctl = TRUE;
    }
  }

  // program swouts
  for (i =0; i < ARTNET_MAX_PORTS; i++) {
    if (p->data.addr.swout[i] == PROGRAM_NO_CHANGE) {
      continue;
    } else if (p->data.addr.swout[i] == PROGRAM_DEFAULTS) {
      // reset to defaults
      n->ports.out[i].port_addr = _make_addr(n->state.subnet, n->ports.out[i].port_default_addr);
      n->ports.out[i].port_net_ctl = FALSE;
      n->ports.out[i].port_enabled = TRUE;
    } else if ( p->data.addr.swout[i] & PROGRAM_CHANGE_MASK) {
      n->ports.out[i].port_addr = _make_addr(n->state.subnet, p->data.addr.swout[i]);
      n->ports.in[i].port_net_ctl = TRUE;
      n->ports.out[i].port_enabled = TRUE;
    }
  }

  // reset sequence numbers if the addresses change
  for (i=0; i< ARTNET_MAX_PORTS; i++) {
    if (addr[i] != n->ports.in[i].port_addr)
      n->ports.in[i].seq = 0;
  }

  // check command
  switch (p->data.addr.command) {
    case ARTNET_PC_CANCEL:
      // fix me
      break;
    case ARTNET_PC_RESET:
      n->ports.out[0].port_status = n->ports.out[0].port_status & ~PORT_STATUS_DMX_SIP & ~PORT_STATUS_DMX_TEST & ~PORT_STATUS_DMX_TEXT;
      // need to force a rerun of short tests here
      break;
    case ARTNET_PC_MERGE_LTP_O:
      n->ports.out[0].merge_mode = ARTNET_MERGE_LTP;
      n->ports.out[0].port_status = n->ports.out[0].port_status | PORT_STATUS_LPT_MODE;
      break;
    case ARTNET_PC_MERGE_LTP_1:
      n->ports.out[1].merge_mode = ARTNET_MERGE_LTP;
      n->ports.out[1].port_status = n->ports.out[1].port_status | PORT_STATUS_LPT_MODE;
      break;
    case ARTNET_PC_MERGE_LTP_2:
      n->ports.out[2].merge_mode = ARTNET_MERGE_LTP;
      n->ports.out[2].port_status = n->ports.out[2].port_status | PORT_STATUS_LPT_MODE;
      break;
    case ARTNET_PC_MERGE_LTP_3:
      n->ports.out[3].merge_mode = ARTNET_MERGE_LTP;
      n->ports.out[3].port_status = n->ports.out[3].port_status | PORT_STATUS_LPT_MODE;
      break;
    case ARTNET_PC_MERGE_HTP_0:
      n->ports.out[0].merge_mode = ARTNET_MERGE_HTP;
      n->ports.out[0].port_status = n->ports.out[0].port_status | PORT_STATUS_LPT_MODE;
      break;
    case ARTNET_PC_MERGE_HTP_1:
      n->ports.out[1].merge_mode = ARTNET_MERGE_HTP;
      n->ports.out[1].port_status = n->ports.out[1].port_status | PORT_STATUS_LPT_MODE;
      break;
    case ARTNET_PC_MERGE_HTP_2:
      n->ports.out[2].merge_mode = ARTNET_MERGE_HTP;
      n->ports.out[2].port_status = n->ports.out[2].port_status | PORT_STATUS_LPT_MODE;
      break;
    case ARTNET_PC_MERGE_HTP_3:
      n->ports.out[3].merge_mode = ARTNET_MERGE_HTP;
      n->ports.out[3].port_status = n->ports.out[3].port_status | PORT_STATUS_LPT_MODE;
      break;

  }

  if (n->callbacks.program_c.fh != NULL)
    n->callbacks.program_c.fh(n , n->callbacks.program_c.data);

  if ((ret = artnet_tx_build_art_poll_reply(n)))
    return ret;

  return artnet_tx_poll_reply(n, TRUE);
}


/*
 * handle art input.
 * ArtInput packets can disable input ports.
 */
int _artnet_handle_input(node n, artnet_packet p) {
  int i, ports, ret;

  if (check_callback(n, p, n->callbacks.input))
    return ARTNET_EOK;

  // servers (and raw nodes) don't respond to input packets
  if (n->state.node_type != ARTNET_NODE && n->state.node_type != ARTNET_MSRV)
    return ARTNET_EOK;

  ports = min( p->data.ainput.numbports, ARTNET_MAX_PORTS);
  for (i =0; i < ports; i++) {
    if (p->data.ainput.input[i] & PORT_DISABLE_MASK) {
      // disable
      n->ports.in[i].port_status = n->ports.in[i].port_status | PORT_STATUS_DISABLED_MASK;
    } else {
      // enable
      n->ports.in[i].port_status = n->ports.in[i].port_status & ~PORT_STATUS_DISABLED_MASK;
    }
  }

  if ((ret = artnet_tx_build_art_poll_reply(n)))
    return ret;

  return artnet_tx_poll_reply(n, TRUE);
}


/***
 * handle tod request packet
 */
int handle_tod_request(node n, artnet_packet p) {
  int i, j, limit;
  int ret = ARTNET_EOK;

  if (check_callback(n, p, n->callbacks.todrequest))
    return ARTNET_EOK;

  if (n->state.node_type != ARTNET_NODE)
    return ARTNET_EOK;

  // limit to 32
  limit = min(ARTNET_MAX_RDM_ADCOUNT, p->data.todreq.adCount);

  // this should always be true
  if (p->data.todreq.command == 0x00) {
    for (i=0; i < limit; i++) {
      for (j=0; j < ARTNET_MAX_PORTS; j++) {
        if (n->ports.out[j].port_addr == p->data.todreq.address[i] &&
            n->ports.out[j].port_enabled) {
          // reply with tod
          ret = ret || artnet_tx_tod_data(n, j);
        }
      }
    }
  }

//  err_warn("tod request received but command is 0x%02hhx rather than 0x00\n", p->data.todreq.command);
  return ret;
}

/**
 * handle tod data packet
 *
 * we don't maintain a tod of whats out on the network,
 * the calling app can deal with this.
 */
void handle_tod_data(node n, artnet_packet p) {

  if (check_callback(n, p, n->callbacks.toddata))
    return;

  // pass data to app

//  if (n->callbacks.rdm_tod_c.fh != NULL)
//    n->callbacks.rdm_tod_c.fh(n, i, n->callbacks.rdm_tod_c.data);

  return;
}



int handle_tod_control(node n, artnet_packet p) {
  int i;
  int ret = ARTNET_EOK;

  if (check_callback(n, p, n->callbacks.todcontrol))
    return ARTNET_EOK;

  for (i=0; i < ARTNET_MAX_PORTS; i++) {
    if (n->ports.out[i].port_addr == p->data.todcontrol.address &&
        n->ports.out[i].port_enabled) {

      if (p->data.todcontrol.cmd == ARTNET_TOD_FLUSH) {
        // flush tod for this port
        flush_tod(&n->ports.out[i].port_tod);

        //initiate full rdm discovery
        // do callback here
        if (n->callbacks.rdm_init_c.fh != NULL)
          n->callbacks.rdm_init_c.fh(n, i, n->callbacks.rdm_init_c.data);

        // not really sure what to do here, the calling app should do a rdm
        // init and call artnet_add_rdm_devices() which will send a tod data
        // but do we really trust the caller ?
        // Instead we'll send an empty tod data and then another one a bit later
        // when our tod is populated
      }
      // reply with tod
      ret = ret || artnet_tx_tod_data(n, i);
    }
  }
  return ret;
}

/**
 * handle rdm packet
 *
 */
void handle_rdm(node n, artnet_packet p) {


  if (check_callback(n, p, n->callbacks.rdm))
    return;

  printf("rdm data\n");

  // hell dodgy
  if (n->callbacks.rdm_c.fh != NULL)
    n->callbacks.rdm_c.fh(n, p->data.rdm.address, p->data.rdm.data, ARTNET_MAX_RDM_DATA, n->callbacks.rdm_c.data);

  return;
}

/**
 * handle a firmware master
 */

// THIS NEEDS TO BE CHECKED FOR BUFFER OVERFLOWS
// IMPORTANT!!!!
int handle_firmware(node n, artnet_packet p) {
  int length, offset, block_length, total_blocks, block_id;
  artnet_firmware_status_code response_code = ARTNET_FIRMWARE_FAIL;

  // run callback if defined
  if (check_callback(n, p, n->callbacks.firmware))
    return ARTNET_EOK;

  /*
   * What happens if an upload is less than 512 bytes ?????
   */

  if ( p->data.firmware.type == ARTNET_FIRMWARE_FIRMFIRST ||
       p->data.firmware.type == ARTNET_FIRMWARE_UBEAFIRST) {
    // a new transfer is initiated

    if (n->firmware.peer.s_addr == 0) {
      //new transfer
      // these are 2 byte words, so we get a total of 1k of data per packet
      length = artnet_misc_nbytes_to_32( p->data.firmware.length ) *
        sizeof(p->data.firmware.data[0]);

      // set parameters
      n->firmware.peer.s_addr = p->from.s_addr;
      n->firmware.data = malloc(length);

      if (n->firmware.data  == NULL) {
        artnet_error_malloc();
        return ARTNET_EMEM;
      }
      n->firmware.bytes_total = length;
      n->firmware.last_time = time(NULL);
      n->firmware.expected_block = 1;

      // check if this is a ubea upload or not
      if (p->data.firmware.type == ARTNET_FIRMWARE_FIRMFIRST)
        n->firmware.ubea = 0;
      else
        n->firmware.ubea = 1;

      // take the minimum of the total length and the max packet size
      block_length = min((unsigned int) length, ARTNET_FIRMWARE_SIZE *
        sizeof(p->data.firmware.data[0]));

      memcpy(n->firmware.data, p->data.firmware.data, block_length);
      n->firmware.bytes_current = block_length;

      if (block_length == length) {
        // this is the first and last packet
        // upload was less than 1k bytes
        // this behaviour isn't in the spec, presumably no firmware will be less that 1k
        response_code = ARTNET_FIRMWARE_ALLGOOD;

        // do the callback here
        if (n->callbacks.firmware_c.fh != NULL)
          n->callbacks.firmware_c.fh(n,
                                     n->firmware.ubea,
                                     n->firmware.data,
                                     n->firmware.bytes_total,
                                     n->callbacks.firmware_c.data);

      } else {
        response_code = ARTNET_FIRMWARE_BLOCKGOOD;
      }

    } else {
      // already in a transfer
      printf("First, but already for a packet\n");

      // send a failure
      response_code = ARTNET_FIRMWARE_FAIL;
    }

  } else if (p->data.firmware.type == ARTNET_FIRMWARE_FIRMCONT ||
             p->data.firmware.type == ARTNET_FIRMWARE_UBEACONT) {
    // continued transfer
    length = artnet_misc_nbytes_to_32(p->data.firmware.length) *
      sizeof(p->data.firmware.data[0]);
    total_blocks = length / ARTNET_FIRMWARE_SIZE / 2 + 1;
    block_length = ARTNET_FIRMWARE_SIZE * sizeof(uint16_t);
    block_id = p->data.firmware.blockId;

    // ok the blockid field is only 1 byte, so it wraps back to 0x00 we
    // need to watch for this
    if (n->firmware.expected_block > UINT8_MAX &&
       (n->firmware.expected_block % (UINT8_MAX+1)) == p->data.firmware.blockId) {

      block_id = n->firmware.expected_block;
    }
    offset = block_id * ARTNET_FIRMWARE_SIZE;

    if (n->firmware.peer.s_addr == p->from.s_addr &&
        length == n->firmware.bytes_total &&
        block_id < total_blocks-1) {

      memcpy(n->firmware.data + offset, p->data.firmware.data, block_length);
      n->firmware.bytes_current += block_length;
      n->firmware.expected_block++;

      response_code = ARTNET_FIRMWARE_BLOCKGOOD;
    } else {
      printf("cont, ips don't match or length has changed or out of range block num\n" );

      // in a transfer not from this ip
      response_code = ARTNET_FIRMWARE_FAIL;
    }

  } else if (p->data.firmware.type == ARTNET_FIRMWARE_FIRMLAST ||
             p->data.firmware.type == ARTNET_FIRMWARE_UBEALAST) {
    length = artnet_misc_nbytes_to_32( p->data.firmware.length) *
      sizeof(p->data.firmware.data[0]);
    total_blocks = length / ARTNET_FIRMWARE_SIZE / 2 + 1;

    // length should be the remaining data
    block_length = n->firmware.bytes_total % (ARTNET_FIRMWARE_SIZE * sizeof(uint16_t));
    block_id = p->data.firmware.blockId;

    // ok the blockid field is only 1 byte, so it wraps back to 0x00 we
    // need to watch for this
    if (n->firmware.expected_block > UINT8_MAX &&
       (n->firmware.expected_block % (UINT8_MAX+1)) == p->data.firmware.blockId) {

      block_id = n->firmware.expected_block;
    }
    offset = block_id * ARTNET_FIRMWARE_SIZE;

    if (n->firmware.peer.s_addr == p->from.s_addr &&
        length == n->firmware.bytes_total &&
        block_id == total_blocks-1) {

      // all the checks work out
      memcpy(n->firmware.data + offset, p->data.firmware.data, block_length);
      n->firmware.bytes_current += block_length;

      // do the callback here
      if (n->callbacks.firmware_c.fh != NULL)
        n->callbacks.firmware_c.fh(n, n->firmware.ubea,
          n->firmware.data,
          n->firmware.bytes_total / sizeof(p->data.firmware.data[0]),
          n->callbacks.firmware_c.data);

      // reset values and free
      reset_firmware_upload(n);

      response_code = ARTNET_FIRMWARE_ALLGOOD;
      printf("Firmware upload complete\n");

    } else if (n->firmware.peer.s_addr != p->from.s_addr) {
      // in a transfer not from this ip
      printf("last, ips don't match\n" );
      response_code = ARTNET_FIRMWARE_FAIL;
    } else if (length != n->firmware.bytes_total) {
      // they changed the length mid way thru a transfer
      printf("last, lengths have changed %d %d\n", length, n->firmware.bytes_total);
      response_code = ARTNET_FIRMWARE_FAIL;
    } else if (block_id != total_blocks -1) {
      // the blocks don't match up
      printf("This is the last block, but not according to the lengths %d %d\n", block_id, total_blocks -1);
      response_code = ARTNET_FIRMWARE_FAIL;
    }
  }

  return artnet_tx_firmware_reply(n, p->from.s_addr, response_code);
}

/**
 * handle an firmware reply
 */
int handle_firmware_reply(node n, artnet_packet p) {
  node_entry_private_t *ent;

  // run callback if defined
  if (check_callback(n, p, n->callbacks.firmware_reply))
    return ARTNET_EOK;

  ent = find_entry_from_ip(&n->node_list, p->from);

  // node doesn't exist in our list, or we're not doing a transfer to this node
  if (ent== NULL || ent->firmware.bytes_total == 0)
    return ARTNET_EOK;

  // three types of response, ALLGOOD,  BLOCKGOOD and FIRMFAIL
  if (p->data.firmwarer.type == ARTNET_FIRMWARE_ALLGOOD) {

    if (ent->firmware.bytes_total == ent->firmware.bytes_current) {
      // transfer complete

      // do the callback
      if (ent->firmware.callback != NULL)
        ent->firmware.callback(n, ARTNET_FIRMWARE_ALLGOOD, ent->firmware.user_data);

      memset(&ent->firmware, 0x0, sizeof(firmware_transfer_t));

    } else {
      // random ALLGOOD received, don't let this abort the transfer
      printf("FIRMWARE_ALLGOOD received before transfer completed\n");
    }

  } else if (p->data.firmwarer.type == ARTNET_FIRMWARE_FAIL) {

    // do the callback
    if (ent->firmware.callback != NULL)
        ent->firmware.callback(n, ARTNET_FIRMWARE_FAIL, ent->firmware.user_data);

    // cancel transfer
    memset(&ent->firmware, 0x0, sizeof(firmware_transfer_t));

  } else if (p->data.firmwarer.type == ARTNET_FIRMWARE_BLOCKGOOD) {
    // send the next block (only if we're not done yet)
    if (ent->firmware.bytes_total != ent->firmware.bytes_current) {
      return artnet_tx_firmware_packet(n, &ent->firmware);
    }
  }
  return ARTNET_EOK;
}


/*
 * have to sort this one out.
 */
void handle_ipprog(node n, artnet_packet p) {

  if (check_callback(n, p, n->callbacks.ipprog))
    return;

  printf("in ipprog\n");
}


/*
 * The main handler for an artnet packet. calls
 * the appropriate handler function
 */
int handle(node n, artnet_packet p) {

  if (check_callback(n, p, n->callbacks.recv))
    return 0;

  switch (p->type) {
    case ARTNET_POLL:
      handle_poll(n, p);
      break;
    case ARTNET_REPLY:
      handle_reply(n,p);
      break;
    case ARTNET_DMX:
      handle_dmx(n, p);
      break;
    case ARTNET_ADDRESS:
      handle_address(n, p);
      break;
    case ARTNET_INPUT:
      _artnet_handle_input(n, p);
      break;
    case ARTNET_TODREQUEST:
      handle_tod_request(n, p);
      break;
    case ARTNET_TODDATA:
      handle_tod_data(n, p);
      break;
    case ARTNET_TODCONTROL:
      handle_tod_control(n, p);
      break;
    case ARTNET_RDM:
      handle_rdm(n, p);
      break;
    case ARTNET_VIDEOSTEUP:
      printf("vid setup\n");
      break;
    case ARTNET_VIDEOPALETTE:
      printf("video palette\n");
      break;
    case ARTNET_VIDEODATA:
      printf("video data\n");
      break;
    case ARTNET_MACMASTER:
      printf("mac master\n");
      break;
    case ARTNET_MACSLAVE:
      printf("mac slave\n");
      break;
    case ARTNET_FIRMWAREMASTER:
      handle_firmware(n, p);
      break;
    case ARTNET_FIRMWAREREPLY:
      handle_firmware_reply(n, p);
      break;
    case ARTNET_IPPROG :
      handle_ipprog(n, p);
      break;
    case ARTNET_IPREPLY:
      printf("ip reply\n");
      break;
    case ARTNET_MEDIA:
      printf("media \n");
      break;
    case ARTNET_MEDIAPATCH:
      printf("media patch\n");
      break;
    case ARTNET_MEDIACONTROLREPLY:
      printf("media control reply\n");
      break;
    default:
      n->state.report_code = ARTNET_RCPARSEFAIL;
      printf("artnet but not yet implemented!, op was %x\n", (int) p->type);
  }
  return 0;
}

/**
 * this gets the opcode from a packet
 */
int16_t get_type(artnet_packet p) {
  uint8_t *data;

  if (p->length < 10)
    return 0;
  if (!memcmp(&p->data, "Art-Net\0", 8)) {
    // not the best here, this needs to be tested on different arch
    data = (uint8_t *) &p->data;

    p->type = (data[9] << 8) + data[8];
    return p->type;
  } else {
    return 0;
  }
}


/*
 * takes a subnet and an address and creates the universe address
 */
uint8_t _make_addr(uint8_t subnet, uint8_t addr) {
  return ((subnet & LOW_NIBBLE) << 4) | (addr & LOW_NIBBLE);
}


/*
 *
 */
void check_merge_timeouts(node n, int port_id) {
  output_port_t *port;
  time_t now;
  time_t timeoutA, timeoutB;
  port = &n->ports.out[port_id];
  time(&now);
  timeoutA = now - port->timeA;
  timeoutB = now - port->timeB;

  if (timeoutA > MERGE_TIMEOUT_SECONDS) {
    // A is old, stop the merge
    port->ipA.s_addr = 0;
  }

  if (timeoutB > MERGE_TIMEOUT_SECONDS) {
    // B is old, stop the merge
    port->ipB.s_addr = 0;
  }
}


/*
 * merge the data from two sources
 */
void merge(node n, int port_id, int length, uint8_t *latest) {
  int i;
  output_port_t *port;
  port = &n->ports.out[port_id];

  if (port->merge_mode == ARTNET_MERGE_HTP) {
    for (i=0; i< length; i++)
      port->data[i] = max(port->dataA[i], port->dataB[i]);
  } else {
    memcpy(port->data, latest, length);
  }
}


void reset_firmware_upload(node n) {
  n->firmware.bytes_current = 0;
  n->firmware.bytes_total = 0;
  n->firmware.peer.s_addr = 0;
  n->firmware.ubea = 0;
  n->firmware.last_time = 0;
  free(n->firmware.data);
}
