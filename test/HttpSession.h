#ifndef __HTTPSESSION_H__
#define __HTTPSESSION_H__


#include <iostream>
#include <functional>
#include "netio/tcp_listener.hpp"
#include "netio/http_inbuf.hpp"



class CHttpSession : public boost::enable_shared_from_this<CHttpSession>
{
public:
    CHttpSession(netio::socket_ptr socket)
        : mSocket(socket)
    {
        printf("this(%p), CHttpSession::ctor()\n", this);
    }

    ~CHttpSession()
    {
        printf("this(%p), CHttpSession::dtor()\n", this);
    }

    void start()
    {
        printf("this(%p), CHttpSession::start()\n", this);
        async_read();
    }

private:
    void async_read()
    {
        printf("this(%p), CHttpSession::async_read()\n", this);
        auto thiz = shared_from_this();
        netio::http_inbuf_ptr inbuf = netio::http_inbuf::create();
        inbuf->async_read_from(*mSocket, [thiz, inbuf](boost::system::error_code ec) {
            printf("this(%p), CHttpSession::handle_read()\n", thiz.get());
            if (!ec)
            {
                auto msg = inbuf->headers();
                std::cout << *msg << std::endl;
            }
        });
    }

private:
    netio::socket_ptr mSocket;
};


#endif // __HTTPSESSION_H__

