#pragma once

struct enable_cancel_resource
{
    template<class resource>
    explicit enable_cancel_resource(resource& res)
        : on_cancel_(bind(&resource::cancel, ref(res)))
        , cancelled_(false)
    {}

    void cancel()
    {
        cancelled_ = true;

        if (on_cancel_)
            on_cancel_();
    }

    void detach_resource()
    {
        on_cancel_.clear();
    }

    bool cancelled() const
    {
        return cancelled_;
    }

private:
    typedef function<void()> on_cancel_f;

private:
    on_cancel_f on_cancel_;
    bool        cancelled_;
};

template<class T>
struct auto_cancel_ptr
{
    typedef shared_ptr<T> ptr_t;

    explicit auto_cancel_ptr(ptr_t ptr = ptr_t())
    {
        reset(ptr);
    }

   ~auto_cancel_ptr()
    {
        reset();
    }

    void reset(ptr_t ptr = ptr_t())
    {
        if (ptr_)
        {
            ptr_->cancel();
            ptr_.reset();
        }

        ptr_ = ptr;
    }

    T& operator* () const { return *ptr_; }
    T* operator->() const { return ptr_.get(); }
    T* get       () const { return ptr_.get(); }

private:
    ptr_t ptr_;
};
