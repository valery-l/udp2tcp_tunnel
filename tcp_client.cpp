#include "common.h"
#include "asio_helper.h"
#include "tcp_service.h"

struct client
{
    client(io_service& io, string server, short port)
        : timer_(io)
    {
        connect(io, server, port, bind(&client::on_connected, this, _1, _2));
    }

private:
    void on_connected(tcp::socket& sock, error_code const& /*err*/)
    {
        sock_.reset(new tcp_fragment_socket(move(sock), boost::bind(&client::on_receive, this, _1, _2)));
        cock_the_clock(1);
    }

    void cock_the_clock(size_t seconds)
    {
        timer_.expires_from_now(boost::posix_time::seconds(seconds));
        timer_.async_wait(bind(&client::on_tick, this, _1));
    }

    void on_tick(const error_code& /*error*/)
    {
        cock_the_clock(1);

        static size_t counter = 0;
        test_msg_data msg(++counter);

        sock_->send(&msg, sizeof(msg));
        cout << "client has sent: " << msg.counter << endl;
    }

    void on_receive(const char* /*data*/, size_t /*size*/)
    {
        // do something... or not
    }

private:
    // todo: optional or not optional ???
    scoped_ptr<tcp_fragment_socket> sock_; // temp solution
    deadline_timer                  timer_;
};

void run_tcp_client(string server, short port)
{
    io_service io;
    client cl(io, server, port);

    io.run();
}


