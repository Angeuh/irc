#include "../includes/Server.hpp"

void Server::addToEpoll(int fd, uint32_t events)
{
    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));

    ev.events = events;
    ev.data.fd = fd;

    if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) < 0)
        throw std::runtime_error("epoll add failed");
}

void Server::modifyEpoll(int fd, uint32_t events)
{
    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));

    ev.events = events;
    ev.data.fd = fd;

    if (epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev) < 0)
        throw std::runtime_error("epoll mod failed");
}

void Server::removeFromEpoll(int fd)
{
    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
}
