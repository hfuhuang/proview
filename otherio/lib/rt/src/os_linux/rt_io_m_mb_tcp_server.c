/* 
 * Proview   $Id$
 * Copyright (C) 2005 SSAB Oxel�sund AB.
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License as 
 * published by the Free Software Foundation, either version 2 of 
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the 
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with the program, if not, write to the Free Software 
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/* rt_io_m_mb_tcp_server.c -- io methods for Modbus/TCP Server */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <pthread.h>

#include "pwr.h"
#include "co_cdh.h"
#include "pwr_baseclasses.h"
#include "pwr_basecomponentclasses.h"
#include "pwr_otherioclasses.h"
#include "pwr_version.h"
#include "rt_gdh.h"
#include "rt_io_base.h"
#include "rt_io_bus.h"
#include "rt_thread.h"
#include "rt_io_msg.h"
#include "rt_errh.h"
#include "rt_thread.h"
#include "co_cdh.h"
#include "co_time.h"
#include "rt_mb_msg.h"

#include "rt_io_mb_locals.h"

char rcv_buffer[65536];
static pwr_tStatus mb_init_channels( io_tCtx ctx, io_sAgent *ap, io_sRack *rp);
static void mb_shift_write( unsigned char *in, unsigned char *out, int sh, int quant);
static void mb_shift_read( unsigned char *in, unsigned char *out, int sh, int quant);

typedef struct {
  io_sRack *rp;
  int idx;
} mb_sCondata;

static void mb_close_connection( io_sRack *rp, int l_idx) 
{
  pwr_sClass_Modbus_TCP_Server *op = (pwr_sClass_Modbus_TCP_Server *) rp->op;
  io_sServerLocal* local = rp->Local;
  pwr_tStatus sts;

  sts = thread_Cancel( &local->connections[l_idx].t);
  close( local->connections[l_idx].c_socket);
  local->connections[l_idx].occupied = 0;
  op->Connections--;
}

static void *mb_receive( void *data)
{
  io_sRack *rp = ((mb_sCondata *)data)->rp;
  int l_idx = ((mb_sCondata *)data)->idx;
  io_sServerLocal* local = rp->Local;
  int c_socket = local->connections[l_idx].c_socket;
  pwr_sClass_Modbus_TCP_Server *op = (pwr_sClass_Modbus_TCP_Server *) rp->op;
  ssize_t data_size;
  rec_buf *rb;
  unsigned char fc;
  unsigned char exception_code;
  ssize_t ssts;
  int size_of_msg;

  free( data);
  op->Connections++;

  while ( 1) {
    size_of_msg = 0;

    data_size = recv(c_socket, rcv_buffer, sizeof(rec_buf), 0);
    if ( data_size < 0) {
      op->ErrorCount++;
      continue;
    }
    if ( data_size == 0) {
      /* Disconnected */
      op->Connections--;
      close( c_socket);
      local->connections[l_idx].occupied = 0;
      errh_Error( "Connection lost for IO modbus tcp server %s, %d", rp->Name, c_socket);
      return 0;
    }
    if ( op->DisableServer)
      continue;

    while ( data_size > 0) {

      if (data_size < sizeof(mbap_header))
	break;

      op->RX_packets++;

      rb = (rec_buf *) &rcv_buffer[size_of_msg];

      if ( rb->head.length == 0)
	break;

      size_of_msg += ntohs(rb->head.length) + 6;
      data_size -= ntohs(rb->head.length) + 6;

      fc = (unsigned char) *rb->buf;

      time_GetTime( &local->connections[l_idx].last_req_time);
      exception_code = 0;

      switch ( fc) {
      case pwr_eModbus_FCEnum_ReadHoldingRegisters: {
	io_sCard *cardp;
	io_sServerModuleLocal *local_card;
	pwr_sClass_Modbus_TCP_ServerModule *mp;
	read_req *rmsg = (read_req *)rb;
	rsp_read msg;
	int found;

	short addr = ntohs( rmsg->addr);
	short quant = ntohs( rmsg->quant);
	unsigned char unit_id = rmsg->head.unit_id;

	if ( quant < 1 || quant >= 0x07d0) {
	  exception_code = 3;
	  break;
	}

	/* Check the address */
	found = 0;
	for ( cardp = rp->cardlist; cardp; cardp = cardp->next) {
	  mp = (pwr_sClass_Modbus_TCP_ServerModule *) cardp->op;
	  if ( mp->UnitId == unit_id) {
	    local_card = cardp->Local;
	    found = 1;
	    break;
	  }	
	}

	if ( !found) {
	  exception_code = 2;
	  break;
	}

	addr -= mp->ReadAddress;

	if ( addr < 0 || 
	     addr + quant * 2 > local_card->output_size) {
	  exception_code = 2;
	  break;
	}

	msg.fc = fc;
	msg.bc = quant * 2;
	msg.head.trans_id = rmsg->head.trans_id;
	// msg.head.length = htons( sizeof(msg) - 6);
	msg.head.length = htons( sizeof(msg) - sizeof(msg.buf) + quant * 2 - 6);
	msg.head.unit_id = rmsg->head.unit_id;
	msg.head.proto_id = rmsg->head.proto_id;

	thread_MutexLock( &local->mutex);
	memcpy( msg.buf, (char *)local_card->output_area + addr, quant * 2);
	thread_MutexUnlock( &local->mutex);

	ssts = send( c_socket, &msg, ntohs(msg.head.length) + 6, MSG_DONTWAIT);
	if (ssts < 0) {
	  op->Connections--;
	  close(c_socket);
	  local->connections[l_idx].occupied = 0;
	  errh_Error( "Connection lost for IO modbus tcp server %s, %d", rp->Name, c_socket);
	  return 0;
	}
	op->TX_packets++;      
      
	break;
      }
      case pwr_eModbus_FCEnum_ReadCoils:
      case pwr_eModbus_FCEnum_ReadDiscreteInputs: {
	io_sCard *cardp;
	io_sServerModuleLocal *local_card;
	pwr_sClass_Modbus_TCP_ServerModule *mp;
	read_req *rmsg = (read_req *)rb;
	rsp_read msg;
	int found;
	unsigned char mask;
	unsigned int bytes;
	int i;
	int offs;

	short addr = ntohs( rmsg->addr);
	short quant = ntohs( rmsg->quant);
	unsigned char unit_id = rmsg->head.unit_id;

	if ( quant < 1 || quant >= 0x07d0) {
	  exception_code = 3;
	  break;
	}

	/* Check the address */
	found = 0;
	for ( cardp = rp->cardlist; cardp; cardp = cardp->next) {
	  mp = (pwr_sClass_Modbus_TCP_ServerModule *) cardp->op;
	  if ( mp->UnitId == unit_id) {
	    local_card = cardp->Local;
	    found = 1;
	    break;
	  }	
	}

	if ( !found) {
	  exception_code = 2;
	  break;
	}

	offs = addr / 8;
	bytes = (addr + quant) / 8 +  (((addr + quant) % 8 == 0) ? 0 : 1) - offs;

	if ( addr < 0 || 
	     offs + bytes + local_card->do_offset > local_card->output_size || 
	     offs + bytes > local_card->do_size) {
	  exception_code = 2;
	  break;
	}

	memset( &msg, 0, sizeof(msg));
	msg.fc = fc;
	msg.bc = bytes;
	msg.head.trans_id = rmsg->head.trans_id;
	// msg.head.length = htons( sizeof(msg) - 6);
	msg.head.length = htons( sizeof(msg) - sizeof(msg.buf) + bytes - 6);
	msg.head.unit_id = rmsg->head.unit_id;
	msg.head.proto_id = rmsg->head.proto_id;

	thread_MutexLock( &local->mutex);
	if ( addr % 8 == 0) {
	  memcpy( msg.buf, (char *)local_card->output_area + local_card->do_offset + addr/8, bytes);      

	  mask = 0;
	  for ( i = 0; i < quant % 8; i++)
	    mask |= 1 << i;

	  if ( quant % 8 != 0) {
	    unsigned char *b = (unsigned char *) msg.buf;
	    b[bytes - 1] &= mask;
	  }
	}
	else {
	  mb_shift_read( (unsigned char *)local_card->output_area + local_card->do_offset + addr / 8,
			 (unsigned char *)msg.buf,
			 addr % 8, quant);
	}
	thread_MutexUnlock( &local->mutex);

	ssts = send( c_socket, &msg, ntohs(msg.head.length) + 6, MSG_DONTWAIT);
	if (ssts < 0) {
	  op->Connections--;
	  close(c_socket);
	  local->connections[l_idx].occupied = 0;
	  errh_Error( "Connection lost for IO modbus tcp server %s, %d", rp->Name, c_socket);
	  return 0;
	}
	op->TX_packets++;      
      
	break;
      }
      case pwr_eModbus_FCEnum_WriteSingleRegister: {
	io_sCard *cardp;
	io_sServerModuleLocal *local_card;
	pwr_sClass_Modbus_TCP_ServerModule *mp;
	write_single_req *rmsg = (write_single_req *)rb;
	rsp_single_write msg;
	int found;

	short addr = ntohs( rmsg->addr);
	unsigned char unit_id = rmsg->head.unit_id;

	/* Check the address */
	found = 0;
	for ( cardp = rp->cardlist; cardp; cardp = cardp->next) {
	  mp = (pwr_sClass_Modbus_TCP_ServerModule *) cardp->op;
	  if ( mp->UnitId == unit_id) {
	    local_card = cardp->Local;
	    found = 1;
	    break;
	  }	
	}

	if ( !found) {
	  exception_code = 2;
	  break;
	}

	addr -= mp->WriteAddress;

	if ( addr < 0 || 
	     addr + 2 > local_card->input_size) {
	  exception_code = 2;
	  break;
	}

	thread_MutexLock( &local->mutex);
	memcpy( (char *)local_card->input_area + addr, &rmsg->value, 2);
	thread_MutexUnlock( &local->mutex);

	msg.fc = fc;
	msg.addr = rmsg->addr;
	msg.value = rmsg->value;
	msg.head.trans_id = rmsg->head.trans_id;
	msg.head.length = htons( sizeof(msg) - 6);
	msg.head.unit_id = rmsg->head.unit_id;
	msg.head.proto_id = rmsg->head.proto_id;

	ssts = send( c_socket, &msg, sizeof(msg), MSG_DONTWAIT);
	if (ssts < 0) {
	  op->Connections--;
	  close(c_socket);
	  local->connections[l_idx].occupied = 0;
	  errh_Error( "Connection lost for IO modbus tcp server %s, %d", rp->Name, c_socket);
	  return 0;
	}
	op->TX_packets++;      
      
	break;
      }
      case pwr_eModbus_FCEnum_WriteMultipleRegisters: {
	io_sCard *cardp;
	io_sServerModuleLocal *local_card;
	pwr_sClass_Modbus_TCP_ServerModule *mp;
	write_reg_req *rmsg = (write_reg_req *)rb;
	rsp_write msg;
	int found;

	short addr = ntohs( rmsg->addr);
	short quant = ntohs( rmsg->quant);
	unsigned char unit_id = rmsg->head.unit_id;

	if ( quant < 1 || quant >= 0x07d0) {
	  exception_code = 3;
	  break;
	}

	/* Check the address */
	found = 0;
	for ( cardp = rp->cardlist; cardp; cardp = cardp->next) {
	  mp = (pwr_sClass_Modbus_TCP_ServerModule *) cardp->op;
	  if ( mp->UnitId == unit_id) {
	    local_card = cardp->Local;
	    found = 1;
	    break;
	  }	
	}

	if ( !found) {
	  exception_code = 2;
	  break;
	}

	addr -= mp->WriteAddress;

	if ( addr < 0 || 
	     addr + quant * 2 > local_card->input_size) {
	  exception_code = 2;
	  break;
	}

	thread_MutexLock( &local->mutex);
	memcpy( (char *)local_card->input_area + addr, rmsg->reg, quant * 2);
	thread_MutexUnlock( &local->mutex);

	msg.fc = fc;
	msg.addr = rmsg->addr;
	msg.quant = rmsg->quant;
	msg.head.trans_id = rmsg->head.trans_id;
	msg.head.length = htons( sizeof(msg) - 6);
	msg.head.unit_id = rmsg->head.unit_id;
	msg.head.proto_id = rmsg->head.proto_id;

	ssts = send( c_socket, &msg, sizeof(msg), MSG_DONTWAIT);
	if (ssts < 0) {
	  op->Connections--;
	  close(c_socket);
	  local->connections[l_idx].occupied = 0;
	  errh_Error( "Connection lost for IO modbus tcp server %s, %d", rp->Name, c_socket);
	  return 0;
	}
	op->TX_packets++;      
      
	break;
      }
      case pwr_eModbus_FCEnum_WriteSingleCoil: {
	io_sCard *cardp;
	io_sServerModuleLocal *local_card;
	pwr_sClass_Modbus_TCP_ServerModule *mp;
	write_single_req *rmsg = (write_single_req *)rb;
	rsp_single_write msg;
	int found;
	unsigned char mask;
	int offs;

	short addr = ntohs( rmsg->addr);
	short value = ntohs( rmsg->value);
	unsigned char unit_id = rmsg->head.unit_id;

	/* Check the address */
	found = 0;
	for ( cardp = rp->cardlist; cardp; cardp = cardp->next) {
	  mp = (pwr_sClass_Modbus_TCP_ServerModule *) cardp->op;
	  if ( mp->UnitId == unit_id) {
	    local_card = cardp->Local;
	    found = 1;
	    break;
	  }	
	}

	if ( !found) {
	  exception_code = 2;
	  break;
	}

	offs = addr / 8;

	if ( addr < 0 || 
	     offs + local_card->di_offset >= local_card->input_size || 
	     offs >= local_card->di_size) {
	  exception_code = 2;
	  break;
	}

	mask = 1 << (addr % 8);
	thread_MutexLock( &local->mutex);
	if ( value)
	  *((char *)local_card->input_area + local_card->di_offset + offs) |= mask;
	else
	  *((char *)local_card->input_area + local_card->di_offset + offs) &= ~mask;
	thread_MutexUnlock( &local->mutex);

	msg.fc = fc;
	msg.addr = rmsg->addr;
	msg.value = rmsg->value;
	msg.head.trans_id = rmsg->head.trans_id;
	msg.head.length = htons( sizeof(msg) - 6);
	msg.head.unit_id = rmsg->head.unit_id;
	msg.head.proto_id = rmsg->head.proto_id;

	ssts = send( c_socket, &msg, sizeof(msg), MSG_DONTWAIT);
	if (ssts < 0) {
	  op->Connections--;
	  close(c_socket);
	  local->connections[l_idx].occupied = 0;
	  errh_Error( "Connection lost for IO modbus tcp server %s, %d", rp->Name, c_socket);
	  return 0;
	}
	op->TX_packets++;      
      
	break;
      }
      case pwr_eModbus_FCEnum_WriteMultipleCoils: {
	io_sCard *cardp;
	io_sServerModuleLocal *local_card;
	pwr_sClass_Modbus_TCP_ServerModule *mp;
	write_reg_req *rmsg = (write_reg_req *)rb;
	rsp_write msg;
	int found;
	unsigned char mask;
	unsigned int bytes;
	int i;
	int offs;

	short addr = ntohs( rmsg->addr);
	short quant = ntohs( rmsg->quant);
	unsigned char unit_id = rmsg->head.unit_id;

	if ( quant < 1 || quant >= 0x07d0) {
	  exception_code = 3;
	  break;
	}

	/* Check the address */
	found = 0;
	for ( cardp = rp->cardlist; cardp; cardp = cardp->next) {
	  mp = (pwr_sClass_Modbus_TCP_ServerModule *) cardp->op;
	  if ( mp->UnitId == unit_id) {
	    local_card = cardp->Local;
	    found = 1;
	    break;
	  }	
	}

	if ( !found) {
	  exception_code = 2;
	  break;
	}

	offs = addr / 8;
	bytes = (addr + quant) / 8 +  (((addr + quant) % 8 == 0) ? 0 : 1) - offs;

	if ( addr < 0 || 
	     offs + bytes + local_card->di_offset > local_card->input_size || 
	     offs + bytes > local_card->di_size) {
	  exception_code = 2;
	  break;
	}

	thread_MutexLock( &local->mutex);
	if ( addr % 8 == 0) {
	  if ( quant % 8 != 0) {
	    mask = 0;
	    for ( i = 0; i < quant % 8; i++)
	      mask |= 1 << i;
	  
	    memcpy( (char *)local_card->input_area + local_card->di_offset + addr / 8, 
		    rmsg->reg, bytes - 1);
	    *((char *)local_card->input_area + local_card->di_offset + addr / 8 + bytes - 1) &= ~mask;
	    *((char *)local_card->input_area + local_card->di_offset + addr / 8 + bytes - 1) |= *((char *)rmsg->reg + bytes - 1) & mask;
	  }
	  else
	    memcpy( (char *)local_card->input_area + local_card->di_offset + addr / 8, 
		    rmsg->reg, bytes);
	}
	else {
	  mb_shift_write( (unsigned char *)rmsg->reg, 
			  (unsigned char *)local_card->input_area + local_card->di_offset + addr / 8,
			  addr % 8, quant);
	}
	thread_MutexUnlock( &local->mutex);
	
	msg.fc = fc;
	msg.addr = rmsg->addr;
	msg.quant = rmsg->quant;
	msg.head.trans_id = rmsg->head.trans_id;
	msg.head.length = htons( sizeof(msg) - 6);
	msg.head.unit_id = rmsg->head.unit_id;
	msg.head.proto_id = rmsg->head.proto_id;

	ssts = send( c_socket, &msg, sizeof(msg), MSG_DONTWAIT);
	if (ssts < 0) {
	  op->Connections--;
	  close(c_socket);
	  local->connections[l_idx].occupied = 0;
	  errh_Error( "Connection lost for IO modbus tcp server %s, %d", rp->Name, c_socket);
	  return 0;
	}
	op->TX_packets++;      
      
	break;
      }
      case 43: {
	/* Encapsulated Interface Transport, Read Device Identification */
	read_dev_id_req *rmsg = (read_dev_id_req *)rb;
	rsp_dev_id msg;
	int i;
	int len;

	if ( rmsg->mei_type != 0x2b) {
	  exception_code = 1;
	  break;
	}

	if ( rmsg->id_code != 1) {
	  exception_code = 1;
	  break;
	}

	if ( rmsg->object_id != 0) {
	  exception_code = 1;
	  break;
	}

	msg.fc = rmsg->fc;
	msg.mei_type = rmsg->mei_type;
	msg.id_code = rmsg->id_code;
	msg.conformity_level = 1;
	msg.more_follows = 0;
	msg.next_object_id = 0;
	msg.number_of_objects = 3;

	i = 0;

	/* Vendor name */
	msg.list[i++] = 0;
	len = strlen("Proview");
	msg.list[i++] = len;
	strncpy( (char *)&msg.list[i], "Proview", len);
	i += len;

	/* Product code */
	msg.list[i++] = 0;
	len = strlen("-");
	msg.list[i++] = len;
	strncpy( (char *)&msg.list[i], "-", len);
	i += len;

	/* Major Minor Revision */
	msg.list[i++] = 0;
	len = strlen(pwrv_cPwrVersionStr);
	msg.list[i++] = len;
	strncpy( (char *)&msg.list[i], pwrv_cPwrVersionStr, len);
	i += len;

	msg.head.trans_id = rmsg->head.trans_id;
	msg.head.length = htons( sizeof(msg) - sizeof(msg.list) + i - 6);
	msg.head.unit_id = rmsg->head.unit_id;
	msg.head.proto_id = rmsg->head.proto_id;
	
	ssts = send( c_socket, &msg, ntohs(msg.head.length) + 6, MSG_DONTWAIT);
	if (ssts < 0) {
	  op->Connections--;
	  close(c_socket);
	  local->connections[l_idx].occupied = 0;
	  errh_Error( "Connection lost for IO modbus tcp server %s, %d", rp->Name, c_socket);
	  return 0;
	}
	op->TX_packets++;      
      
	break;
      }
      default:
	exception_code = 1;
      }

      if ( exception_code) {
	rsp_fault rsp_f;

	rsp_f.fc = fc + 0x80;
	rsp_f.ec = exception_code;
	rsp_f.head.trans_id = rb->head.trans_id;
	rsp_f.head.length = htons( sizeof(rsp_f) - 6);
	rsp_f.head.unit_id = rb->head.unit_id;

	ssts = send( c_socket, &rsp_f, sizeof(rsp_f), MSG_DONTWAIT);
	if (ssts < 0) {
	  op->Connections--;
	  close(c_socket);
	  local->connections[l_idx].occupied = 0;
	  errh_Error( "Connection lost for IO modbus tcp server %s, %d", rp->Name, c_socket);
	  return 0;
	}
	op->TX_packets++;
      }
    }    
  }
  return 0;
}

static void *mb_connect( void *arg)
{
  io_sRack *rp = (io_sRack *)arg;
  io_sServerLocal* local = rp->Local;
  int sts;
  pwr_sClass_Modbus_TCP_Server *op;
  struct sockaddr_in r_addr;
  socklen_t r_addr_len;
  int c_socket;
  mb_sCondata *condata;
  int idx;
  int found;
  int i;
  
  op = (pwr_sClass_Modbus_TCP_Server *) rp->op;


  while ( 1) {
    /* Wait for client connect request */
    r_addr_len = sizeof(r_addr);

    c_socket = accept( local->s, (struct sockaddr *) &r_addr, &r_addr_len);
    if ( c_socket < 0) {
      errh_Error( "Error accept IO modbus tcp server %s, %d", rp->Name, local->s);
      continue;
    }
    if ( op->DisableServer)
      continue;

    errh_Info( "Connection accepted for IO modbus tcp server %s, %d", rp->Name, c_socket);

    /* Close other connections to this address */
    for ( i = 0; i < MB_MAX_CONNECTIONS; i++) {
      if ( local->connections[i].occupied && 
	   r_addr_len == local->connections[i].addrlen &&
	   r_addr.sin_family == local->connections[i].addr.sin_family &&
	   memcmp( &r_addr.sin_addr, &local->connections[i].addr.sin_addr, sizeof(r_addr.sin_addr)) == 0) {
	mb_close_connection( rp, i);
      }
    }
    

    /* Find next empty in connection list */
    found = 0;
    for ( i = 0; i < MB_MAX_CONNECTIONS; i++) {
      if ( !local->connections[i].occupied) {
	found = 1;
	idx = i;
	break;
      }
    }

    if ( !found) {
      /* Remove the oldest connection */
      int oldest_idx = 0;

      for ( i = 1; i < MB_MAX_CONNECTIONS; i++) {
	if ( time_Acomp( &local->connections[i].last_req_time, &local->connections[oldest_idx].last_req_time) < 0)
	  oldest_idx = i;
      }
      mb_close_connection( rp, oldest_idx);
      errh_Info( "Connection closed, IO modbus tcp server %s, %d", rp->Name, local->s);
      idx = oldest_idx;
    }

    local->connections[idx].c_socket = c_socket;
    local->connections[idx].occupied = 1;
    time_GetTime( &local->connections[idx].last_req_time);
    local->connections[idx].addrlen = r_addr_len;
    memcpy( &local->connections[idx].addr, &r_addr, r_addr_len);

    /* Create a thread for this connection */
    condata = (mb_sCondata *) malloc( sizeof(mb_sCondata));
    condata->rp = rp;
    condata->idx = idx;
    

    sts = thread_Create( &local->connections[idx].t, 0, mb_receive, (void *)condata);
    if ( EVEN(sts)) {
      local->connections[idx].occupied = 0;
      errh_Error( "Error creating thread IO modbus tcp server %s, %d", rp->Name, local->s);
      free( condata);
      continue;
    }
  }
  return 0;
}


/*----------------------------------------------------------------------------*\
   Init method for the Modbus/TCP server
\*----------------------------------------------------------------------------*/

static pwr_tStatus IoRackInit (
  io_tCtx	ctx,
  io_sAgent	*ap,
  io_sRack	*rp
) 
{
  io_sServerLocal *local;
  pthread_t 	thread;
  pwr_tOName 	name;
  pwr_tStatus   sts;
  pwr_sClass_Modbus_TCP_Server *op;
  int 		i;
  unsigned short port;
    
  op = (pwr_sClass_Modbus_TCP_Server *) rp->op;
  op->Connections = 0;

  port = op->Port == 0 ? 502 : op->Port;

  sts = gdh_ObjidToName(rp->Objid, (char *) &name, sizeof(name), cdh_mNName);
  errh_Info( "Init of Modbus TCP Server %s", name);

  rp->Local = calloc(1, sizeof(io_sServerLocal));
  local = rp->Local;

  if ( op->DisableServer)
    return IO__SUCCESS;

  /* Create socket, store in local struct */
  uid_t ruid;
  ruid = getuid();
  printf( "ruid: %d\n", ruid);
  
  local->s = socket(AF_INET, SOCK_STREAM, 0);
  if (local->s < 0) { 
    errh_Error( "Error creating socket for IO modbus tcp server %s, %d", rp->Name, local->s);
    return 0;
  }

  local->loc_addr.sin_family = AF_INET;
  local->loc_addr.sin_port = htons(port);
  for ( i = 0; i < 10; i++) {
    sts = bind( local->s, (struct sockaddr *) &local->loc_addr, sizeof(local->loc_addr));
    if ( sts == 0)
      break;
    perror( "Modbus TCP Bind socket failure, retrying... ");
    sleep( 10);
  }
  if ( sts != 0) {
    printf( "Modbus TCP Bind socket failure, exiting");
    errh_Error( "Error bind socket to port for IO modbus tcp server %s, %d", rp->Name, local->s);
    return 0;
  }

  errh_Info( "Modbus TCP Sever bind to port %d, %s", port, name);
  
  sts = listen( local->s, 16);

  sts = thread_MutexInit( &local->mutex);
  if ( EVEN(sts)) return sts;

  /* Create a thread that listens for connections */
  sts = pthread_create( &thread, NULL, mb_connect, (void *)rp);
  if ( sts != 0) return sts;

  sts = mb_init_channels( ctx, ap, rp);
  if ( EVEN(sts)) return sts;

  op->Status = MB__NORMAL;

  return IO__SUCCESS;
}


static pwr_tStatus mb_init_channels( io_tCtx ctx, io_sAgent *ap, io_sRack *rp) 
{
  io_sServerModuleLocal *local_card;
  io_sCard *cardp;
  io_sServerLocal *local;
  short input_counter;
  short output_counter;
  short card_input_counter;
  short card_output_counter;
  pwr_sClass_Modbus_TCP_Server *op;
  pwr_sClass_Modbus_TCP_ServerModule *mp;
  char name[196];
  pwr_tStatus sts;
  pwr_tCid cid;
  
  io_sChannel *chanp;
  int i, latent_input_counter, latent_output_counter;
  pwr_tInt32 chan_size;
  pwr_sClass_ChanDi *chan_di;
  pwr_sClass_ChanDo *chan_do;
  pwr_sClass_ChanAi *chan_ai;
  pwr_sClass_ChanAit *chan_ait;
  pwr_sClass_ChanIi *chan_ii;
  pwr_sClass_ChanAo *chan_ao;
  pwr_sClass_ChanIo *chan_io;

  sts = gdh_ObjidToName(rp->Objid, (char *) &name, sizeof(name), cdh_mNName);

  op = (pwr_sClass_Modbus_TCP_Server *) rp->op;
  
  local = rp->Local;

  /* Create socket, store in local struct */
  
  /* Do configuration check and initialize modules. */

  cardp = rp->cardlist;

  input_counter = 0;
  output_counter = 0;
  card_input_counter = 0;
  card_output_counter = 0;
  latent_input_counter = 0;
  latent_output_counter = 0;

  while(cardp) {
    local_card = calloc(1, sizeof(*local_card));
    cardp->Local = local_card;
    input_counter = input_counter + card_input_counter + latent_input_counter;
    output_counter = output_counter + card_output_counter + latent_output_counter;
    local_card->input_area = (void *) &(op->Inputs) + input_counter;
    local_card->output_area = (void *) &(op->Outputs) + output_counter;
    card_input_counter = 0;
    card_output_counter = 0;
    latent_input_counter = 0;
    latent_output_counter = 0;

    /* From v4.1.3 we can have subclasses, find the super class */
    
    cid = cardp->Class;
    while ( ODD( gdh_GetSuperClass( cid, &cid, cardp->Objid))) ;

    switch (cid) {

      case pwr_cClass_Modbus_TCP_ServerModule:
        mp = (pwr_sClass_Modbus_TCP_ServerModule *) cardp->op;
        mp->Status = pwr_eModbusModule_StatusEnum_StatusUnknown;
        for (i = 0; i < cardp->ChanListSize; i++) {
          chanp = &cardp->chanlist[i];

	  if ( is_diag( &chanp->ChanAref)) {
	    chanp->udata |= PB_UDATA_DIAG;
	    switch (chanp->ChanClass) {	    
            case pwr_cClass_ChanIi:
	      chanp->offset = ((pwr_sClass_ChanIi *)chanp->cop)->Number;
	      chanp->size = GetChanSize( ((pwr_sClass_ChanIi *)chanp->cop)->Representation);
	      break;
	    default:
	      errh_Error( "Diagnostic channel class, card %s", cardp->Name);
	    }
	    continue;
	  }

          if (chanp->ChanClass != pwr_cClass_ChanDi) {
            card_input_counter += latent_input_counter;
	    latent_input_counter = 0;
          }

          if (chanp->ChanClass != pwr_cClass_ChanDo) {
            card_output_counter += latent_output_counter;
	    latent_output_counter = 0;
          }
      
          switch (chanp->ChanClass) {
      
            case pwr_cClass_ChanDi:
	      chan_di = (pwr_sClass_ChanDi *) chanp->cop;
              if (chan_di->Number == 0) {
	        card_input_counter += latent_input_counter;
	        latent_input_counter = 0;
	      }
              chanp->offset = card_input_counter;
	      chanp->mask = 1 << chan_di->Number;
	      if (chan_di->Representation == pwr_eDataRepEnum_Bit16) 
	        chanp->mask = swap16(chanp->mask);
	      if (chan_di->Representation == pwr_eDataRepEnum_Bit32)
	        chanp->mask = swap32((unsigned short) chanp->mask);
	      if (chan_di->Number == 0) latent_input_counter = GetChanSize(chan_di->Representation);
	      if (local_card->di_size == 0)
		local_card->di_offset = chanp->offset;
	      if (chan_di->Number == 0 || local_card->di_size == 0)
		local_card->di_size += GetChanSize(chan_di->Representation);
//	      printf("Di channel found in %s, Number %d, Offset %d\n", cardp->Name, chan_di->Number, chanp->offset);
	      break;
	  
            case pwr_cClass_ChanAi:
	      chan_ai = (pwr_sClass_ChanAi *) chanp->cop;
              chanp->offset = card_input_counter;
	      chan_size = GetChanSize(chan_ai->Representation);
              chanp->size = chan_size;
	      chanp->mask = 0;
	      card_input_counter += chan_size;
              io_AiRangeToCoef(chanp);
//	      printf("Ai channel found in %s, Number %d, Offset %d\n", cardp->Name, chan_ai->Number, chanp->offset);
	      break;
	  
            case pwr_cClass_ChanAit:
	      chan_ait = (pwr_sClass_ChanAit *) chanp->cop;
              chanp->offset = card_input_counter;
	      chan_size = GetChanSize(chan_ait->Representation);
              chanp->size = chan_size;
	      chanp->mask = 0;
	      card_input_counter += chan_size;
              io_AiRangeToCoef(chanp);
	      break;
	  
            case pwr_cClass_ChanIi:
	      chan_ii = (pwr_sClass_ChanIi *) chanp->cop;
              chanp->offset = card_input_counter;
	      chan_size = GetChanSize(chan_ii->Representation);
              chanp->size = chan_size;
	      chanp->mask = 0;
	      card_input_counter += chan_size;
//	      printf("Ii channel found in %s, Number %d, Offset %d\n", cardp->Name, chan_ii->Number, chanp->offset);
	      break;
	  
            case pwr_cClass_ChanDo:
	      chan_do = (pwr_sClass_ChanDo *) chanp->cop;
              if (chan_do->Number == 0) {
	        card_output_counter += latent_output_counter;
	        latent_output_counter = 0;
	      }
              chanp->offset = card_output_counter;
	      chan_size = GetChanSize(chan_do->Representation);
	      chanp->mask = 1 << chan_do->Number;
	      if (chan_do->Representation == pwr_eDataRepEnum_Bit16) 
	        chanp->mask = swap16(chanp->mask);
	      if (chan_do->Representation == pwr_eDataRepEnum_Bit32)
	        chanp->mask = swap32((unsigned short) chanp->mask);
	      if (chan_do->Number == 0) latent_output_counter = GetChanSize(chan_do->Representation);
	      if (local_card->do_size == 0)
		local_card->do_offset = chanp->offset;
	      if (chan_do->Number == 0 || local_card->do_size == 0)
		local_card->do_size += GetChanSize(chan_do->Representation);
//	      printf("Do channel found in %s, Number %d, Offset %d\n", cardp->Name, chan_do->Number, chanp->offset);
	      break;
	  
	    case pwr_cClass_ChanAo:
	      chan_ao = (pwr_sClass_ChanAo *) chanp->cop;
              chanp->offset = card_output_counter;
	      chan_size = GetChanSize(chan_ao->Representation);
              chanp->size = chan_size;
	      chanp->mask = 0;
	      card_output_counter += chan_size;
              io_AoRangeToCoef(chanp);
//	      printf("Ao channel found in %s, Number %d, Offset %d\n", cardp->Name, chan_ao->Number, chanp->offset);
	      break;
	  
            case pwr_cClass_ChanIo:
	      chan_io = (pwr_sClass_ChanIo *) chanp->cop;
              chanp->offset = card_output_counter;
	      chan_size = GetChanSize(chan_io->Representation);
              chanp->size = chan_size;
	      chanp->mask = 0;
	      card_output_counter += chan_size;
//	      printf("Io channel found in %s, Number %d, Offset %d\n", cardp->Name, chan_io->Number, chanp->offset);
	      break;
          }
        } /* End - for ... */
        
        break;
    } /* End - switch ... */

    local_card->input_size = card_input_counter + latent_input_counter;
    local_card->output_size = card_output_counter + latent_output_counter;

    cardp = cardp->next;
  }

  local->input_size = input_counter + card_input_counter + latent_input_counter;
  local->output_size = output_counter + card_output_counter + latent_output_counter;

  return IO__SUCCESS;
}

static void mb_shift_write( unsigned char *in, unsigned char *out, int sh, int quant)
{
  int i;

  if ( sh + quant <= 8) {
    unsigned char mask = 0;
    for ( i = sh; i < sh + quant; i++)
      mask |= 1 << i;
    out[0] &= ~mask;
    out[0] |= mask & (in[0] << sh);
    return;
  }

  for ( i = 0; i < (quant + sh)/ 8; i++) {
    if ( i == 0) {
      unsigned char mask = ~0 << sh;

      out[0] &= ~mask;
      out[0] |= mask & (in[0] << sh);
    }
    else {
      out[i] = in[i] << sh;
      out[i] |= in[i-1] >> (8 - sh);
    }
  }
  if ( (quant + sh) % 8 != 0) {
    unsigned char mask = ~0 << ((quant + sh) % 8);
    mask = ~mask;

    out[i] &= ~mask;
    out[i] |= mask & (in[i] << sh);
    out[i] |= mask & (in[i-1] >> (8 - sh));
  }
}

void mb_shift_read( unsigned char *in, unsigned char *out, int sh, int quant)
{
  int i;

  if ( sh + quant <= 8) {
    unsigned char mask = ~0;
    mask = mask >> (8 - quant);

    out[0] = mask & (in[0] >> sh);
    return;
  }

  for ( i = 0; i < quant / 8; i++) {
    out[i] = in[i] >> sh;
    out[i] |= in[i+1] << (8 - sh);
  }

  out[i] = in[i] >> sh;
  if ( (quant + sh) / 8 > quant / 8) 
    out[i] |= in[i+1] << (8 - sh);

  if ( quant % 8 != 0) {
    unsigned char mask = ~0;
    mask = mask >> (8 - (quant % 8));
    out[i] &= mask;
  }
}


/*----------------------------------------------------------------------------*\
   Read method for the Modbus TCP server
\*----------------------------------------------------------------------------*/
static pwr_tStatus IoRackRead (
  io_tCtx	ctx,
  io_sAgent	*ap,
  io_sRack	*rp
) 
{
  
  return IO__SUCCESS;
}


/*----------------------------------------------------------------------------*\
   Write method for the Modbus_TCP server
\*----------------------------------------------------------------------------*/
static pwr_tStatus IoRackWrite (
  io_tCtx	ctx,
  io_sAgent	*ap,
  io_sRack	*rp
) 
{
  return IO__SUCCESS;
}


/*----------------------------------------------------------------------------*\
  
\*----------------------------------------------------------------------------*/
static pwr_tStatus IoRackClose (
  io_tCtx	ctx,
  io_sAgent	*ap,
  io_sRack	*rp
) 
{
  io_sServerLocal* local = rp->Local;
  int i;

  for ( i = 1; i < MB_MAX_CONNECTIONS; i++) {
    if ( local->connections[i].occupied)
      mb_close_connection( rp, i);
  }

  close( local->s);

  return IO__SUCCESS;
}


/*----------------------------------------------------------------------------*\
  Every method to be exported to the workbench should be registred here.
\*----------------------------------------------------------------------------*/

pwr_dExport pwr_BindIoMethods(Modbus_TCP_Server) = {
  pwr_BindIoMethod(IoRackInit),
  pwr_BindIoMethod(IoRackRead),
  pwr_BindIoMethod(IoRackWrite),
  pwr_BindIoMethod(IoRackClose),
  pwr_NullMethod
};
