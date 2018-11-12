import pika

if __name__ == '__main__':
    connection = pika.BlockingConnection(pika.ConnectionParameters("localhost"))
    channel = connection.channel()

    count = 100000;

    for i in range(1):
        channel.basic_publish(exchange='exchange-out', routing_key='', body='demo'+str(i));

    for i in range(count):
        channel.basic_publish(exchange='', routing_key='queue-B', body='partB'+str(i));

    for i in range(count):
        channel.basic_publish(exchange='', routing_key='queue-A', body=('partA'+str(i))*100);

    connection.close()
