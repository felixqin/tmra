#ifndef __NETIO_TCP_CONNECTION_H__
#define __NETIO_TCP_CONNECTION_H__


#define BOOST_THREAD_PROVIDES_FUTURE
#define BOOST_THREAD_PROVIDES_FUTURE_CONTINUATION
#include <boost/thread/future.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>


namespace netio {



class tcp_connection;
typedef boost::shared_ptr<tcp_connection> tcp_connection_ptr;


class tcp_connection : public boost::enable_shared_from_this<tcp_connection>
{
    typedef tcp_connection this_type;

public:
    tcp_connection(boost::asio::io_service& ios)
        : socket_(ios)
    {
    }

    boost::asio::ip::tcp::socket& socket()
    {
        return socket_;
    }

    template <class Buffer>
    boost::future<int> read(Buffer const& buf)
    {
        boost::shared_ptr<boost::promise<int> > p = boost::make_shared<boost::promise<int> >();
        boost::future<int> f = p->get_future();
        socket_.get_io_service().post(
            boost::bind(&this_type::async_read<Buffer>, shared_from_this(),
                p, buf));
        return f;
    }

    void write();

private:
    template <class Buffer>
    void async_read(boost::shared_ptr<boost::promise<int> > p, Buffer const& buf)
    {
        boost::asio::async_read(socket_, buf,
            boost::bind(&this_type::handle_read, p,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
    }

    static void handle_read(boost::shared_ptr<boost::promise<int> > p, boost::system::error_code ec, size_t bytes_transferred)
    {
        if (ec)
        {
            p->set_value(ec.value());
        }
        else
        {
            p->set_value(bytes_transferred);
        }
    }

private:
    boost::asio::ip::tcp::socket socket_;
};


} // namespace netio

#endif // __NETIO_TCP_CONNECTION_H__


