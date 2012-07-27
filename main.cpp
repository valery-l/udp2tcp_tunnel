#include "common.h"
#include "tunnel.h"
#include "config_parser.h"

int main(int argc, char** argv)
{
    try
    {
        if (argc != 2)
        {
            cerr << "invalid config" << endl;
            cout << "usage: udp2tcp_tunnel <config>" << endl;
            return 1;
        }

        tunnel tun;

        parse_config(argv[1], tun);
        tun.run();

        return 0;
    }
    catch (cfg_error const& e)
    {
        cout << "invalid parametes: " << e.what() << endl;
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
