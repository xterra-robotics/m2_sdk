#ifndef __DDS_SUBSCRIBER_HPP__
#define __DDS_SUBSCRIBER_HPP__

#include <dds/dds.hpp>
#include <functional>
#include <iostream>
#include <thread>
// using namespace dds::sub;
// using namespace dds::domain;
// using namespace dds::topic;

namespace xterra {
    
template <typename T>
class DDSSubscriber {
   public:
    // Define a callback type for user-provided callbacks

    using UserCallback = std::function<void(const T&)>;

   private:
    dds::domain::DomainParticipant participant;
    dds::topic::Topic<T> topic;
    dds::sub::Subscriber subscriber;
    dds::sub::DataReader<T> reader;
    dds::sub::qos::DataReaderQos qos;
    T latestMessage;
    UserCallback userCallback;

    bool m_message_received = false;

    // Internal Listener class
    class CustomListener : public dds::sub::NoOpDataReaderListener<T> {
       private:
        DDSSubscriber& parent;

       public:
        CustomListener(DDSSubscriber& parent) : parent(parent) {}

        void on_data_available(dds::sub::DataReader<T>& reader) override {
            // Take all available data
            auto samples = reader.take();
            if (samples.length() > 0) {
                for (const auto& sample : samples) {
                    const auto& info = sample.info();
                    if (info.valid()) {
                        parent.m_message_received = true;
                        parent.latestMessage =
                            sample->data();  // Update the latest message
                        // If a user callback is provided, execute it
                        if (parent.userCallback) {
                            parent.userCallback(parent.latestMessage);
                        }
                    }
                }
            }
        }
    };

   public:
    // Constructor
    DDSSubscriber(const std::string& topicName, UserCallback callback = nullptr,
                  const int id = 0)
        : participant(id),
          topic(participant, topicName),
          subscriber(participant),
          userCallback(callback),
          reader(subscriber, topic, dds::sub::qos::DataReaderQos(),
                 new CustomListener(*this),
                 dds::core::status::StatusMask::data_available()) {}

    void setBestEffortQoS() {
        reader.close();
        qos << dds::core::policy::detail::Reliability::BestEffort();
        // reader.qos(qos);
        reader = dds::sub::DataReader<T>(
            subscriber, topic, qos, new CustomListener(*this),
            dds::core::status::StatusMask::data_available());
    }

    // Method to get the latest message
    T getLatestMessage() const { return latestMessage; }

    bool messageReceived() const { return m_message_received; }
};

}
#endif
