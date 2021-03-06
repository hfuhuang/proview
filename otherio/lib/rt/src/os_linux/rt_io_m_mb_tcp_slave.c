/* 
 * Proview   $Id: rt_io_m_mb_tcp_slave.c,v 1.5 2008-09-30 14:25:47 claes Exp $
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

/* rt_io_m_pb_dp_slave.c -- io methods for a profibus DP slave */


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

#include "pwr.h"
#include "co_cdh.h"
#include "pwr_baseclasses.h"
#include "pwr_basecomponentclasses.h"
#include "pwr_otherioclasses.h"
#include "rt_gdh.h"
#include "rt_io_base.h"
#include "rt_io_bus.h"
#include "rt_io_msg.h"
#include "rt_errh.h"
#include "co_cdh.h"
#include "co_time.h"
#include "rt_mb_msg.h"

#include "rt_io_mb_locals.h"

char rcv_buffer[65536];

/* Check if channel should be fetched from diagnostic area, 
   i.e. channel name starts with "Diag_" */

static int connect_slave( io_sRackLocal *local, io_sRack *rp)
{
  pwr_tStatus sts;
  pwr_sClass_Modbus_TCP_Slave *op;
  int buffsize = 10000;
  int flags;
  fd_set fdr;				/* For select call */
  fd_set fdw;				/* For select call */
  struct timeval tv;
  unsigned short port;

  time_GetTimeMonotonic( &local->last_try_connect_time);

  op = (pwr_sClass_Modbus_TCP_Slave *) rp->op;

  /* Create socket, store in local struct */
  port = op->Port == 0 ? 502 : op->Port;
  
  local->s = socket(AF_INET, SOCK_STREAM, 0);
  if (local->s < 0) { 
    errh_Error( "Error creating socket for IO modbus tcp slave %s, %d", rp->Name, local->s);
    return local->s;
  }

  sts = setsockopt(local->s, SOL_SOCKET, SO_RCVBUF, &buffsize, sizeof(buffsize)); 

  if (sts < 0) { 
    printf( "Error setting receive buffer size \n");
  }
  
  /* Initialize remote address structure */

  local->rem_addr.sin_family = AF_INET;
  local->rem_addr.sin_port = htons(port);
  local->rem_addr.sin_addr.s_addr = inet_addr((char *) &(op->Address));
  
  /* Connect to remote address */

  fcntl(local->s, F_SETFL, (flags = fcntl(local->s, F_GETFL)) | O_NONBLOCK);
  
  sts = connect(local->s, (struct sockaddr *) &local->rem_addr, sizeof(local->rem_addr)); 

  if (sts < 0) { 
    FD_ZERO(&fdr);
    FD_ZERO(&fdw);
    FD_SET(local->s, &fdr);
    FD_SET(local->s, &fdw);
    
    if (op->ReconnectLimit > 200) {
      tv.tv_sec = 0;
      tv.tv_usec = 1000 * op->ReconnectLimit;
    } else {
      tv.tv_sec = 0;
      tv.tv_usec = 200000;
    }

    sts = select(32, &fdr, &fdw, NULL, &tv);
    
    if (sts <= 0) {
      close(local->s);
      errh_Error( "Error connecting remote socket for IO modbus slave %s, %d", rp->Name, sts);
      return -1;
    }
  }

  fcntl(local->s, F_SETFL, (flags = fcntl(local->s, F_GETFL)) ^ O_NONBLOCK);
  
  return sts;
}

pwr_tStatus mb_recv_data(io_sRackLocal *local, 
                             io_sRack      *rp,
			     pwr_sClass_Modbus_TCP_Slave *sp)
{
  io_sCardLocal *local_card;
  io_sCard *cardp;
  pwr_sClass_Modbus_Module *mp;
  pwr_tStatus sts;
  fd_set fdr;				/* For select call */
  fd_set fde;				/* For select call */
  fd_set fdw;				/* For select call */
  struct timeval tv;
  pwr_tBoolean found;
  int data_size;
  rec_buf *rb;
  pwr_tCid cid;
  unsigned char fc;
  short int trans_id;
  short int size_of_msg;
  
  /* Receive answer */

  sts = 1;

  while (sts > 0) {
    FD_ZERO(&fdr);
    FD_ZERO(&fdw);
    FD_ZERO(&fde);
    FD_SET(local->s, &fdr);
    FD_SET(local->s, &fdw);
    FD_SET(local->s, &fde);

    size_of_msg = 0;
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    sts = select(32, &fdr, &fdw, &fde, &tv);

    if (sts < 0) {
      sp->Status = MB__CONNLOST;
      close(local->s);
      errh_Error( "Connection lost to modbus slave, %s", rp->Name);
      return IO__SUCCESS;
    }
    
    if (!(FD_ISSET(local->s, &fdw))) {
      sp->Status = MB__CONNDOWN;
      close(local->s);
      errh_Error( "Connection down to modbus slave, %s", rp->Name);
      return IO__SUCCESS;
    }

    if (local->expected_msgs > 0) {
      tv.tv_sec = 0;
      tv.tv_usec = sp->ResponseTime * 1000;
    } else {
      tv.tv_sec = 0;
      tv.tv_usec = 0;
    }

    FD_ZERO(&fdr);
    FD_ZERO(&fde);
    FD_SET(local->s, &fdr);
    FD_SET(local->s, &fde);

    sts = select(32, &fdr, NULL, &fde, &tv);

    if (sts < 0) {
      sp->Status = MB__CONNLOST;
      close(local->s);
      errh_Error( "Connection lost to modbus slave, %s", rp->Name);
      return IO__SUCCESS;
    }
    
    if ((sts == 0) && (local->expected_msgs > 0)) {
      local->msgs_lost++;
      if (local->msgs_lost > MAX_MSGS_LOST) {
        sp->Status = MB__CONNDOWN;
        close(local->s);
        errh_Error( "Connection down to modbus slave, %s", rp->Name);
      }
      return IO__SUCCESS;
    }

    if (sts > 0 && FD_ISSET(local->s, &fdr)) {
      data_size = recv(local->s, rcv_buffer, sizeof(rec_buf), 0);

      if (data_size < 0) {
        sp->Status = MB__CONNLOST;
	close(local->s);
	errh_Error( "Connection lost to modbus slave, %s", rp->Name);
	return IO__SUCCESS;
      }

      if (data_size == 0) {
        sp->Status = MB__CONNDOWN;
	close(local->s);
	errh_Error( "Connection down to modbus slave, %s", rp->Name);
	return IO__SUCCESS;
      }

      while (data_size > 0) {

        local->msgs_lost = 0;

  	sp->RX_packets++;
	
	local->expected_msgs--;

	cardp = rp->cardlist;

	if (data_size < sizeof(mbap_header))
	  break;

	rb = (rec_buf *) &rcv_buffer[size_of_msg];

	trans_id = ntohs(rb->head.trans_id);
	size_of_msg += ntohs(rb->head.length) + 6;
	data_size -= ntohs(rb->head.length) + 6;

        while(cardp) {
          /* From v4.1.3 we can have subclasses, find the super class */
          found = FALSE;
          cid = cardp->Class;
          while ( ODD( gdh_GetSuperClass( cid, &cid, cardp->Objid))) ;

          switch (cid) {

            case pwr_cClass_Modbus_Module: {
              mp = (pwr_sClass_Modbus_Module *) cardp->op;

              local_card = cardp->Local;

	      if (local_card->trans_id == trans_id) {

                fc = (unsigned char) *rb->buf;
                if (fc > 0x80) {
	          res_fault *res_f;
	          res_f = (res_fault *) rb->buf;
	          mp->Status = res_f->ec;
	          break;
	        }

                if (fc != mp->FunctionCode) {
	          mp->Status = pwr_eModbusModule_StatusEnum_StatusUnknown;
	          break;
	        }

	        mp->Status = pwr_eModbusModule_StatusEnum_OK;

                switch (fc) {
	          case pwr_eModbus_FCEnum_ReadCoils: {
	            res_read *res_r;
                    res_r = (res_read *) rb->buf;
		    memcpy(local_card->input_area, res_r->buf, MIN(res_r->bc, local_card->input_size));
	            break;
		  }

	          case pwr_eModbus_FCEnum_ReadDiscreteInputs: {
	            res_read *res_r;
                    res_r = (res_read *) rb->buf;
		    memcpy(local_card->input_area, res_r->buf, MIN(res_r->bc, local_card->input_size));
	            break;
		  }

	          case pwr_eModbus_FCEnum_ReadHoldingRegisters: {
	            res_read *res_r;
                    res_r = (res_read *) rb->buf;
		    memcpy(local_card->input_area, res_r->buf, MIN(res_r->bc, local_card->input_size));
	            break;
		  }

	          case pwr_eModbus_FCEnum_ReadInputRegisters: { 
	            res_read *res_r;
                    res_r = (res_read *) rb->buf;
		    memcpy(local_card->input_area, res_r->buf, MIN(res_r->bc, local_card->input_size));
	            break;
		  }

	          case pwr_eModbus_FCEnum_WriteMultipleCoils:
	          case pwr_eModbus_FCEnum_WriteMultipleRegisters:
	            // Nothing good to do here
	            break;
	        }
		found = TRUE;
	      }
	      break;
	    }
	    default:
	      break;
	  } /* End - switch (cid) ... */
	  if (found)
	    break;
          cardp = cardp->next;
        } /* End - while(cardp) ... */
      } /* End - data_size > 0 ... */
    } /* End - if received message ... */
  } /* End - receive messages ... */
  
  return IO__SUCCESS;
}

pwr_tStatus mb_send_data(io_sRackLocal *local, 
                             io_sRack      *rp,
			     pwr_sClass_Modbus_TCP_Slave *sp, 
			     mb_tSendMask   mask)
{
  io_sCardLocal *local_card;
  io_sCard *cardp;
  pwr_sClass_Modbus_Module *mp;
  pwr_tStatus sts;
  pwr_tCid cid;

  /* Send messages to slave */

  cardp = rp->cardlist;

  while(cardp) {

    /* From v4.1.3 we can have subclasses, find the super class */
    cid = cardp->Class;
    while ( ODD( gdh_GetSuperClass( cid, &cid, cardp->Objid))) ;

    switch (cid) {
      case pwr_cClass_Modbus_Module:
        mp = (pwr_sClass_Modbus_Module *) cardp->op;

        if (!mp->Continous && !mp->SendOp) {
	  break;
	}
		
        local_card = cardp->Local;

        if (mask & mb_mSendMask_ReadReq) {
	  switch (mp->FunctionCode) {
	    case pwr_eModbus_FCEnum_ReadCoils:
	    case pwr_eModbus_FCEnum_ReadDiscreteInputs: {
	      read_req rr;

	      mp->SendOp = FALSE;

	      local->trans_id++;
	      local_card->trans_id = local->trans_id;

              rr.head.trans_id = htons(local->trans_id);
              rr.head.proto_id = 0;
              rr.head.length = htons(sizeof(read_req) - 6);
              rr.head.unit_id = mp->UnitId;
              rr.fc = mp->FunctionCode;
              rr.addr = htons(mp->Address);
              rr.quant = htons(local_card->no_di);
	      //              rr.quant = ntohs(local_card->input_size * 8);

	      sts = send(local->s, &rr, sizeof(read_req), MSG_DONTWAIT);
	      if (sts < 0) {
                sp->Status = MB__CONNDOWN;
                close(local->s);
                errh_Error( "Connection down to modbus slave, %s", rp->Name);
                return IO__SUCCESS;
              }
	      
	      local->expected_msgs++;
	      sp->TX_packets++;
	      break;
	    }

	    case pwr_eModbus_FCEnum_ReadHoldingRegisters:
	    case pwr_eModbus_FCEnum_ReadInputRegisters: {
	      read_req rr;

	      mp->SendOp = FALSE;

	      local->trans_id++;
	      local_card->trans_id = local->trans_id;

              rr.head.trans_id = htons(local->trans_id);
              rr.head.proto_id = 0;
              rr.head.length = htons(sizeof(read_req) - 6);
              rr.head.unit_id = mp->UnitId;
              rr.fc = mp->FunctionCode;
              rr.addr = htons(mp->Address);
              rr.quant = ntohs((local_card->input_size + 1) / 2);

	      sts = send(local->s, &rr, sizeof(read_req), MSG_DONTWAIT);
	      if (sts < 0) {
                sp->Status = MB__CONNDOWN;
                close(local->s);
                errh_Error( "Connection down to modbus slave, %s", rp->Name);
                return IO__SUCCESS;
              }
	      sp->TX_packets++;
	      local->expected_msgs++;
	      break;
	    }
	  } /* End - switch FC ... */
	}

        if (mask & mb_mSendMask_WriteReq) {
	  switch (mp->FunctionCode) {

	    case pwr_eModbus_FCEnum_WriteSingleCoil: {
	      write_single_req wsr;

	      mp->SendOp = FALSE;

	      local->trans_id++;
	      local_card->trans_id = local->trans_id;

              wsr.head.trans_id = htons(local->trans_id);
              wsr.head.proto_id = 0;
              wsr.head.length = htons(sizeof(wsr) - 6); 
              wsr.head.unit_id = mp->UnitId;
              wsr.fc = mp->FunctionCode;
              wsr.addr = htons(mp->Address);
	      if (local_card->output_size == 4) {
                if (*(int *)local_card->output_area)
		  wsr.value = ntohs(0xFF00);
		else wsr.value = 0;
	      } else if (local_card->output_size == 2) {
                if (*(short int *)local_card->output_area)
		  wsr.value = ntohs(0xFF00);
		else wsr.value = 0;
	      } else if (local_card->output_size == 1) {
                if (*(char *)local_card->output_area)
		  wsr.value = ntohs(0xFF00);
		else wsr.value = 0;
	      } else wsr.value = 0;

	      sts = send(local->s, &wsr, ntohs(wsr.head.length) + 6, MSG_DONTWAIT);
	      if (sts < 0) {
                sp->Status = MB__CONNDOWN;
                close(local->s);
                errh_Error( "Connection down to modbus slave, %s", rp->Name);
                return IO__SUCCESS;
              }
	      local->expected_msgs++;
	      sp->TX_packets++;
	      break;
	    }

	    case pwr_eModbus_FCEnum_WriteMultipleCoils: {
	      write_coils_req wcr;

	      mp->SendOp = FALSE;

	      local->trans_id++;
	      local_card->trans_id = local->trans_id;

              wcr.head.trans_id = htons(local->trans_id);
              wcr.head.proto_id = 0;
              wcr.head.length = htons(sizeof(wcr) - 6 - 
	                             sizeof(wcr.reg) + local_card->output_size);
              wcr.head.unit_id = mp->UnitId;
              wcr.fc = mp->FunctionCode;
              wcr.addr = htons(mp->Address);
              wcr.quant = htons(local_card->no_do);
	      //              wcr.quant = ntohs((local_card->output_size) * 8);
	      wcr.bc = local_card->output_size;
	      memcpy(wcr.reg, local_card->output_area, local_card->output_size);

	      sts = send(local->s, &wcr, ntohs(wcr.head.length) + 6, MSG_DONTWAIT);
	      if (sts < 0) {
                sp->Status = MB__CONNDOWN;
                close(local->s);
                errh_Error( "Connection down to modbus slave, %s", rp->Name);
                return IO__SUCCESS;
              }
	      local->expected_msgs++;
	      sp->TX_packets++;
	      break;
	    }

	    case pwr_eModbus_FCEnum_WriteMultipleRegisters: {
	      write_reg_req wrr;

	      mp->SendOp = FALSE;

	      local->trans_id++;
	      local_card->trans_id = local->trans_id;

              wrr.head.trans_id = htons(local->trans_id);
              wrr.head.proto_id = 0;
              wrr.head.length = htons(sizeof(wrr) - 6 - 
	                             sizeof(wrr.reg) + local_card->output_size);
              wrr.head.unit_id = mp->UnitId;
              wrr.fc = mp->FunctionCode;
              wrr.addr = htons(mp->Address);
              wrr.quant = ntohs((local_card->output_size) / 2);
	      wrr.bc = local_card->output_size;
	      memcpy(wrr.reg, local_card->output_area, local_card->output_size);

	      sts = send(local->s, &wrr, ntohs(wrr.head.length) + 6, MSG_DONTWAIT);
	      if (sts < 0) {
                sp->Status = MB__CONNDOWN;
                close(local->s);
                errh_Error( "Connection down to modbus slave, %s", rp->Name);
                return IO__SUCCESS;
              }
	      sp->TX_packets++;
	      local->expected_msgs++;
	      break;
	    }
	  } /* End - switch FC ... */
	}
	if (sts < 0) {
	  sp->Status = MB__CONNDOWN;
	  close(local->s);
	  errh_Error( "Connection down to modbus slave, %s", rp->Name);
	  return IO__SUCCESS;
	}

	if (sp->SingleOp)    
	  sts = mb_recv_data(local, rp, sp);

	if (sp->Status != MB__NORMAL) return IO__SUCCESS;

	break;
    } /* End - switch cid ... */

    cardp = cardp->next;

  } /* End - while cardp ... */

  return IO__SUCCESS;
}
/*----------------------------------------------------------------------------*\
   Init method for the Modbus_TCP slave 
\*----------------------------------------------------------------------------*/

static pwr_tStatus IoRackInit (
  io_tCtx	ctx,
  io_sAgent	*ap,
  io_sRack	*rp
) 
{
  io_sCardLocal *local_card;
  io_sCard *cardp;
  io_sRackLocal *local;
  short input_counter;
  short output_counter;
  short card_input_counter;
  short card_output_counter;
  short no_di;
  short no_do;
  pwr_sClass_Modbus_TCP_Slave *op;
  pwr_sClass_Modbus_Module *mp;
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
  errh_Info( "Init of Modbus TCP Slave and Modules %s", name);

  op = (pwr_sClass_Modbus_TCP_Slave *) rp->op;
  
  rp->Local = calloc(1, sizeof(io_sRackLocal));
  local = rp->Local;

  /* Create socket, store in local struct */
  
  sts = connect_slave(local, rp);
  
  if (sts < 0) {
    op->Status = MB__CONNDOWN;
  } else {
    op->Status = MB__NORMAL;
  }
  
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
    no_di = 0;
    no_do = 0;

    /* From v4.1.3 we can have subclasses, find the super class */
    
    cid = cardp->Class;
    while ( ODD( gdh_GetSuperClass( cid, &cid, cardp->Objid))) ;

    switch (cid) {

      case pwr_cClass_Modbus_Module:
        mp = (pwr_sClass_Modbus_Module *) cardp->op;
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
	      no_di++;
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
	      no_do++;
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
    local_card->no_di = no_di;
    local_card->no_do = no_do;

    cardp = cardp->next;
  }

  local->input_size = input_counter + card_input_counter + latent_input_counter;
  local->output_size = output_counter + card_output_counter + latent_output_counter;

  return IO__SUCCESS;
}


/*----------------------------------------------------------------------------*\
   Read method for the Modbus_TCP slave 
\*----------------------------------------------------------------------------*/
static pwr_tStatus IoRackRead (
  io_tCtx	ctx,
  io_sAgent	*ap,
  io_sRack	*rp
) 
{
  io_sRackLocal *local;
  pwr_sClass_Modbus_TCP_Slave *sp;
  pwr_tStatus sts;
  pwr_tTime now;
  pwr_tDeltaTime dt;
  
  local = rp->Local;
  
  sp = (pwr_sClass_Modbus_TCP_Slave *) rp->op;

  if (((sp->Status == MB__CONNDOWN) || (sp->Status == MB__CONNLOST)) && sp->DisableSlave != 1) {
    /* Reconnect */

    time_GetTimeMonotonic( &now);
    time_Adiff(&dt, &now, &local->last_try_connect_time);
    if (dt.tv_sec >= (1 << MIN(sp->ReconnectCount, 6))) {
      sts = connect_slave(local, rp);
      if (sts >= 0) {
	sp->ReconnectCount = 0;
        sp->Status = MB__NORMAL;
      } else {
        sp->ReconnectCount++;
        memset(&sp->Inputs, 0, local->input_size);
      }
    }
  }

  /* Receive data */
  if ((sp->Status == MB__NORMAL) && !sp->SingleOp) {
    sts = mb_recv_data(local, rp, sp);
  }  
  
  if (sp->DisableSlave != 1) {
  
    if (sp->Status == MB__NORMAL) {
      sp->ErrorCount = 0;
    }
    else {
      sp->ErrorCount++;
    }

    if (sp->ErrorCount > sp->ErrorLimit) {
      memset(&sp->Inputs, 0, local->input_size);
    }
  }
  else {
    sp->ErrorCount = 0;
    sp->Status = MB__DISABLED;
  }
  
  return IO__SUCCESS;
}


/*----------------------------------------------------------------------------*\
   Write method for the Modbus_TCP slave
\*----------------------------------------------------------------------------*/
static pwr_tStatus IoRackWrite (
  io_tCtx	ctx,
  io_sAgent	*ap,
  io_sRack	*rp
) 
{
  io_sRackLocal *local;
  pwr_sClass_Modbus_TCP_Slave *sp;
  pwr_tStatus sts;

  local = rp->Local;
  
  sp = (pwr_sClass_Modbus_TCP_Slave *) rp->op;
  
  local->expected_msgs = 0;

  if (sp->Status == MB__NORMAL && sp->DisableSlave != 1) {
    sts = mb_send_data(local, rp, sp, mb_mSendMask_WriteReq);
  }

  if (sp->DisableSlave == 1) sp->Status = MB__DISABLED;

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
  return IO__SUCCESS;
}


/*----------------------------------------------------------------------------*\
  Every method to be exported to the workbench should be registred here.
\*----------------------------------------------------------------------------*/

pwr_dExport pwr_BindIoMethods(Modbus_TCP_Slave) = {
  pwr_BindIoMethod(IoRackInit),
  pwr_BindIoMethod(IoRackRead),
  pwr_BindIoMethod(IoRackWrite),
  pwr_BindIoMethod(IoRackClose),
  pwr_NullMethod
};
