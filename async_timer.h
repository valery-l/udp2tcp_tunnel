#pragma once
#include "auto_cancel.h"

typedef function<void()> on_timer_f;

struct async_timer
{
    async_timer(io_service& io, on_timer_f const& on_timer);
   ~async_timer();

    void wait(posix_time::time_duration const& dtime);
    void cancel();

private:
    struct impl;
    auto_cancel_ptr<impl> pimpl_;

private:
    io_service& io_;
    on_timer_f  on_timer_;
};

