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
 * transmit.c
 * Functions to handle sending datagrams
 * Copyright (C) 2004-2005 Simon Newton
 */

#include "private.h"

/*
 * Send an art poll
 *
 * @param ip the ip address to send to
 * @param ttm the talk to me value, either ARTNET_TTM_DEFAULT,
 *   ARTNET_TTM_PRIVATE or ARTNET_TTM_AUTO
 */
int artnet_tx_poll(node n, const char *ip, artnet_ttm_value_t ttm) {
  artnet_packet_t p;
  int ret;

  if (n->state.mode != ARTNET_ON)
    return ARTNET_EACTION;

  if (n->state.node_type == ARTNET_SRV || n->state.node_type == ARTNET_RAW) {
    if (ip) {
      ret = artnet_net_inet_aton(ip, &p.to);
      if (ret)
        return ret;
    } else {
      p.to.s_addr = n->state.bcast_addr.s_addr;
    }

    memcpy(&p.data.ap.id, ARTNET_STRING, ARTNET_STRING_SIZE);
    p.data.ap.opCode = htols(ARTNET_POLL);
    p.data.ap.verH = 0;
    p.data.ap.ver = ARTNET_VERSION;
    p.data.ap.ttm = ~ttm;
    p.data.ap.pad = 0;

    p.length = sizeof(artnet_poll_t);
    return artnet_net_send(n, &p);

  } else {
    artnet_error("Not sending poll, not a server or raw device");
    return ARTNET_EACTION;
  }
}

/*
 * Send an ArtPollReply
 * @param n the node
 * @param response true if this reply is in response to a network packet
 *            false if this reply is due to the node changing it's conditions
 */
int artnet_tx_poll_reply(node n, int response) {
  artnet_packet_t reply;
  int i;

  if (!response && n->state.mode == ARTNET_ON) {
    n->state.ar_count++;
  }

  reply.to = n->state.reply_addr;
  reply.type = ARTNET_REPLY;
  reply.length = sizeof(artnet_reply_t);

  // copy from a poll reply template
  memcpy(&reply.data, &n->ar_temp, sizeof(artnet_reply_t));

  for (i=0; i< ARTNET_MAX_PORTS; i++) {
    reply.data.ar.goodinput[i] = n->ports.in[i].port_status;
    reply.data.ar.goodoutput[i] = n->ports.out[i].port_status;
  }

  snprintf((char *) &reply.data.ar.nodereport,
           sizeof(reply.data.ar.nodereport),
           "%04x [%04i] libartnet",
           n->state.report_code,
           n->state.ar_count);

  return artnet_net_send(n, &reply);
}


/*
 * Send a tod request
 */
int artnet_tx_tod_request(node n) {
  int i;
  artnet_packet_t todreq;

  todreq.to = n->state.bcast_addr;
  todreq.type = ARTNET_TODREQUEST;
  todreq.length = sizeof(artnet_todrequest_t);
  memset(&todreq.data,0x00, todreq.length);

  // set up the data
  memcpy(&todreq.data.todreq.id, ARTNET_STRING, ARTNET_STRING_SIZE);
  todreq.data.todreq.opCode = htols(ARTNET_TODREQUEST);
  todreq.data.todreq.verH = 0;
  todreq.data.todreq.ver = ARTNET_VERSION;
  todreq.data.todreq.command = ARTNET_TOD_FULL; // todfull
  todreq.data.todreq.adCount = 0;

  // include all enabled ports
  for (i=0; i < ARTNET_MAX_PORTS; i++) {
    if (n->ports.out[i].port_enabled) {
      todreq.data.todreq.address[todreq.data.todreq.adCount++] = n->ports.out[i].port_addr;
    }
  }

  return artnet_net_send(n, &todreq);
}


/*
 * Send a tod data for port number id
 * @param id the number of the port to send data for
 */
int artnet_tx_tod_data(node n, int id) {
  artnet_packet_t tod;
  int lim, remaining, bloc, offset;
  int ret = ARTNET_EOK;

  // ok we need to check how many uid's we have,
  // may need to send more than one datagram

  tod.to = n->state.bcast_addr;
  tod.type = ARTNET_TODDATA;
  tod.length = sizeof(artnet_toddata_t);

  memset(&tod.data,0x00, tod.length);

  // set up the data
  memcpy(&tod.data.toddata.id, ARTNET_STRING, ARTNET_STRING_SIZE);
  tod.data.toddata.opCode = htols(ARTNET_TODDATA);
  tod.data.toddata.verH = 0;
  tod.data.toddata.ver = ARTNET_VERSION;
  tod.data.toddata.port = id;

  // this is interesting, the spec mentions TOD_ADD and TOD_SUBTRACT, but the
  // codes aren't given. The windows drivers don't have these either....
  tod.data.toddata.cmdRes = ARTNET_TOD_FULL;

  tod.data.toddata.address = n->ports.out[id].port_addr;
  tod.data.toddata.uidTotalHi = short_get_high_byte(n->ports.out[id].port_tod.length);
  tod.data.toddata.uidTotal = short_get_low_byte(n->ports.out[id].port_tod.length);

  remaining = n->ports.out[id].port_tod.length;
  bloc = 0;

  while (remaining > 0) {
    // NAP-local compile fix, from https://github.com/OpenLightingProject/libartnet/issues/13 
    memset(&tod.data.toddata.tod,0x00, ARTNET_MAX_UID_COUNT * ARTNET_RDM_UID_WIDTH);
    lim = min(ARTNET_MAX_UID_COUNT, remaining);
    tod.data.toddata.blockCount = bloc++;
    tod.data.toddata.uidCount = lim;

    offset = (n->ports.out[id].port_tod.length - remaining) * ARTNET_RDM_UID_WIDTH;
    if (n->ports.out[id].port_tod.data != NULL)
      memcpy(tod.data.toddata.tod,
             n->ports.out[id].port_tod.data + offset,
             lim * ARTNET_RDM_UID_WIDTH);

    ret = ret || artnet_net_send(n, &tod);
    remaining = remaining - lim;
  }
  return ret;
}


/*
 * Send a tod data for port number id
 * @param id the number of the port to send data for
 */
int artnet_tx_tod_control(node n,
                          uint8_t address,
                          artnet_tod_command_code action) {
  artnet_packet_t tod;

  tod.to = n->state.bcast_addr;
  tod.type = ARTNET_TODCONTROL;
  tod.length = sizeof(artnet_todcontrol_t);

  memset(&tod.data,0x00, tod.length);

  // set up the data
  memcpy(&tod.data.todcontrol.id, ARTNET_STRING, ARTNET_STRING_SIZE);
  tod.data.todcontrol.opCode = htols(ARTNET_TODCONTROL);
  tod.data.todcontrol.verH = 0;
  tod.data.todcontrol.ver = ARTNET_VERSION;
  tod.data.todcontrol.cmd = action;
  tod.data.todcontrol.address = address;

  return artnet_net_send(n, &tod);
}


/*
 * Send a RDM message
 * @param address the universe to address this datagram to
 * @param action the action to perform. Either ARTNET_TOD_FULL or
 *   ARTNET_TOD_FLUSH
 */
int artnet_tx_rdm(node n, uint8_t address, uint8_t *data, int length) {
  artnet_packet_t rdm;
  int len;

  rdm.to = n->state.bcast_addr;
  rdm.type = ARTNET_RDM;
  rdm.length = sizeof(artnet_rdm_t);

  memset(&rdm.data,0x00, rdm.length);

  // set up the data
  memcpy(&rdm.data.todcontrol.id, ARTNET_STRING, ARTNET_STRING_SIZE);
  rdm.data.rdm.opCode = htols(ARTNET_RDM);
  rdm.data.rdm.verH = 0;
  rdm.data.rdm.ver = ARTNET_VERSION;
  rdm.data.rdm.cmd = 0x00;
  rdm.data.rdm.address = address;

  len = min(length, ARTNET_MAX_RDM_DATA);
  memcpy(&rdm.data.rdm.data, data, len);
  return artnet_net_send(n, &rdm);

}


/*
 * Send a firmware reply
 * @param ip the ip address to send to
 * @param code the response code
 */
int artnet_tx_firmware_reply(node n, in_addr_t ip,
                             artnet_firmware_status_code code) {
  artnet_packet_t p;
  memset(&p, 0x0, sizeof(p));

  p.to.s_addr = ip;
  p.length = sizeof(artnet_firmware_t);
  p.type = ARTNET_FIRMWAREREPLY;

  // now build packet
  memcpy(&p.data.firmware.id, ARTNET_STRING, ARTNET_STRING_SIZE);
  p.data.firmware.opCode = htols(ARTNET_FIRMWAREREPLY);
  p.data.firmware.verH = 0;
  p.data.firmware.ver = ARTNET_VERSION;
  p.data.firmware.type = code;

  return artnet_net_send(n, &p);
}


/*
 * Send an firmware data datagram
 *
 * @param firm a pointer to the firmware structure for this transfer
 */
int artnet_tx_firmware_packet(node n, firmware_transfer_t *firm) {
  artnet_packet_t p;
  uint8_t type = 0;
  int data_len, max_len, ret;

  memset(&p, 0x0, sizeof(p));

  // max value of data_len is 1024;
  max_len = ARTNET_FIRMWARE_SIZE * sizeof(p.data.firmware.data[0]);

  // calculate length
  data_len = firm->bytes_total - firm->bytes_current;
  data_len = min(data_len, max_len);

  // work out type - 6 cases
  if(firm->ubea) {
    // ubea upload
    if (firm->bytes_current == 0) {
      // first
      type = ARTNET_FIRMWARE_UBEAFIRST;
    } else if (data_len == max_len) {
      // cont
      type = ARTNET_FIRMWARE_UBEACONT;
    } else if (data_len < max_len) {
      // last
      type = ARTNET_FIRMWARE_UBEALAST;
    } else {
      // this should never happen, something has gone wrong
      artnet_error("Attempting to send %d when the max is %d, very very bad...\n", data_len, max_len);
    }
  } else {
    // firmware upload
    if (firm->bytes_current == 0) {
      // first
      type = ARTNET_FIRMWARE_FIRMFIRST;
    } else if (data_len == max_len) {
      // cont
      type = ARTNET_FIRMWARE_FIRMCONT;
    } else if (data_len < max_len) {
      // last
      type = ARTNET_FIRMWARE_FIRMLAST;
    } else {
      // this should never happen, something has gone wrong
      artnet_error("Attempting to send %d when the max is %d, very very bad...\n", data_len, max_len);
    }
  }

  // set packet properties
  p.to.s_addr = firm->peer.s_addr;
  p.length = sizeof(artnet_firmware_t);
  p.type = ARTNET_FIRMWAREMASTER;

  // now build packet
  memcpy(&p.data.firmware.id, ARTNET_STRING, ARTNET_STRING_SIZE);
  p.data.firmware.opCode = htols(ARTNET_FIRMWAREMASTER);
  p.data.firmware.verH = 0;
  p.data.firmware.ver = ARTNET_VERSION;
  p.data.firmware.type =  type;
  p.data.firmware.blockId = firm->expected_block;

  artnet_misc_int_to_bytes(firm->bytes_total / sizeof(uint16_t),
                           p.data.firmware.length);

  memcpy(&p.data.firmware.data,
         firm->data + (firm->bytes_current / sizeof(uint16_t)),
         data_len);

  if ((ret = artnet_net_send(n, &p))) {
    // send failed
    return ret;
  } else {
    // update stats
    firm->bytes_current = firm->bytes_current + data_len;
    firm->last_time = time(NULL);
    firm->expected_block++;
    // limit between 0 and 255 (only 8 bits wide)
    // we dont' actually need this cause it will be shorted when assigned above
    firm->expected_block %= UINT8_MAX;
  }
  return ARTNET_EOK;
}


// this is called when the node's state changes to rebuild the
// artpollreply packet
int artnet_tx_build_art_poll_reply(node n) {
  int i;

  // shorten the amount we have to type
  artnet_reply_t *ar = &n->ar_temp;

  memset(ar, 0x00, sizeof(artnet_reply_t));

  memcpy(&ar->id, ARTNET_STRING, ARTNET_STRING_SIZE);
  ar->opCode = htols(ARTNET_REPLY);
  memcpy(&ar->ip, &n->state.ip_addr.s_addr, 4);
  ar->port = htols(ARTNET_PORT);
  ar->verH = 0;
  ar->ver = 0;
  ar->subH = 0;
  ar->sub = n->state.subnet;
  ar->oemH = n->state.oem_hi;
  ar->oem = n->state.oem_lo;
  ar->ubea = 0;
  // ar->status

//  if(n->state

  // status need to be recalc everytime
  //ar->status

  // ESTA Manufacturer ID
  // Assigned 18/4/2006
  ar->etsaman[0] = n->state.esta_hi;
  ar->etsaman[1] = n->state.esta_lo;

  memcpy(&ar->shortname, &n->state.short_name, sizeof(n->state.short_name));
  memcpy(&ar->longname, &n->state.long_name, sizeof(n->state.long_name));

  // the report is generated on every send

  // port stuff here
  ar->numbportsH = 0;

  for (i = ARTNET_MAX_PORTS; i > 0; i--) {
    if (n->ports.out[i-1].port_enabled || n->ports.in[i-1].port_enabled)
      break;
  }

  ar->numbports = i;

  for (i=0; i< ARTNET_MAX_PORTS; i++) {
    ar->porttypes[i] = n->ports.types[i];
    ar->goodinput[i] = n->ports.in[i].port_status;
    ar->goodoutput[i] = n->ports.out[i].port_status;
    ar->swin[i] = n->ports.in[i].port_addr;
    ar->swout[i] = n->ports.out[i].port_addr;
  }

  ar->swvideo  = 0;
  ar->swmacro = 0;
  ar->swremote = 0;

  // spares
  ar->sp1 = 0;
  ar->sp2 = 0;
  ar->sp3 = 0;

  // hw address
  memcpy(&ar->mac, &n->state.hw_addr, ARTNET_MAC_SIZE);

  // set style
  switch (n->state.node_type) {
    case ARTNET_SRV:
      ar->style = STSERVER;
      break;
    case ARTNET_NODE:
      ar->style = STNODE;
      break;
    case ARTNET_MSRV:
      ar->style = STMEDIA;
      break;
    // we should fix this, it'll do for now
    case ARTNET_RAW:
      ar->style = STNODE;
      break;
    default:
      artnet_error("Node type not recognised!");
      ar->style = STNODE;
      return ARTNET_ESTATE;
  }

  return ARTNET_EOK;
}
