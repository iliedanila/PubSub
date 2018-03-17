#ifndef _LOGICAL_LAYER_SUBSCRIBER_HPP_
#define _LOGICAL_LAYER_SUBSCRIBER_HPP_

#include <set>
#include <functional>
#include <map>

#include "messageVisitor.hpp"
#include "brokerIdentity.hpp"
#include "subscription.hpp"
#include "sendError.hpp"



namespace NetworkLayer {
    class DataMessage;
    class Node;
}

namespace LogicalLayer
{
    class PublisherData;

    class Subscriber
    {
    public:
        typedef std::function<void(PublisherData& publisherData)> PublisherDataHandlerT;

        explicit Subscriber(NetworkLayer::Node& node);
        ~Subscriber();

        void AddSubscription(SubscriptionT& subscription, PublisherDataHandlerT handler);
        void RemoveSubscription(SubscriptionT& subscription);

    private:
        friend struct MessageVisitor<Subscriber>;

        void HandleIncomingMessage(NetworkLayer::DataMessage& message);

        void SendSubscription(
            const SubscriptionT& subscription, 
            const std::string& brokerName,
            SubscriptionMessage::Action) const;

        void SendNewSubscription(SubscriptionT& subscription);
        void SendRemoveSubscription(SubscriptionT& subscription);
        void SendAllSubscriptions(const std::string& brokerName);
        void HandleNewBroker(BrokerIdentity& message);

        void HandleNewNodeStatus(const std::string nodeName, bool isAlive);

        void HandleBrokerAck(
            const std::string nodeName,
            NetworkLayer::SendError error) const;

        template<typename MessageT>
        void HandleMessage(MessageT& message);

        NetworkLayer::Node& node;
        std::set<std::string> brokers;
        std::map<SubscriptionT, PublisherDataHandlerT> subscriptions;
    };
}

#endif