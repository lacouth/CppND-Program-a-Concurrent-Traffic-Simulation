#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

 
template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
    std::unique_lock lock(_mutex);
    _conditional.wait(lock,[this]{return !_queue.empty();});
    T message = std::move(_queue.back());
    
    return message;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::unique_lock lock(_mutex);
    _queue.emplace_back(std::move(msg));
    _conditional.notify_one();
}


/* Implementation of class "TrafficLight" */

 
TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    TrafficLightPhase phase;
    do{
        phase = _queue.receive();
    }while(phase != TrafficLightPhase::green);

}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
    std::thread t(&TrafficLight::cycleThroughPhases,this);
    threads.emplace_back(std::move(t));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<double> uni(4.0,6.0);
    auto random_time = uni(mt);

    auto initStopWatch = std::chrono::high_resolution_clock::now();
    while(true){
        auto now = std::chrono::high_resolution_clock::now();
        auto difference = std::chrono::duration_cast<std::chrono::seconds>(now - initStopWatch).count();

        if(difference >= random_time){

            _currentPhase = _currentPhase == TrafficLightPhase::red ? TrafficLightPhase::green : TrafficLightPhase::red;
            _queue.send(std::move(_currentPhase));
            initStopWatch = std::chrono::high_resolution_clock::now();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}
