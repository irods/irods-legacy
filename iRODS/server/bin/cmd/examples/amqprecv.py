#!/usr/bin/env python
import sys
import pika

host = sys.argv[1]
queue = sys.argv[2]

connection = pika.BlockingConnection(pika.ConnectionParameters(
        host=host))
channel = connection.channel()

channel.queue_declare(queue=queue)

mf, hf, body = channel.basic_get(queue=queue, no_ack=True)

if mf.name == 'Basic.GetEmpty':
    sys.stdout.write('')
else:
    sys.stdout.write(body)
    sys.stdout.write('\n')

connection.close()
exit()

