#include "common.h"

struct cmdline_error
        : std::runtime_error
{
    cmdline_error(std::string err)
        :  runtime_error(err)
    {
    }
};

size_t get_port(string port_str)
{
    try
    {
        return lexical_cast<size_t>(port_str);
    }
    catch (bad_lexical_cast)
    {
        throw cmdline_error("incorrect port");
    }

}

void parse_cmdline(int argc, char**argv, bool& is_server, string& server, size_t& port)
{
    if (argc < 2)
        throw cmdline_error("not enought arguments");

    string address = argv[1];
    is_server = false;

    if (string(argv[1]) == "-s")
    {
        address   = argv[2];
        is_server = true;
    }

    vector<string> tokens;
    split(tokens, address, is_any_of(":"), boost::token_compress_on);

    if (tokens.size() != 2)
        throw cmdline_error("invalid server:port format");

    server = tokens[0];
    port   = get_port(tokens[1]);
}

int main(int argc, char** argv)
{
    try
    {
        string server;
        size_t port;
        bool   is_server;

        parse_cmdline(argc, argv, is_server, server, port);

        if (is_server)
            run_server(server, port);
        else
            run_client(server, port);
    }
    catch (cmdline_error const& e)
    {
        cout << "invalid parametes: " << e.what() << endl;

        cout << "usage: asio_test -s <server>:<port> or"    << endl
             << "       asio_test    <server>:<port>" << endl;

        return 1;

    }
    catch (std::exception const& e)
    {
        cout << "c++ exception caught: " << e.what() << endl;
        return 1;
    }
    catch (...)
    {
        cout << "unknown eception caught" << endl;
        return 1;
    }
}
