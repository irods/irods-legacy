#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <amqp.h>
#include <amqp_framing.h>
#include "utils.h"
#include <time.h>
#include <msgpack.h>

typedef struct BytesBuf {   /* have to add BytesBuf to get Doxygen working */
    int len;    /* len in bytes in buf */
    void *buf;
} bytesBuf_t;

int 
initOoiReqProp (amqp_basic_properties_t *props, char *replyQueueName,
char *receiver, char *op, char *format, char *ts);
int
initRecvApiReply (amqp_connection_state_t conn, amqp_frame_t *frame);
int
recvApiReplyProp (amqp_connection_state_t conn, amqp_frame_t *frame);
int
recvApiReplyBody (amqp_connection_state_t conn, int channel,
amqp_frame_t *frame, bytesBuf_t **extBytesBuf);
int
printAmqpHeaders (amqp_table_t *headers);

#define NUM_OOI_HEADER_ENTRIES	20

int main(int argc, char const * const *argv) {
  char const *hostname;
  int port;
  char const *exchange;
  char const *routingkey, *replyRoutingkey;
  int sockfd;
  amqp_connection_state_t conn;
  amqp_bytes_t replyQueue;
  amqp_queue_declare_ok_t *queue_declare_ret;
  amqp_basic_properties_t props;
  amqp_bytes_t message_bytes;
  char receiverStr[128], tsStr[128], replyToStr[128];
  msgpack_sbuffer* buffer;
  msgpack_packer* pk;
  amqp_frame_t frame;
  int status;
  bytesBuf_t *extBytesBuf;	/* used only if more than 1 receive is needed */
  amqp_basic_properties_t *p;
  msgpack_unpacked unpackedRes;
  msgpack_object *unpackedObj;
  size_t offset = 0;
  char bankId[1204];

  hostname = "localhost";
  port = 5672;
  exchange = "ion_mwan-hp";
  routingkey = "bank";
  replyRoutingkey = "myAPIReply";

  conn = amqp_new_connection();

  die_on_error(sockfd = amqp_open_socket(hostname, port), "Opening socket");
  amqp_set_sockfd(conn, sockfd);
  die_on_amqp_error(amqp_login(conn, "/", 0, 131072, 0, AMQP_SASL_METHOD_PLAIN, "guest", "guest"),
		    "Logging in");
  amqp_channel_open(conn, 1);
  die_on_amqp_error(amqp_get_rpc_reply(conn), "Opening channel");

  queue_declare_ret = amqp_queue_declare(conn, 1, amqp_empty_bytes, 0, 0, 0, 1,
                                                    amqp_empty_table);
  die_on_amqp_error(amqp_get_rpc_reply(conn), "Declaring queue");
  replyQueue = amqp_bytes_malloc_dup(queue_declare_ret->queue);
  if (replyQueue.bytes == NULL) {
    fprintf(stderr, "Out of memory while copying queue name");
    return 1;
  } else {
    printf ("replyQueue name:%s\n", (char *) replyQueue.bytes);
  }
  amqp_queue_bind(conn, 1, replyQueue, amqp_cstring_bytes(exchange), 
    amqp_cstring_bytes(replyRoutingkey), amqp_empty_table);
  amqp_basic_consume(conn, 1, replyQueue, amqp_empty_bytes, 0, 0, 0, amqp_empty_table);

  snprintf (receiverStr, 128, "%s,%s", exchange, routingkey);
  snprintf (tsStr, 128, "%d", (int) time (0));
  snprintf (replyToStr, 128, "%s,%s", exchange, replyRoutingkey);
  initOoiReqProp (&props, (char *) replyToStr, receiverStr, 
      (char *) "new_account", (char *) "bank_new_account_in", tsStr);

  /* creates buffer and serializer instance. */
  buffer = msgpack_sbuffer_new();
  pk = msgpack_packer_new(buffer, msgpack_sbuffer_write);
  msgpack_pack_map (pk, 2);
  msgpack_pack_raw (pk, strlen ("account_type"));
  msgpack_pack_raw_body (pk, "account_type", strlen ("account_type"));
  msgpack_pack_raw (pk, strlen ("Savings"));
  msgpack_pack_raw_body (pk, "Savings", strlen ("Savings"));
  msgpack_pack_raw (pk, strlen ("name"));
  msgpack_pack_raw_body (pk, "name", strlen ("name"));
  msgpack_pack_raw (pk, strlen ("John"));
  msgpack_pack_raw_body (pk, "John", strlen ("John"));

  message_bytes.len = buffer->size;
  message_bytes.bytes = buffer->data;

  printf ("len=%d, data=%s\n", message_bytes.len, (char *) message_bytes.bytes);

  die_on_error(amqp_basic_publish (conn,
				    1,
				    amqp_cstring_bytes(exchange),
				    amqp_cstring_bytes(routingkey),
				    0,
				    0,
				    &props,
				    message_bytes),
		 "Publishing");

  msgpack_sbuffer_free(buffer);
  msgpack_packer_free (pk);
  status = initRecvApiReply (conn, &frame);
  if (status < 0) return status;

  status = recvApiReplyProp (conn, &frame);
  if (status < 0) return status;

  p = (amqp_basic_properties_t *) frame.payload.properties.decoded;
  printAmqpHeaders (&p->headers);

  status = recvApiReplyBody (conn, 1, &frame, &extBytesBuf);
  if (status < 0) return status;

  msgpack_unpacked_init(&unpackedRes);

  while (msgpack_unpack_next(&unpackedRes, 
   (const char*) frame.payload.body_fragment.bytes, 
    frame.payload.body_fragment.len, &offset)) {
    unpackedObj = &unpackedRes.data;
    /* expect a str return */
    if (unpackedObj->type != MSGPACK_OBJECT_RAW) {
      fprintf (stderr, "unexpected unpacked type %d\n", unpackedObj->type);
    } else {
      if (unpackedObj->via.raw.size >= 1024) {
        fprintf (stderr, "unpackedObj size %d too large\n", 
          unpackedObj->via.raw.size);
      } else {
          strncpy (bankId, unpackedObj->via.raw.ptr, unpackedObj->via.raw.size);
          bankId[unpackedObj->via.raw.size] = '\0';
          printf ("bankId = %s\n", bankId);
      }
    }
#if 0
    msgpack_object_print(stdout, unpackedObj);
    puts("");
#endif
  }
  amqp_maybe_release_buffers(conn);
  die_on_amqp_error(amqp_channel_close(conn, 1, AMQP_REPLY_SUCCESS), "Closing channel");
  die_on_amqp_error(amqp_connection_close(conn, AMQP_REPLY_SUCCESS), "Closing connection");
  die_on_error(amqp_destroy_connection(conn), "Ending connection");

  return 0;
}

int
initRecvApiReply (amqp_connection_state_t conn, amqp_frame_t *frame)
{
    amqp_basic_deliver_t *d;
    int status;

    amqp_maybe_release_buffers(conn);
    status = amqp_simple_wait_frame(conn, frame);
    if (status < 0) {
      fprintf(stderr, 
        "initRecvApiReply: amqp_simple_wait_frame failed. status %d\n", status);
      return status;
    }
    printf ("Frame type %d, channel %d\n", frame->frame_type, frame->channel);
    if (frame->frame_type != AMQP_FRAME_METHOD) {
	fprintf (stderr, "frame_type %d is not AMQP_FRAME_METHOD", 
	 frame->frame_type);
	return -1;
    }

    printf("Method %s\n", amqp_method_name(frame->payload.method.id));
    if (frame->payload.method.id != AMQP_BASIC_DELIVER_METHOD) {
	fprintf (stderr, "method.id %x is not AMQP_BASIC_DELIVER_METHOD",
	  frame->payload.method.id);
    } else {
      d = (amqp_basic_deliver_t *) frame->payload.method.decoded;
      printf("Delivery %u, exchange %.*s routingkey %.*s\n",
             (unsigned) d->delivery_tag,
             (int) d->exchange.len, (char *) d->exchange.bytes,
             (int) d->routing_key.len, (char *) d->routing_key.bytes);
    }
    return 0;
}

int
recvApiReplyProp (amqp_connection_state_t conn, amqp_frame_t *frame)
{
    amqp_basic_properties_t *p;
    int status;

    status = amqp_simple_wait_frame(conn, frame);
    if (status < 0) {
      fprintf(stderr, 
        "recvApiReplyProp: amqp_simple_wait_frame failed. status %d\n", status);
      return status;
    }

    if (frame->frame_type != AMQP_FRAME_HEADER) {
      fprintf (stderr, 
        "recvApiReplyProp: frame_type %d is not AMQP_FRAME_HEADER", 
        frame->frame_type);
      return -1;
    }
    p = (amqp_basic_properties_t *) frame->payload.properties.decoded;

    if (p != NULL) 
      return 0;
    else 
      return -1;
}

int
recvApiReplyBody (amqp_connection_state_t conn, int channel,
amqp_frame_t *frame, bytesBuf_t **extBytesBuf)
{
    size_t body_target;
    size_t body_received;
    amqp_basic_deliver_t *d;
    int status;


    *extBytesBuf = NULL;
    body_target = frame->payload.properties.body_size;
    body_received = 0;

    while (body_received < body_target) {
      status = amqp_simple_wait_frame(conn, frame);
      if (status < 0)
        break;

      if (frame->frame_type != AMQP_FRAME_BODY) {
        fprintf(stderr, "Expected body!");
        return -1;
      }

      body_received += frame->payload.body_fragment.len;

#if 0
      amqp_dump(frame->payload.body_fragment.bytes,
                frame->payload.body_fragment.len);
#endif
    }
    d = (amqp_basic_deliver_t *) frame->payload.method.decoded;
    amqp_basic_ack(conn, channel, d->delivery_tag, 0);
    if (body_received != body_target) {
      /* Can only happen when amqp_simple_wait_frame returns <= 0 */
      /* We break here to close the connection */
      fprintf (stderr, "body_received %d != body_target %d",
        (int) body_received, (int) body_target);
        return -1;
    }
    return 0;
}

int initOoiReqProp (amqp_basic_properties_t *props, char *replyToStr,
char *receiver, char *op, char *format, char *ts)
{
  amqp_table_entry_t *entries;
  bzero (props, sizeof (amqp_basic_properties_t));
  props->_flags = AMQP_BASIC_CONTENT_TYPE_FLAG | 
    AMQP_BASIC_DELIVERY_MODE_FLAG | AMQP_BASIC_HEADERS_FLAG;
#if 0
    props.content_type = amqp_cstring_bytes("text/plain");
#endif
  props->delivery_mode = 2; /* persistent delivery mode */
  props->headers.num_entries = NUM_OOI_HEADER_ENTRIES;

  props->headers.entries = entries = (amqp_table_entry_t *) calloc 
	(NUM_OOI_HEADER_ENTRIES, sizeof (amqp_table_entry_t));

  entries[0].key = amqp_cstring_bytes("protocol");
  entries[0].value.kind = AMQP_FIELD_KIND_UTF8;
  entries[0].value.value.bytes = amqp_cstring_bytes("rpc");

  entries[1].key = amqp_cstring_bytes("sender-name");
  entries[1].value.kind = AMQP_FIELD_KIND_UTF8;
  entries[1].value.value.bytes = amqp_cstring_bytes("service_gateway");

  entries[2].key = amqp_cstring_bytes("encoding");
  entries[2].value.kind = AMQP_FIELD_KIND_UTF8;
  entries[2].value.value.bytes = amqp_cstring_bytes("msgpack");

  entries[3].key = amqp_cstring_bytes("reply-by");
  entries[3].value.kind = AMQP_FIELD_KIND_UTF8;
  entries[3].value.value.bytes = amqp_cstring_bytes("todo");

  entries[4].key = amqp_cstring_bytes("expiry");
  entries[4].value.kind = AMQP_FIELD_KIND_UTF8;
  entries[4].value.value.bytes = amqp_cstring_bytes("0");

  entries[5].key = amqp_cstring_bytes("ion-actor-id");
  entries[5].value.kind = AMQP_FIELD_KIND_UTF8;
  entries[5].value.value.bytes = amqp_cstring_bytes("anonymous");

  entries[6].key = amqp_cstring_bytes("reply-to");
  entries[6].value.kind = AMQP_FIELD_KIND_UTF8;
  entries[6].value.value.bytes = amqp_cstring_bytes(replyToStr);

  entries[7].key = amqp_cstring_bytes("conv-id");
  entries[7].value.kind = AMQP_FIELD_KIND_UTF8;
  entries[7].value.value.bytes = amqp_cstring_bytes("one_8347-1");

  entries[8].key = amqp_cstring_bytes("conv-seq");
  entries[8].value.kind = AMQP_FIELD_KIND_I32;
  entries[8].value.value.i32 = 10;

  entries[9].key = amqp_cstring_bytes("origin-container-id");
  entries[9].value.kind = AMQP_FIELD_KIND_UTF8;
  entries[9].value.value.bytes = amqp_cstring_bytes("one_8347");

  entries[10].key = amqp_cstring_bytes("sender");
  entries[10].value.kind = AMQP_FIELD_KIND_UTF8;
  entries[10].value.value.bytes = amqp_cstring_bytes("one_8347");

  entries[11].key = amqp_cstring_bytes("language");
  entries[11].value.kind = AMQP_FIELD_KIND_UTF8;
  entries[11].value.value.bytes = amqp_cstring_bytes("ion-r2");

  entries[12].key = amqp_cstring_bytes("sender-type");
  entries[12].value.kind = AMQP_FIELD_KIND_UTF8;
  entries[12].value.value.bytes = amqp_cstring_bytes("service");

  entries[13].key = amqp_cstring_bytes("sender-service");
  entries[13].value.kind = AMQP_FIELD_KIND_UTF8;
  entries[13].value.value.bytes = amqp_cstring_bytes("ion_one,service_gateway");

  entries[14].key = amqp_cstring_bytes("sender-service");
  entries[14].value.kind = AMQP_FIELD_KIND_UTF8;
  entries[14].value.value.bytes = amqp_cstring_bytes("ion_one,service_gateway");

  entries[15].key = amqp_cstring_bytes("ts");
  entries[15].value.kind = AMQP_FIELD_KIND_UTF8;
  entries[15].value.value.bytes = amqp_cstring_bytes(ts);

  entries[16].key = amqp_cstring_bytes("receiver");
  entries[16].value.kind = AMQP_FIELD_KIND_UTF8;
  entries[16].value.value.bytes = amqp_cstring_bytes(receiver);

  entries[17].key = amqp_cstring_bytes("format");
  entries[17].value.kind = AMQP_FIELD_KIND_UTF8;
  entries[17].value.value.bytes = amqp_cstring_bytes(format);

  entries[18].key = amqp_cstring_bytes("performative");
  entries[18].value.kind = AMQP_FIELD_KIND_UTF8;
  entries[18].value.value.bytes = amqp_cstring_bytes("request");

  entries[19].key = amqp_cstring_bytes("op");
  entries[19].value.kind = AMQP_FIELD_KIND_UTF8;
  entries[19].value.value.bytes = amqp_cstring_bytes(op);

  return 0;
}

int
printAmqpHeaders (amqp_table_t *headers)
{
    int i;
    char outstr[1024];
    int len;
    char *ptr;

    if (headers < 0) return -1;

    printf ("headers: { ");    
    for (i = 0; i < headers->num_entries; i++) {
        len = headers->entries[i].key.len;
        ptr = (char *) headers->entries[i].key.bytes;
        if (len >= 1024) {
              printf ("entry %d length too long\n", len);
        } else {
             strncpy (outstr, ptr, len);
             outstr[len] = '\0';
             printf ("%s: ", outstr);
        }
	switch (headers->entries[i].value.kind) {
          case AMQP_FIELD_KIND_I16:
              printf ("%d", headers->entries[i].value.value.i16);
              break;
          case AMQP_FIELD_KIND_U16:
              printf ("%d", headers->entries[i].value.value.u16);
              break;
          case AMQP_FIELD_KIND_I32:
              printf ("%d", headers->entries[i].value.value.i32);
              break;
          case AMQP_FIELD_KIND_U32:
              printf ("%d", headers->entries[i].value.value.u32);
              break;
          case AMQP_FIELD_KIND_I64:
              printf ("%d", headers->entries[i].value.value.i64);
              break;
          case AMQP_FIELD_KIND_U64:
              printf ("%d", headers->entries[i].value.value.u64);
              break;
          case AMQP_FIELD_KIND_F32:
              printf ("%f", headers->entries[i].value.value.f32);
              break;
          case AMQP_FIELD_KIND_F64:
              printf ("%f", headers->entries[i].value.value.f64);
              break;
          case AMQP_FIELD_KIND_UTF8:
              len = headers->entries[i].value.value.bytes.len;
              ptr = (char *) headers->entries[i].value.value.bytes.bytes;
              if (len >= 1024) {
		  printf ("entry %d length too long\n", len); 
              } else {
                  strncpy (outstr, ptr, len);
                  outstr[len] = '\0';
                  printf ("%s", outstr); 
              }
              break;
          default:
              printf ("UNKNOW type %d", headers->entries[i].value.kind);
        }
        if (i < headers->num_entries - 1) printf (", ");
    }
    printf ("  }\n");
    return 0;
}

