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
#include <set>
#include <map>

#include <boost/algorithm/string.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/date_time.hpp>
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

using std::cin;
using std::cerr;
using std::cout;
using std::endl;

using std::string;
using std::vector;
using std::list;
using std::array;
using std::queue;
using std::set;
using std::map;

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

namespace posix_time = boost::posix_time;

inline void trace_error_from_source(string source, error_code const& code)
{
    cerr
        << "Error: "
        << source
        << " -- "
        << code.message()
        << "; category: " << code.category().name()
        << "; value: "    << code.value()
        << endl;
}

typedef function<void (error_code const&)> on_error_f;
inline on_error_f error_tracer(string source)
{
    return bind(trace_error_from_source, source, _1);
}





