#pragma once

#include <cmath>

#include <iostream>
#include <stdexcept>

#include <algorithm>

#include <string>
#include <vector>
#include <list>
#include <array>
#include <queue>

#include <boost/algorithm/string.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/noncopyable.hpp>
#include <boost/none.hpp>
#include <boost/optional.hpp>
#include <boost/thread/thread.hpp>
#include <boost/ref.hpp>
#include <boost/smart_ptr.hpp>

#include <boost/utility/in_place_factory.hpp>
#include <boost/utility/typed_in_place_factory.hpp>

using std::cout;
using std::endl;

using std::string;
using std::vector;
using std::list;
using std::array;
using std::queue;

using std::move;
using std::forward;

using boost::ref;
using boost::cref;

using boost::bind;
using boost::function;

using boost::lexical_cast;
using boost::bad_lexical_cast;

using boost::format;

using boost::optional;
using boost::in_place;

using boost::split;
using boost::is_any_of;

using boost::enable_shared_from_this;
using boost::noncopyable;

using boost::none;

using boost::system::error_code;
using boost::asio::ip::tcp;
using boost::asio::ip::udp;

using namespace boost::asio;

using boost::shared_ptr;
using boost::weak_ptr;
using boost::scoped_ptr;
using boost::make_shared;

void run_server(string remote_ip, size_t port);
void run_client(string local_ip , size_t port);

void run_tcp_server(string server, size_t port);
void run_tcp_client(string server, size_t port);

#pragma pack(push, 1)
struct test_msg_data
{
    size_t counter;
    char data[1 << 14];

    test_msg_data(size_t counter)
        : counter(counter)
    {
    }
};
#pragma pack(pop)

inline void trace_error(boost::system::error_code const& code)
{
    cout << "Err! Msg: " << code.message() << "; category: " << code.category().name() << "; value: " << code.value() << endl;
}




