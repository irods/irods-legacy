#!/usr/bin/env python
import sys
import pika

host = sys.argv[1]
queue = sys.argv[2]
body = sys.argv[3]

connection = pika.BlockingConnection(pika.ConnectionParameters(
        host=host))
channel = connection.channel()

channel.queue_declare(queue=queue)

channel.basic_publish(exchange='',
                      routing_key=queue,
                      body=body)
connection.close()
