#include <zmq.hpp>

int main(){
    zmq::context_t context(1);
    zmq::socket_t  subscriber(context, ZMQ_SUB);
    subscriber.setsockopt(ZMQ_SUBSCRIBE, "", 0);
    subscriber.connect("tcp://localhost:5555");

    while(true) {
        zmq::message_t msg;
        subscriber.recv(&msg);
        printf("%.*s", msg.size(), msg.data());
    }
    
}
