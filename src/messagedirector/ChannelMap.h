#pragma once
#include <list>
#include <set>
#include <unordered_map>
#include <mutex>
#include "core/types.h"
#include <boost/icl/interval_map.hpp>

class ChannelSubscriber
{
  public:
    virtual ~ChannelSubscriber() {}

    inline std::set<channel_t> &channels()
    {
        return m_channels;
    }
    inline boost::icl::interval_set<channel_t> &ranges()
    {
        return m_ranges;
    }

  private:
    std::set<channel_t> m_channels; // The set of all individually subscribed channels.
    boost::icl::interval_set<channel_t> m_ranges; // The set of all subscribed channel ranges.
};

// ChannelMap is a convenience template. It provides functionality for mapping
// channels (as channel_t) to objects interested in that channel (represented as
// the template class).
class ChannelMap
{
  public:
    ChannelMap();

    // subscribe_channel adds a single channel to the mapping.
    // (Args) "c": the channel to be added.
    void subscribe_channel(ChannelSubscriber *p, channel_t c);

    // unsubscribe_channel removes a channel from the mapping.
    // (Args) "c": the channel to be removed.
    void unsubscribe_channel(ChannelSubscriber *p, channel_t c);

    // subscribe_range adds an object to be subscribed to a range of channels.
    // (Args) "lo": the lowest channel to be removed.
    //        "hi": the highest channel to be removed.
    // The range is inclusive.
    void subscribe_range(ChannelSubscriber *p, channel_t lo, channel_t hi);

    // unsubscribe_range undoes a subscribe_range.
    // (Args) "lo": the lowest channel to be removed.
    //        "hi": the highest channel to be removed.
    // The range is inclusive.
    void unsubscribe_range(ChannelSubscriber *p, channel_t lo, channel_t hi);

    // unsubscribe_all removes all channel and range subscriptions from a subscriber.
    void unsubscribe_all(ChannelSubscriber *p);

    // is_subscribed tests if a given object has a subscription on a channel.
    bool is_subscribed(ChannelSubscriber *p, channel_t c);

    // lookup_channel populates a set with the subscribers for a channel.
    void lookup_channel(channel_t c, std::set<ChannelSubscriber *> &ps);

    // lookup_channels is the same, but it works on a list of channels.
    void lookup_channels(const std::list<channel_t> &cl, std::set<ChannelSubscriber *> &ps);

  protected:
    virtual void on_add_channel(channel_t) { }

    virtual void on_remove_channel(channel_t) { }

    virtual void on_add_range(channel_t, channel_t) { }

    virtual void on_remove_range(channel_t, channel_t) { }

  private:
    // Single channel subscriptions
    std::unordered_map<channel_t, std::set<ChannelSubscriber *> > m_channel_subscriptions;

    // Range channel subscriptions
    boost::icl::interval_map<channel_t, std::set<ChannelSubscriber *> > m_range_subscriptions;

    // In order to make this object thread-safe...
    std::recursive_mutex m_lock;
};
