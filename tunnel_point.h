#pragma once

struct tunnel_point
{
    enum protocol_type {pt_udp, pt_tcp};

    tunnel_point(protocol_type type, string address, size_t port)
        : type      (type   )
        , address   (address)
        , port      (port   )
    {
    }

    protocol_type   type;
    string          address;
    size_t          port;
};

struct receiver
{
    virtual tunnel_point const& point() = 0;

    virtual ~receiver(){}
};
typedef shared_ptr<receiver> receiver_ptr;


struct sender
{
    virtual tunnel_point const& point() = 0;

    virtual void send(const void* data, size_t size) = 0;
    virtual void ~sender(){}
};
typedef shared_ptr<sender> sender_ptr;

namespace std
{
    template<>
    struct less<receiver_ptr>
            : binary_function<receiver_ptr, receiver_ptr, bool>
    {
        bool operator()(receiver_ptr const& lhs, receiver_ptr const& rhs) const { return lhs->point < rhs->point; }
    };

    template<>
    struct less<sender_ptr>
            : binary_function<sender_ptr, sender_ptr, bool>
    {
        bool operator()(sender_ptr const& lhs, sender_ptr const& rhs) const { return lhs->point < rhs->point; }
    };
} // namespace std
