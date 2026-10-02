#pragma once
#include <iostream>
#include <list>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <unordered_map>
#include <utility>

template<class TOwner>
class StateMachine {
public:
    class State {
    public:
        virtual ~State() = default;

    protected:
        // 継承先のStateから使える
        StateMachine<TOwner>& stateMachine() {
            return *_stateMachine;
        }

        TOwner& owner() {
            return _stateMachine->owner();
        }

        // 継承先で必要なものだけ override する
        virtual void OnEnter(State* previousState) {}
        virtual void OnUpdate() {}
        virtual void OnFixedUpdate() {}
        virtual void OnExit(State* nextState) {}

    private:
        friend class StateMachine<TOwner>;

        StateMachine<TOwner>* _stateMachine = nullptr;

        std::unordered_map<int, State*> _transitions;
        std::unordered_map<State*, int> _reverseTransitions;

        // StateMachineだけが呼べる
        void Enter(State* previousState) {
            OnEnter(previousState);
        }

        void Update() {
            OnUpdate();
        }

        void FixedUpdate() {
            OnFixedUpdate();
        }

        void Exit(State* nextState) {
            OnExit(nextState);
        }
    };

    class AnyState final : public State {};

    explicit StateMachine(TOwner owner)
        : _owner(std::move(owner)) {
    }

    TOwner& owner() {
        return _owner;
    }

    const TOwner& owner() const {
        return _owner;
    }

    State* currentState() const {
        return _currentState;
    }

    int nextStateEventId() const {
        return _nextStateEventId;
    }

    template<class TState>
    TState& Add() {
        static_assert(std::is_base_of_v<State, TState>);

        auto state = std::make_unique<TState>();
        state->stateMachine_ = this;

        TState& result = *state;
        _statesList.push_back(std::move(state));
        return result;
    }

    template<class TFrom, class TTo>
    void AddTransition(int eventId) {
        TFrom& from = GetOrAddState<TFrom>();

        if (from.transitions_.contains(eventId)) {
            std::cerr << "すでに登録されています。\n";
            return;
        }

        TTo& to = GetOrAddState<TTo>();
        from.transitions_.emplace(eventId, &to);
        from.reverseTransitions_.emplace(&to, eventId);
    }

    template<class TTo>
    void AnyAddTransition(int eventId) {
        AddTransition<AnyState, TTo>(eventId);
    }

    template<class TFirst>
    void Start() {
        Start(GetOrAddState<TFirst>());
    }

    void Start(State& firstState) {
        _currentState = &firstState;
        _currentState->Enter(nullptr);
    }

    void Dispatch(int eventId) {
        if (_currentState == nullptr) {
            throw std::logic_error("StateMachine is not started.");
        }

        State* nextState = nullptr;

        auto current = _currentState->transitions_.find(eventId);
        if (current != _currentState->transitions_.end()) {
            nextState = current->second;
        }
        else {
            AnyState& any = GetOrAddState<AnyState>();
            auto anyTransition = any.transitions_.find(eventId);

            if (anyTransition == any.transitions_.end()) {
                std::cerr << "Current state has no event: "
                    << eventId << '\n';
                return;
            }

            nextState = anyTransition->second;
        }

        _nextStateEventId = eventId;
        Change(*nextState);
    }

    void Update() {
        if (_currentState != nullptr) {
            _currentState->Update();
        }
    }

    void FixedUpdate() {
        if (_currentState != nullptr) {
            _currentState->FixedUpdate();
        }
    }

private:
    template<class TState>
    TState& GetOrAddState() {
        static_assert(std::is_base_of_v<State, TState>);

        for (const auto& state : _statesList) {
            if (auto* result = dynamic_cast<TState*>(state.get())) {
                return *result;
            }
        }

        return Add<TState>();
    }

    void Change(State& nextState) 
    {
        _currentState->Exit(&nextState);
        nextState.Enter(_currentState);
        _currentState = &nextState;
    }

    TOwner _owner;
    State* _currentState = nullptr; // statesList_が所有するため非所有ポインタ
    int _nextStateEventId = -1;

    std::list<std::unique_ptr<State>> _statesList;
};