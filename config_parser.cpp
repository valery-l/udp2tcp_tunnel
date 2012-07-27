#include "common.h"
#include "tunnel.h"
#include "config_parser.h"

namespace
{
using namespace network;
using namespace boost;
using namespace boost::algorithm;

struct config_parser
{
    config_parser(string filename, tunnel& t)
        : tunnel_(t)
    {
        std::ifstream in(filename);

        while(in.good())
        {
            const size_t bufsize = 0x400;

            char buf[bufsize];
            in.getline(buf, bufsize);

            string str = buf;
            trim(str);

            if (str.empty())
                continue;

            if (starts_with(str, "tcp"))
                process_tcp(str);

            else if (starts_with(str, "#"))
                ; // skip - it's a comment

            else
                process_udp_point(str);
        }
    }

private:
    void process_tcp(string str)
    {
        vector<string> tokens;
        split(tokens, str, boost::is_any_of("=:"), boost::token_compress_on);

        if (tokens.front() == "tcp_client")
        {
            if (tokens.size() < 3 || tokens.size() > 4)
                throw cfg_error("invalid tcp client description: " + str);

            endpoint point(tokens[1] + ":" + tokens[2]);

            if (tokens.size() == 3)
                tunnel_.create_tcp_sender(true, point);

            if (tokens.size() == 4)
                tunnel_.create_tcp_sender(true, point, boost::posix_time::seconds(lexical_cast<size_t>(tokens[3])));
        }
        else if (tokens.front() == "tcp_server")
        {
            if (tokens.size() != 3)
                throw cfg_error("invalid tcp server description: " + str);

            tunnel_.create_tcp_sender(false, endpoint(tokens[1] + ":" + tokens[2]));
        }
        else
            throw cfg_error("invalid tcp point description: " + tokens.front());
    }

    void process_udp_point(string str)
    {
        vector<string> tokens;
        split(tokens, str, boost::is_any_of("->"), boost::token_compress_on);

        if (tokens.size() < 2 || tokens.size() > 3)
            throw cfg_error("invalid udp point description: " + str);

        if (tokens.size() == 2)
            tunnel_.create_udp_receiver(endpoint(tokens[0]), endpoint(tokens[1]));

        if (tokens.size() == 3)
            tunnel_.create_udp_receiver(endpoint(tokens[0]), endpoint(tokens[2]), tokens[1]);
    }

private:
    tunnel& tunnel_;
};


} // 'anonymous'

void parse_config(string filename, tunnel& t)
{
    config_parser(filename, t);
}
