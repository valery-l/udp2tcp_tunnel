#include "common.h"
#include "async_timer.h"

struct async_timer::impl
    : enable_shared_from_this<impl>
{
    typedef shared_ptr<impl> ptr_t;

    static ptr_t create(io_service& io, on_timer_f const& on_timer)
    {
        return ptr_t(new impl(io, on_timer));
    }

    void wait(posix_time::time_duration const& dtime)
    {
        cancel();
        to_skip_.push_front(false);

        timer_.expires_from_now(dtime);
        timer_.async_wait(boost::bind(&impl::on_timer, shared_from_this(), _1, to_skip_.begin()));
    }

    void cancel()
    {
        for(bool& skip : to_skip_)
            skip = true;

        timer_.cancel();
    }

    // just for test
    ~impl()
    {
        cout << "to_skip: " << to_skip_.size() << endl;
    }


private:
    impl(io_service& io, on_timer_f const& on_timer)
        : timer_    (io)
        , on_timer_ (on_timer)
    {
    }

    void on_timer(error_code const& code, list<bool>::iterator to_skip)
    {
        bool need_skip = *to_skip;
        to_skip_.erase(to_skip);

        if (need_skip)
            return;

        assert(!code);
        on_timer_();
    }

private:
    deadline_timer  timer_;
    on_timer_f      on_timer_;
    list<bool>      to_skip_;
};


////////////////////////////////////////////////////////////////////////
async_timer::async_timer(io_service& io, on_timer_f const& on_timer)
    : pimpl_(impl::create(io, on_timer))
{
}

async_timer::~async_timer()
{
    cancel();
}

void async_timer::wait(posix_time::time_duration const& dtime)
{
    pimpl_->wait(dtime);
}

void async_timer::cancel()
{
    pimpl_->cancel();
}
