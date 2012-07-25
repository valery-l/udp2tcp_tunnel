#pragma once
#include "net_common.h"
#include "auto_cancel.h"

namespace network
{
    typedef
        function<void(bool, posix_time::milliseconds const&)>
        on_pinged_f;

    struct async_pinger
    {
        async_pinger(io_service& io, string destination, posix_time::milliseconds const& timeout, on_pinged_f const& on_pinged);
       ~async_pinger();

    private:
        struct impl;
        auto_cancel_ptr<impl> pimpl_;
    };

} // namespace network
