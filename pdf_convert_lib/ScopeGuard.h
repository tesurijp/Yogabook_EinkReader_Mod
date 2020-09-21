#ifndef SCOPE_GUARD_H_
#define SCOPE_GUARD_H_
#include <functional>

class ScopeGuard
{
public:
    explicit ScopeGuard(std::function<void()> onExitScope) 
        : onExitScope_(onExitScope), dismissed_(false)
    { }

    ~ScopeGuard()
    {
        if(!dismissed_)
        {
            onExitScope_();
        }
    }

    ScopeGuard(ScopeGuard const&) = delete;
    ScopeGuard& operator=(ScopeGuard const&) = delete;

    void Dismiss()
    {
        dismissed_ = true;
    }

private:
    std::function<void()> onExitScope_;
    bool dismissed_;
};

#define SCOPEGUARD_LINENAME_CAT(name, line) name##line
#define SCOPEGUARD_LINENAME(name, line) SCOPEGUARD_LINENAME_CAT(name, line)

#define ON_SCOPE_EXIT(callback) ::ScopeGuard SCOPEGUARD_LINENAME(EXIT, __LINE__)(callback)

#endif

