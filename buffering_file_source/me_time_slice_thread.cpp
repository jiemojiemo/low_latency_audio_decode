
//
// Created by William.Hua on 2021/1/26.
//

#include "me_time_slice_thread.h"

namespace mammon
{

TimeSliceThread::~TimeSliceThread() {
    stopThread();
}

void TimeSliceThread::addTimeSliceClient(TimeSliceClient* client){
    if(client){
        std::lock_guard<std::mutex> clients_lock(mutex_clients_);
        addTimeSliceClientIfNotExist(client);
    }
}

void TimeSliceThread::removeClient(TimeSliceClient* client){
    if(client == client_being_called_){
        std::lock_guard<std::mutex> lg(mutex_callback_);
        removeTimeSliceClientIfExist(client);
    }else{
        removeTimeSliceClientIfExist(client);
    }
}

void TimeSliceThread::clearClients() {
    clients_.clear();
}

int TimeSliceThread::getNumClient() const {
    return clients_.size();
}

TimeSliceClient* TimeSliceThread::getClient(int index){
    std::lock_guard<std::mutex> clients_lock(mutex_clients_);
    return clients_[index];
}

bool TimeSliceThread::isThreadRunning() const{
    return thread_ != nullptr;
}

void TimeSliceThread::startThread(){
    auto* thread_temp = new std::thread(&TimeSliceThread::run,this);
    auto* old_thread = thread_.exchange(thread_temp);

    if(old_thread){
        throw std::runtime_error("the previous thread was not stopped");
    }

    should_exit_.store(false);
}

void TimeSliceThread::stopThread(){
    auto* current_thread = thread_.exchange(nullptr);
    if(current_thread && current_thread->joinable()){
        should_exit_.store(true);
        current_thread->join();
        delete current_thread;
    }
}

void TimeSliceThread::run() {
    int index = 0;
    while(!should_exit_){
        int num_clients = 0;
        {
            std::lock(mutex_callback_, mutex_clients_);
            std::lock_guard<std::mutex> callback_lock(mutex_callback_, std::adopt_lock);
            std::lock_guard<std::mutex> clients_lock(mutex_clients_, std::adopt_lock);

            num_clients = clients_.size();
            if(num_clients > 0){
                index = num_clients > 0 ? ((index + 1) % num_clients) : 0;
                client_being_called_ = getNextClient(index);
                if(client_being_called_ != nullptr){
                    int ms_until_next_call = client_being_called_->useTimeSlice();
                    if(ms_until_next_call >= 0){
                        client_being_called_->next_call_time = std::chrono::steady_clock::now() + std::chrono::milliseconds{ms_until_next_call};
                    }else{
                        removeTimeSliceClientIfExist(client_being_called_);
                    }
                }
            }

        }

    }
}

void TimeSliceThread::addTimeSliceClientIfNotExist(TimeSliceClient* client){
    auto is_exist = std::find(clients_.begin(), clients_.end(), client) != clients_.end();
    if(!is_exist){
        client->next_call_time = std::chrono::steady_clock::now();
        clients_.push_back(client);
    }
}

void TimeSliceThread::removeTimeSliceClientIfExist(TimeSliceClient* client){
    auto iter = std::find(clients_.begin(), clients_.end(), client);
    if(iter != clients_.end()){
        clients_.erase(iter);
    }
}

TimeSliceClient* TimeSliceThread::getNextClient(int index){

    auto soonest = std::chrono::steady_clock::now();
    TimeSliceClient* client = nullptr;

    // find the nearest client
    for(int i = clients_.size(); --i >= 0;){
        auto* c = clients_[ (i + index) % clients_.size()];

        if(client == nullptr || c->next_call_time < soonest){
            client = c;
            soonest = c->next_call_time;
        }
    }

    return client;
}
}