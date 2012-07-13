#include <common.h>

void run_client(string server, size_t port)
{
    using namespace boost::asio;
    using namespace boost::asio::ip;

    using namespace boost::posix_time;

    io_service io;

    udp::socket s(io, udp::v4());
    s.set_option(socket_base::broadcast(true));

    udp::resolver res(io);
    udp::resolver::query q(udp::v4(), server, lexical_cast<string>(port));

    udp::endpoint remote_adr = *res.resolve(q);

    //connect(s, res.resolve(q));

    deadline_timer t(io, seconds(1));

    const size_t msgs_at_time = 10;
    const size_t delay_ms     = 50;

    for(size_t count = 0;; ++count)
    {
        if (count % msgs_at_time == 0)
        {
            t.wait();
            t.expires_at(t.expires_at() + milliseconds(delay_ms));
        }

        std::vector<char> x(101, 0);
        for (size_t i = 0; i < x.size(); ++i)
            x[i] = 32 + (i % 224);

        //string msg = str(format("%d message from a client") % count);
        s.send_to(buffer(x), remote_adr);

        if (count % (msgs_at_time * (1000 / delay_ms)) == 0)
            cout << count << " messages has been sent" << endl;
    }
}
