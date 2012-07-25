#pragma once

typedef function<void()> on_timer_f;

struct async_timer
{
    async_timer(io_service& io, on_timer_f const& on_timer);
   ~async_timer();

    void wait(posix_time::time_duration const& dtime);
    void cancel();

private:
    struct impl;
    shared_ptr<impl> pimpl_;
};

