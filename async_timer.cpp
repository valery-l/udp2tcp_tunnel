#include "common.h"
#include "async_timer.h"

struct async_timer::impl
    : enable_cancel_resource
{
    typedef shared_ptr<impl> ptr_t;

    static ptr_t create(io_service& io, on_timer_f const& on_timer, posix_time::time_duration const& dtime)
    {
        ptr_t ptr(new impl(io, on_timer));

        ptr->timer_.expires_from_now(dtime);
        ptr->timer_.async_wait(boost::bind(&impl::on_timer, ptr, _1));

        return ptr;
    }

private:
    impl(io_service& io, on_timer_f const& on_timer)
        : enable_cancel_resource(timer_)

        , timer_    (io)
        , on_timer_ (on_timer)
    {
    }

    void on_timer(error_code const& code)
    {
        if(cancelled() || code)
            return;

        on_timer_();
    }

private:
    deadline_timer  timer_;
    on_timer_f      on_timer_;
};


////////////////////////////////////////////////////////////////////////
async_timer::async_timer(io_service& io, on_timer_f const& on_timer)
    : io_       (io)
    , on_timer_ (on_timer)
{
}

async_timer::~async_timer()
{
}

void async_timer::wait(posix_time::time_duration const& dtime)
{
    pimpl_.reset(impl::create(io_, on_timer_, dtime));
}

void async_timer::cancel()
{
    pimpl_.reset();
}
