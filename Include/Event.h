#pragma once
#include <algorithm>
#include <functional>
#include <vector>

//登録された関数（ハンドラ）を、Invoke 時に全部まとめて呼ぶ（C# の event に近い）
template<typename... Args>
class Event
{
public:
    using HandlerId = std::size_t;
    using Handler = std::function<void(Args...)>;

    //解除に使う ID を返す
    HandlerId Subscribe(Handler handler)
    {
        const HandlerId handlerId = _nextHandlerId++;
        _handlerEntries.push_back({ handlerId, std::move(handler) });
        return handlerId;
    }

    void Unsubscribe(HandlerId handlerId)
    {
        _handlerEntries.erase(
            std::remove_if(_handlerEntries.begin(), _handlerEntries.end(),
                [handlerId](const HandlerEntry& entry) { return entry.id == handlerId; }),
            _handlerEntries.end());
    }

    void Clear() { _handlerEntries.clear(); }

    //登録されている関数をすべて呼ぶ
    void Invoke(const Args&... args) const
    {
        //呼び出し中に Subscribe / Unsubscribe されても壊れないよう、コピーしてから回す
        const std::vector<HandlerEntry> handlerSnapshot = _handlerEntries;
        for (const HandlerEntry& entry : handlerSnapshot)
        {
            entry.handler(args...);
        }
    }

private:
    struct HandlerEntry
    {
        HandlerId id;
        Handler   handler;
    };

    std::vector<HandlerEntry> _handlerEntries;
    HandlerId _nextHandlerId = 1;
};