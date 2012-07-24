#pragma once
#include "net_common.h"

namespace network
{

    typedef
        function<void(bool, posix_time::milliseconds const&)>
        on_pinged_f;

    void ping(io_service& io, string destination, posix_time::milliseconds const& timeout, on_pinged_f const& on_pinged);

} // namespace network
