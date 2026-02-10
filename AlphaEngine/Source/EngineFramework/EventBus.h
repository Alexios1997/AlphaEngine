#pragma once

#include <vector>
#include <functional>
#include "Event.h"

namespace AlphaEngine {

    using EventCallbackFn = std::function<void(Event&)>;


    // We use our Event BUs for engine-wide events (Global Engine Events, UI triggers)
    // No coupling iterates through a list of listeners though
    class EventBus {
    private:
        static EventBus& Get() {
            static EventBus instance;
            return instance;
        }
        std::vector<EventCallbackFn> m_Subscribers;

    public:
      

        // Systems call this to "listen" to the engine
        static void Subscribe(const EventCallbackFn& callback) {
            Get().m_Subscribers.push_back(callback);
        }

        // The Application calls this to "broadcast"
        static void Publish(Event& event) {
            for (auto& callback : Get().m_Subscribers) {
                if (event.Handled) break;
                callback(event);
            }
        }
    };
}