#include "connection.hpp"
#include "allMessages.hpp"
#include "node.hpp"

#include <iostream>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/variant.hpp>

namespace NetworkLayer
{

Connection::Connection(
    Node& _node,
    boost::asio::io_service& _io_service,
    tcp::socket&& _socket,
    std::function<void(std::shared_ptr<Connection>)> _closeHandler)
:
    io_service(_io_service),
    socket(std::move(_socket)),
    closeHandler(_closeHandler),
    node(_node)
{
    std::cout << "Connection() in node " << node.Name() << "\n";
}

Connection::~Connection()
{
    std::cout << "~Connection() in node " << node.Name() << "\n";
}

void Connection::Read(ReadCallback _callback)
{
    auto self(shared_from_this());
    boost::asio::async_read(
        socket,
        boost::asio::buffer(readMessage, Node::MaxMessageSize),
        [this, self, _callback]
        (const boost::system::error_code& error, std::size_t length)
        {
            if( !error)
            {
                assert(length <= Node::MaxMessageSize);
                std::string content(readMessage, Node::MaxMessageSize);
                std::stringstream ss(content);
                boost::archive::binary_iarchive iarchive(ss);
                                    
                MessageVariant message;
                iarchive >> message;
                                    
                _callback(message, self);
                Read(_callback);
            }
            else
            {
                std::cout   << "Error reading body: "
                            << error.value()
                            << " "
                            << error.message()
                            << " on node "
                            << node.Name()
                            << "\n";
                closeHandler(shared_from_this());
            }
        });
}

void Connection::Write(
    MessageVariant _message, 
    std::function<void(boost::system::error_code)> _callback)
{
    auto self(shared_from_this());
    std::stringstream ss;
    boost::archive::binary_oarchive oarchive(ss);
    oarchive << _message;
    assert(ss.str().size() <= Node::MaxMessageSize);
    auto bufferData = std::make_shared<std::string>(std::move(ss.str()));

    boost::asio::async_write(
        socket,
        buffer(bufferData->c_str(), Node::MaxMessageSize),
        [self, _callback, bufferData]
        (boost::system::error_code error, std::size_t lenght)
        {
            _callback(error);
        });
}

void Connection::Close()
{
    socket.close();
}

}
