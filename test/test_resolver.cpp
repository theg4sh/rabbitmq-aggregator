#include <string>
#include <iostream>
#include <assert.h>

#include "resolver.hpp"

bool testConnectionUri(std::string connectionUri, bool isValid)
{
    AmqpResolver resolver(connectionUri);
    bool res = resolver.verify();
    if (isValid == res) {
        std::cout << "PASS: " << connectionUri << std::endl;
        return true;
    }
    const char* vnv[] = {"valid", "invalid"};

    std::cerr << "FAIL: Uri '" << connectionUri << "' is "
        << vnv[(int)isValid] << ", but got " << vnv[(int)res] << std::endl;
    //std::cout << (int)resolver.verify() << std::endl;
    return false;
}

int main(int, char**)
{
    assert(testConnectionUri("amqp://guest:guest@localhost:5672/vhost", true));

    assert(testConnectionUri("amqp://guest@localhost:5672/vhost", true));

    assert(testConnectionUri("amqp://@localhost:5672/vhost", false));

    assert(testConnectionUri("amqp://@localhost:/vhost", false));

    assert(testConnectionUri("amqp://localhost:/vhost", true));

    assert(testConnectionUri("amqp://localhost:7456/vhost", true));
}
