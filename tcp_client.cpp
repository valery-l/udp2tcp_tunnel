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
        cout << "Bingo, connected!" << endl;

        sock_ = in_place(
            ref(sock),
            bind(&client::on_receive, this, _1, _2),
            bind(&client::on_disconnected, this),
            trace_error);

        cock_the_clock(1);
    }

    void cock_the_clock(size_t seconds)
    {
        timer_.expires_from_now(boost::posix_time::seconds(seconds));
        timer_.async_wait(bind(&client::on_tick, this, _1));
    }

    void on_tick(const error_code& code)
    {
        if (code)
        {
            cout << "supressing timer error" << endl;
            //trace_error(code);
            return;
        }

        cock_the_clock(1);

        static size_t counter = 0;
        test_msg_data msg(++counter);

        sock_->send(&msg, sizeof(msg));
        cout << "client has sent: " << msg.counter << endl;
    }

    void on_receive(const void* /*data*/, size_t /*size*/)
    {
        // do something... or not
        cout << "Some data has been received - didn't expect!" << endl;
    }

    void on_disconnected()
    {
        cout << "Olala, remote host has been disconnected" << endl;

//        timer_.cancel();
//        sock_ .reset ();
    }

private:
    optional<tcp_fragment_socket>   sock_; // temp solution
    deadline_timer                  timer_;
};

void run_tcp_client(string server, size_t port)
{
    io_service io;
    client cl(io, server, port);

    io.run();
}


