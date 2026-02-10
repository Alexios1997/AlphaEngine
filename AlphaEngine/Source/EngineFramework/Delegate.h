#pragma once


// Usage:
// Delegate<Player, float> onHit(&playerEntity, &Player::TakeDamage);
// onHit.Invoke(10.0f);

template<typename T, typename... Args>
class Delegate
{
    // (T::*) - > A pointer to a function that belongs to Class T
	using InsideFunction = void (T::*)(Args...);
	T* m_Instance;
	InsideFunction m_Function;

public:
    Delegate(T* instance, InternalFunction func)
        : m_Instance(instance), m_Function(func) {
    }

    void Call(Args... args) {
        (m_Instance->*m_Function)(std::forward<Args>(args)...);
    }
};