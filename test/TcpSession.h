#ifndef __TCPSESSION_H__
#define __TCPSESSION_H__


#include "netio/tcp_listener.hpp"



class CTcpSession : public boost::enable_shared_from_this<CTcpSession>
{
public:
    CTcpSession(netio::socket_ptr socket)
        : mSocket(socket)
    {
        printf("this(%p), CTcpSession::ctor()\n", this);
    }

    ~CTcpSession()
    {
        printf("this(%p), CTcpSession::dtor()\n", this);
    }

    void start()
    {
        printf("this(%p), CTcpSession::start()\n", this);
        async_read();
    }

private:
    void async_read()
    {
        printf("this(%p), CTcpSession::async_read()\n", this);
        mBuffer.resize(4096);
        mSocket->async_read_some(boost::asio::buffer(mBuffer),
            boost::bind(&CTcpSession::handle_read, shared_from_this(),
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
    }

    void handle_read(boost::system::error_code const& ec, size_t bytes_transferred)
    {
        printf("this(%p), CTcpSession::handle_read()\n", this);
        printf("this(%p), ec(%d) bytes(%d)\n", this, ec.value(), bytes_transferred);
        if (!ec)
        {
            printf("this(%p), buf(%s)\n", this, &mBuffer[0]);
        }
    }

private:
    netio::socket_ptr mSocket;
    std::vector<char> mBuffer;
};


#endif // __TCPSESSION_H__

