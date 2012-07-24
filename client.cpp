#include "common.h"
#include "network.h"

struct client
{
    client(io_service& io, string server, short port)
        : timer_(io)
        , port_ (port)
    {
        network::connect(io, network::endpoint(server, port), bind(&client::on_connected, this, _1), trace_error);
        //cock_the_clock(1);
    }

private:
    void on_connected(tcp::socket& sock)
    {
        cout << "Bingo, connected!" << endl;

        tcp_sock_ = in_place(
            ref(sock),
            bind(&client::on_receive, this, _1, _2),
            bind(&client::on_disconnected, this),
            trace_error);

        udp_sock_ = in_place(
            ref(sock.get_io_service()),
            network::endpoint(ip::address_v4::broadcast(), port_ + 1),
            none,
            bind(&client::on_receive, this, _1, _2),
            trace_error);

        cock_the_clock(1);
    }

    void cock_the_clock(size_t seconds)
    {
        // test
        timer_.expires_from_now(boost::posix_time::milliseconds(10 * seconds));
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

        //udp_sock_->send(&msg, sizeof(msg));
        tcp_sock_->send(&msg, sizeof(msg));
        //cout << "client has sent: " << msg.counter << endl;
    }

    void on_receive(const void* data, size_t size)
    {
        if (size != sizeof(test_msg_data))
            cout << "Size is wrong, got " << size << " should be: " << sizeof(test_msg_data) << endl;

        assert(size == sizeof(test_msg_data));

        test_msg_data const& msg = *reinterpret_cast<test_msg_data const*>(data);
        cout << "Received back from server: " << msg.counter << endl;

        if (last_counter_)
            assert(msg.counter - *last_counter_ == 1);

        last_counter_ = msg.counter;
    }

    void on_disconnected()
    {
        cout << "Olala, remote host has been disconnected" << endl;

        timer_.cancel();
        tcp_sock_ .reset();
        udp_sock_ .reset();
    }

private:
    optional<network::tcp_fragment_wrapper> tcp_sock_;
    optional<network::udp_socket          > udp_sock_;
    deadline_timer                          timer_;
    optional<size_t>                        last_counter_;
    const size_t                            port_;
};

///////////////////////////////////////////////////
// ping test
/*
void on_pinged(io_service& io, string server, bool success, posix_time::milliseconds const& delay);

void start_ping(io_service& io, string server)
{
    network::ping(io, server, posix_time::milliseconds(500), bind(&on_pinged, ref(io), server, _1, _2));
}

void on_pinged(io_service& io, string server, bool success, posix_time::milliseconds const& delay)
{
    cout << "ping " << (success ? "succedded" : "failed") << " timeout: " << delay << endl;

    deadline_timer timer(io);
    timer.expires_from_now(posix_time::seconds(1));
    timer.wait();

    start_ping(io, server);
}
*/

void run_tcp_client(string server, size_t port)
{
    io_service io;
    client cl(io, server, port);

    //start_ping(io, server);

    io.run();
}


