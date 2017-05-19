#ifndef __NETIO_TCP_LISTENER_H__
#define __NETIO_TCP_LISTENER_H__


#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>


namespace netio {


typedef boost::shared_ptr<boost::asio::ip::tcp::socket> socket_ptr;


/// tcp listener template class
///
/// Session concept:
///
///     class Session
///     {
///     public:
///         sesstion_type(socket_ptr socket);
///         void start();
///     };
///
template <class Session>
class tcp_listener
{
    typedef tcp_listener<Session>   this_type;
    typedef Session                 session_type;

public:
    tcp_listener(boost::asio::io_service& ios, int port)
        : io_service_(ios)
        , acceptor_(ios, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
    {
    }

    void open()
    {
        io_service_.post(boost::bind(&this_type::async_accept, this));
    }

    void close()
    {
        acceptor_.cancel();
        acceptor_.close();
        io_service_.stop();
    }

private:
    void async_accept()
    {
        auto socket = boost::make_shared<boost::asio::ip::tcp::socket>(io_service_);
        acceptor_.async_accept(*socket,
            boost::bind(&this_type::handle_accept, this, socket,
                boost::asio::placeholders::error));
    }

    void handle_accept(socket_ptr socket, boost::system::error_code ec)
    {
        if (!ec)
        {
            auto session = boost::make_shared<session_type>(socket);
            session->start();
            async_accept();
        }
    }

private:
    boost::asio::io_service& io_service_;
    boost::asio::ip::tcp::acceptor acceptor_;
};


} // namespace netio

#endif // __NETIO_TCP_LISTENER_H__


