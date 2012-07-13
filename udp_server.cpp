#include "common.h"

using namespace boost::asio;
using namespace boost::asio::ip;

struct udp_listener
{
    udp_listener(io_service& io, string /*local_ip*/, size_t port)
        : sock_(
              io,
              udp::endpoint(
                  //ip::address::from_string(local_ip),
                  udp::v4(),
                  port))
        , buf_      (max_msg, 0)
        , received_ (0)
    {
        sock_.set_option(socket_base::broadcast(true));
        cout << "server socket is bound to: " << sock_.local_endpoint() << endl;
        request_receive_data();
    }

private:
    void request_receive_data()
    {
        sock_.async_receive(buffer(buf_), boost::bind(&udp_listener::on_receive, this, _1, _2));
    }

    void on_receive(boost::system::error_code const& e, size_t size)
    {
        if (!e || e == boost::asio::error::message_size)
        {
            ++received_;

            if (true)
            {
                string s(buf_.begin(), buf_.begin() + std::min(size, buf_.size()));
                cout << received_ << ": " << size << " bytes received: " << s << endl;
            }

            request_receive_data();
        }
        else
            cout << "Error occuried while receiving data from ud socket: " << e.message() << endl;
    }

private:
    static const size_t max_msg = 200000;

private:
    udp::socket         sock_;
    std::vector<char>   buf_;
    size_t              port_;

    size_t              received_;
};

void run_server(string local_ip, size_t port)
{
    using namespace boost::posix_time;

    io_service io;
    udp_listener listener (io, local_ip, port);

    io.run();
}
