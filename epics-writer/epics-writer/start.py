import json

from pika import BlockingConnection, ConnectionParameters


def on_message(channel, method_frame, header_frame, body):
    request = json.loads(body.decode())
    epics_pvs = request.get("epics_pvs")

    if not epics_pvs:
        channel.basic_reject(delivery_tag=method_frame.delivery_tag,
                             requeue=False)

        # TODO: Send to status exchange.


    channel.basic_ack(delivery_tag=method_frame.delivery_tag)


def connect_to_broker(broker_url, exchange_name, queue_name):
    connection = BlockingConnection(ConnectionParameters(broker_url))
    channel = connection.channel()

    channel.exchange_declare(exchange=exchange_name,
                             exchange_type="topic")

    channel.queue_declare(queue=queue_name, auto_delete=True)
    channel.queue_bind(queue=queue_name,
                       exchange=exchange_name,
                       routing_key="*.%s.*" % queue_name)

    channel.basic_consume(queue_name, on_message)

    try:
        channel.start_consuming()
    except KeyboardInterrupt:
        channel.stop_consuming()


BROKER_URL = "127.0.0.1"
EXCHANGE_NAME = "request"
QUEUE_NAME = "epics"


def main():
    import argparse
    parser = argparse.ArgumentParser(description='Epics HDF5 writer')
    parser.add_argument('--broker_url', dest='broker_url',
                        default=BROKER_URL,
                        help='RabbitMQ broker URL')
    parser.add_argument('--exchange_name', dest='exchange_name',
                        default=EXCHANGE_NAME,
                        help='Name of the request exchange.')
    parser.add_argument('--queue_name', dest='queue_name',
                        default=QUEUE_NAME,
                        help='Name of the queue to connect to')

    args = parser.parse_args()

    connect_to_broker(broker_url=args.broker_url,
                      exchange_name=args.exchange_name,
                      queue_name=args.queue_name)


if __name__ == '__main__':
    main()
