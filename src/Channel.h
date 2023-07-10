#pragma once

#include "EventLoop.h"
#include "Timestamp.h"
#include "noncopyable.h"

#include <functional>
#include <memory>

class EventLoop;
class Timestamp;

// 理清楚  EventLoop、Channel、Poller之间的关系   《= Reactor模型上对应 Demultiplex
// Channel 理解为通道，封装了sockfd和其感兴趣的event，如EPOLLIN、EPOLLOUT事件
// 还绑定了poller返回的具体事件
class Channel : noncopyable{
public:
    using EventCallBack = std::function<void()>;
    using ReadEventCallBack = std::function<void(Timestamp)>;

    Channel(EventLoop* loop,int fd);
    ~Channel();

    // fd得到poller通知以后，处理事件的
    void handleEvent(Timestamp receiveTime);

    // 设置回调函数对象
    void setReadCallback(ReadEventCallBack cb) { readCallback_ = std::move(cb); };
    void setWriteCallback(EventCallBack cb) { writeCallback_ = std::move(cb); };
    void setCloseCallback(EventCallBack cb) { closeCallback_ = std::move(cb); };
    void setErrorCallback(EventCallBack cb) { errorCallback_ = std::move(cb); };

    // 防止当channel被手动remove掉，channel还在执行回调操作
    void tie(const std::shared_ptr<void>&);

    int fd() const {return fd_; }
    int events() const { return events_; }
    void set_revents(int revt) { revents_ = revt; }

    // 设置fd相应的事件状态
    void enableReading() { events_ |= KReadEvent; update(); }
    void disableReading() { events_ &= ~KReadEvent; update(); }
    void enableWriting() { events_ |= KWriteEvent; update(); }
    void disableWriting() { events_ &= ~KWriteEvent; update(); }
    void disableAll() { events_ = KNoneEvent; update(); }

    // 返回fd当前的事件状态
    bool isNoneEvnet() const { return events_ == KNoneEvent; }
    bool isReading() const { return events_ & KReadEvent; }
    bool isWriting() const { return events_ & KWriteEvent; }

    int index() { return index_; }
    void set_index(int idx) { index_ = idx; }

    // one loop per thread
    EventLoop* ownerLoop() { return loop_; }
    void remove();
private:
    void update();
    void handleEventWithGuard(Timestamp receiveTime);

    static const int KNoneEvent;
    static const int KReadEvent;
    static const int KWriteEvent;

    EventLoop* loop_; // 事件循环
    const int fd_;  // fd, Poller监听的对象
    int events_; // 注册fd感兴趣的事件
    int revents_;  // poller返回的具体发生的事件
    int index_;

    std::weak_ptr<void> tie_; // 监控另外一个对象的生命周期
    bool tied_;

    // 因为channel通道里面能够获知fd最终发生的具体的事件revents，所以它负责调用具体事件的回调操作
    ReadEventCallBack readCallback_;
    EventCallBack writeCallback_;
    EventCallBack closeCallback_;
    EventCallBack errorCallback_;
};