import json

from pika import BlockingConnection, ConnectionParameters, BasicProperties


class BrokerClient(object):

    REQUEST_EXCHANGE = "request"
    STATUS_EXCHANGE = "status"
    DEFAULT_BROKER_URL = "127.0.0.1"

    def __init__(self, broker_url=DEFAULT_BROKER_URL):
        self.connection = BlockingConnection(ConnectionParameters(broker_url))
        self.channel = self.connection.channel()

        self.channel.exchange_declare(exchange=self.REQUEST_EXCHANGE,
                                      exchange_type="topic")

        self.channel.exchange_declare(exchange=self.STATUS_EXCHANGE,
                                      exchange_type="fanout")

    def close(self):
        self.connection.close()

    def request_write(self,
                      output_prefix,
                      metadata=None,
                      detectors=None,
                      bsread_channels=None,
                      epics_pvs=None):

        routing_key = "."

        if detectors:
            for detector in detectors:
                routing_key += detector + "."

        if bsread_channels:
            routing_key += "bsread" + "."

        if epics_pvs:
            routing_key += "epics" + "."

        body_bytes = json.dumps({
            "output_prefix": output_prefix,
            "metadata": metadata,
            "detectors": detectors,
            "bsread_channels": bsread_channels,
            "epics_pvs": epics_pvs
        }).encode()

        self.channel.basic_publish(exchange=self.REQUEST_EXCHANGE,
                                   routing_key=routing_key,
                                   body=body_bytes)

        status_header = {
            "action": "write_request",
            "source": "BrokerClient",
            "routing_key": routing_key
        }

        self.channel.basic_publish(exchange=self.STATUS_EXCHANGE,
                                   properties=BasicProperties(
                                       headers=status_header),
                                   routing_key="",
                                   body=body_bytes)


broker = BrokerClient()
broker.request_write("/tmp/test", epics_pvs=["test"])
broker.close()
