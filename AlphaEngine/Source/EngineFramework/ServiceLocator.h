#pragma once

#include <unordered_map>
#include <typeindex>
#include <memory>
#include <cassert>
#include <any>
#include <vector>

namespace AlphaEngine 
{

    class IService
    {
    public:
        virtual ~IService() = default;
        virtual void InitService() = 0;
    };

    // We use a ServiceLocator to have the Renderer, ECS , maybe audio , scripting or any service
    // in one Place and so we can get them without passing for the layers for example
    // all these. Cleaner and more modular
    class ServiceLocator {

    public:
        // Register a service (Renderer, ECS ++)
        template<typename T>
        static void Provide(T* service) {
            //  Ensure T actually inherits from IService at compile time
            static_assert(std::is_base_of_v<IService, T>, "T must inherit from IService");

            //We store the specific system as a general IService pointer
            m_Services[typeid(T)] = static_cast<IService*>(service);

            m_Services[typeid(T)]->InitService();
        }

        // Retrieve a service
        template<typename T>
        static T& Get() {
            auto it = m_Services.find(typeid(T));
            assert(it != m_Services.end() && "Service not provided!");
            return *static_cast<T*>(it->second);
        }

    private:
        // We use raw pointers here because the Application owns the memory, 
        // the Locator just "points" to it.
        inline static std::unordered_map<std::type_index, IService*> m_Services;
    };
}