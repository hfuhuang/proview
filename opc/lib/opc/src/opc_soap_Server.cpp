/* opc_soap_Server.cpp
   Generated by gSOAP 2.7.9d from opc_msg.h
   Copyright(C) 2000-2006, Robert van Engelen, Genivia Inc. All Rights Reserved.
   This part of the software is released under one of the following licenses:
   GPL, the gSOAP public license, or Genivia's license for commercial use.
*/
#include "opc_soap_H.h"

SOAP_SOURCE_STAMP("@(#) opc_soap_Server.cpp ver 2.7.9d 2007-03-23 08:02:36 GMT")


SOAP_FMAC5 int SOAP_FMAC6 soap_serve(struct soap *soap)
{
#ifndef WITH_FASTCGI
	unsigned int k = soap->max_keep_alive;
#endif

	do
	{
#ifdef WITH_FASTCGI
		if (FCGI_Accept() < 0)
		{
			soap->error = SOAP_EOF;
			return soap_send_fault(soap);
		}
#endif

		soap_begin(soap);

#ifndef WITH_FASTCGI
		if (soap->max_keep_alive > 0 && !--k)
			soap->keep_alive = 0;
#endif

		if (soap_begin_recv(soap))
		{	if (soap->error < SOAP_STOP)
			{
#ifdef WITH_FASTCGI
				soap_send_fault(soap);
#else 
				return soap_send_fault(soap);
#endif
			}
			soap_closesock(soap);

			continue;
		}

		if (soap_envelope_begin_in(soap)
		 || soap_recv_header(soap)
		 || soap_body_begin_in(soap)
		 || soap_serve_request(soap)
		 || (soap->fserveloop && soap->fserveloop(soap)))
		{
#ifdef WITH_FASTCGI
			soap_send_fault(soap);
#else
			return soap_send_fault(soap);
#endif
		}

#ifdef WITH_FASTCGI
	} while (1);
#else
	} while (soap->keep_alive);
#endif
	return SOAP_OK;
}

#ifndef WITH_NOSERVEREQUEST
SOAP_FMAC5 int SOAP_FMAC6 soap_serve_request(struct soap *soap)
{
	soap_peek_element(soap);
	if (!soap_match_tag(soap, soap->tag, "s0:GetStatus"))
		return soap_serve___s0__GetStatus(soap);
	if (!soap_match_tag(soap, soap->tag, "s0:Read"))
		return soap_serve___s0__Read(soap);
	if (!soap_match_tag(soap, soap->tag, "s0:Write"))
		return soap_serve___s0__Write(soap);
	if (!soap_match_tag(soap, soap->tag, "s0:Subscribe"))
		return soap_serve___s0__Subscribe(soap);
	if (!soap_match_tag(soap, soap->tag, "s0:SubscriptionPolledRefresh"))
		return soap_serve___s0__SubscriptionPolledRefresh(soap);
	if (!soap_match_tag(soap, soap->tag, "s0:SubscriptionCancel"))
		return soap_serve___s0__SubscriptionCancel(soap);
	if (!soap_match_tag(soap, soap->tag, "s0:Browse"))
		return soap_serve___s0__Browse(soap);
	if (!soap_match_tag(soap, soap->tag, "s0:GetProperties"))
		return soap_serve___s0__GetProperties(soap);
	return soap->error = SOAP_NO_METHOD;
}
#endif

SOAP_FMAC5 int SOAP_FMAC6 soap_serve___s0__GetStatus(struct soap *soap)
{	struct __s0__GetStatus soap_tmp___s0__GetStatus;
	_s0__GetStatusResponse s0__GetStatusResponse;
	s0__GetStatusResponse.soap_default(soap);
	soap_default___s0__GetStatus(soap, &soap_tmp___s0__GetStatus);
	soap->encodingStyle = NULL;
	if (!soap_get___s0__GetStatus(soap, &soap_tmp___s0__GetStatus, "-s0:GetStatus", NULL))
		return soap->error;
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap->error;
	soap->error = __s0__GetStatus(soap, soap_tmp___s0__GetStatus.s0__GetStatus, &s0__GetStatusResponse);
	if (soap->error)
		return soap->error;
	soap_serializeheader(soap);
	s0__GetStatusResponse.soap_serialize(soap);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || s0__GetStatusResponse.soap_put(soap, "s0:GetStatusResponse", "")
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	};
	if (soap_end_count(soap)
	 || soap_response(soap, SOAP_OK)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || s0__GetStatusResponse.soap_put(soap, "s0:GetStatusResponse", "")
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap->error;
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_serve___s0__Read(struct soap *soap)
{	struct __s0__Read soap_tmp___s0__Read;
	_s0__ReadResponse s0__ReadResponse;
	s0__ReadResponse.soap_default(soap);
	soap_default___s0__Read(soap, &soap_tmp___s0__Read);
	soap->encodingStyle = NULL;
	if (!soap_get___s0__Read(soap, &soap_tmp___s0__Read, "-s0:Read", NULL))
		return soap->error;
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap->error;
	soap->error = __s0__Read(soap, soap_tmp___s0__Read.s0__Read, &s0__ReadResponse);
	if (soap->error)
		return soap->error;
	soap_serializeheader(soap);
	s0__ReadResponse.soap_serialize(soap);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || s0__ReadResponse.soap_put(soap, "s0:ReadResponse", "")
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	};
	if (soap_end_count(soap)
	 || soap_response(soap, SOAP_OK)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || s0__ReadResponse.soap_put(soap, "s0:ReadResponse", "")
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap->error;
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_serve___s0__Write(struct soap *soap)
{	struct __s0__Write soap_tmp___s0__Write;
	_s0__WriteResponse s0__WriteResponse;
	s0__WriteResponse.soap_default(soap);
	soap_default___s0__Write(soap, &soap_tmp___s0__Write);
	soap->encodingStyle = NULL;
	if (!soap_get___s0__Write(soap, &soap_tmp___s0__Write, "-s0:Write", NULL))
		return soap->error;
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap->error;
	soap->error = __s0__Write(soap, soap_tmp___s0__Write.s0__Write, &s0__WriteResponse);
	if (soap->error)
		return soap->error;
	soap_serializeheader(soap);
	s0__WriteResponse.soap_serialize(soap);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || s0__WriteResponse.soap_put(soap, "s0:WriteResponse", "")
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	};
	if (soap_end_count(soap)
	 || soap_response(soap, SOAP_OK)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || s0__WriteResponse.soap_put(soap, "s0:WriteResponse", "")
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap->error;
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_serve___s0__Subscribe(struct soap *soap)
{	struct __s0__Subscribe soap_tmp___s0__Subscribe;
	_s0__SubscribeResponse s0__SubscribeResponse;
	s0__SubscribeResponse.soap_default(soap);
	soap_default___s0__Subscribe(soap, &soap_tmp___s0__Subscribe);
	soap->encodingStyle = NULL;
	if (!soap_get___s0__Subscribe(soap, &soap_tmp___s0__Subscribe, "-s0:Subscribe", NULL))
		return soap->error;
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap->error;
	soap->error = __s0__Subscribe(soap, soap_tmp___s0__Subscribe.s0__Subscribe, &s0__SubscribeResponse);
	if (soap->error)
		return soap->error;
	soap_serializeheader(soap);
	s0__SubscribeResponse.soap_serialize(soap);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || s0__SubscribeResponse.soap_put(soap, "s0:SubscribeResponse", "")
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	};
	if (soap_end_count(soap)
	 || soap_response(soap, SOAP_OK)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || s0__SubscribeResponse.soap_put(soap, "s0:SubscribeResponse", "")
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap->error;
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_serve___s0__SubscriptionPolledRefresh(struct soap *soap)
{	struct __s0__SubscriptionPolledRefresh soap_tmp___s0__SubscriptionPolledRefresh;
	_s0__SubscriptionPolledRefreshResponse s0__SubscriptionPolledRefreshResponse;
	s0__SubscriptionPolledRefreshResponse.soap_default(soap);
	soap_default___s0__SubscriptionPolledRefresh(soap, &soap_tmp___s0__SubscriptionPolledRefresh);
	soap->encodingStyle = NULL;
	if (!soap_get___s0__SubscriptionPolledRefresh(soap, &soap_tmp___s0__SubscriptionPolledRefresh, "-s0:SubscriptionPolledRefresh", NULL))
		return soap->error;
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap->error;
	soap->error = __s0__SubscriptionPolledRefresh(soap, soap_tmp___s0__SubscriptionPolledRefresh.s0__SubscriptionPolledRefresh, &s0__SubscriptionPolledRefreshResponse);
	if (soap->error)
		return soap->error;
	soap_serializeheader(soap);
	s0__SubscriptionPolledRefreshResponse.soap_serialize(soap);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || s0__SubscriptionPolledRefreshResponse.soap_put(soap, "s0:SubscriptionPolledRefreshResponse", "")
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	};
	if (soap_end_count(soap)
	 || soap_response(soap, SOAP_OK)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || s0__SubscriptionPolledRefreshResponse.soap_put(soap, "s0:SubscriptionPolledRefreshResponse", "")
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap->error;
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_serve___s0__SubscriptionCancel(struct soap *soap)
{	struct __s0__SubscriptionCancel soap_tmp___s0__SubscriptionCancel;
	_s0__SubscriptionCancelResponse s0__SubscriptionCancelResponse;
	s0__SubscriptionCancelResponse.soap_default(soap);
	soap_default___s0__SubscriptionCancel(soap, &soap_tmp___s0__SubscriptionCancel);
	soap->encodingStyle = NULL;
	if (!soap_get___s0__SubscriptionCancel(soap, &soap_tmp___s0__SubscriptionCancel, "-s0:SubscriptionCancel", NULL))
		return soap->error;
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap->error;
	soap->error = __s0__SubscriptionCancel(soap, soap_tmp___s0__SubscriptionCancel.s0__SubscriptionCancel, &s0__SubscriptionCancelResponse);
	if (soap->error)
		return soap->error;
	soap_serializeheader(soap);
	s0__SubscriptionCancelResponse.soap_serialize(soap);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || s0__SubscriptionCancelResponse.soap_put(soap, "s0:SubscriptionCancelResponse", "")
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	};
	if (soap_end_count(soap)
	 || soap_response(soap, SOAP_OK)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || s0__SubscriptionCancelResponse.soap_put(soap, "s0:SubscriptionCancelResponse", "")
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap->error;
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_serve___s0__Browse(struct soap *soap)
{	struct __s0__Browse soap_tmp___s0__Browse;
	_s0__BrowseResponse s0__BrowseResponse;
	s0__BrowseResponse.soap_default(soap);
	soap_default___s0__Browse(soap, &soap_tmp___s0__Browse);
	soap->encodingStyle = NULL;
	if (!soap_get___s0__Browse(soap, &soap_tmp___s0__Browse, "-s0:Browse", NULL))
		return soap->error;
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap->error;
	soap->error = __s0__Browse(soap, soap_tmp___s0__Browse.s0__Browse, &s0__BrowseResponse);
	if (soap->error)
		return soap->error;
	soap_serializeheader(soap);
	s0__BrowseResponse.soap_serialize(soap);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || s0__BrowseResponse.soap_put(soap, "s0:BrowseResponse", "")
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	};
	if (soap_end_count(soap)
	 || soap_response(soap, SOAP_OK)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || s0__BrowseResponse.soap_put(soap, "s0:BrowseResponse", "")
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap->error;
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_serve___s0__GetProperties(struct soap *soap)
{	struct __s0__GetProperties soap_tmp___s0__GetProperties;
	_s0__GetPropertiesResponse s0__GetPropertiesResponse;
	s0__GetPropertiesResponse.soap_default(soap);
	soap_default___s0__GetProperties(soap, &soap_tmp___s0__GetProperties);
	soap->encodingStyle = NULL;
	if (!soap_get___s0__GetProperties(soap, &soap_tmp___s0__GetProperties, "-s0:GetProperties", NULL))
		return soap->error;
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap->error;
	soap->error = __s0__GetProperties(soap, soap_tmp___s0__GetProperties.s0__GetProperties, &s0__GetPropertiesResponse);
	if (soap->error)
		return soap->error;
	soap_serializeheader(soap);
	s0__GetPropertiesResponse.soap_serialize(soap);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || s0__GetPropertiesResponse.soap_put(soap, "s0:GetPropertiesResponse", "")
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	};
	if (soap_end_count(soap)
	 || soap_response(soap, SOAP_OK)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || s0__GetPropertiesResponse.soap_put(soap, "s0:GetPropertiesResponse", "")
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap->error;
	return soap_closesock(soap);
}

/* End of opc_soap_Server.cpp */
