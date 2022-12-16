/**
 * execution service.hpp
 * Defines the data types and Service for executions.
 *
 * @author Breman Thuraisingham, Mingsen Wang
 */



#ifndef EXECUTION_SERVICE_HPP
#define EXECUTION_SERVICE_HPP

#include <string>
#include "soa.hpp"
#include "market_data_service.hpp"
#include "algo_execution_service.hpp"
#include "execution_order.hpp"


template <typename T>
class AlgoExecutionToExecutionListener;

/**
 * Service for executing orders on an exchange.
 * Keyed on product identifier.
 * Type T is the product type.
 */
template<typename T>
class ExecutionService : public Service<string, ExecutionOrder <T> >
{
private:
    unordered_map<string, ExecutionOrder<T>> execution_orders_;
    AlgoExecutionToExecutionListener<T>* in_listener_;
    
public:
    ExecutionService();
    ~ExecutionService();
    
    // SERVICE CLASS OVERRIDE BELOW
    // Get data on our service given a key (orderbook)
    virtual ExecutionOrder<T>& GetData(string product_id) override;
    
    // The callback that a Connector should invoke for any new or updated data
    virtual void OnMessage(ExecutionOrder<T>& data) override;
    
    // Add a listener to the Service for callbacks on add, remove, and update events
    // for data to the Service.
    virtual void AddListener(ServiceListener<ExecutionOrder<T>>* listener) override;
    
    // Get all listeners on the Service.
    virtual const vector<ServiceListener<ExecutionOrder<T>>*>& GetListeners() const override;
    // SERVICE CLASS OVERRIDE ABOVE
    
    // Execute an order on a market
    void ExecuteOrder(const ExecutionOrder<T>& order, Market market);

};

template <typename T>
class AlgoExecutionToExecutionListener : public ServiceListener<AlgoExecutionOrder<T>> {
private:
    ExecutionService<T>* service_;
    
public:
    AlgoExecutionToExecutionListener(ExecutionService<T>* service);
    ~AlgoExecutionToExecutionListener() = default;
    
    // SERVICELISTENER CLASS OVERRIDE BELOW
    // Listener callback to process an add event to the Service
    virtual void ProcessAdd(AlgoExecutionOrder<T> &data) override;

    // Listener callback to process a remove event to the Service
    virtual void ProcessRemove(AlgoExecutionOrder<T> &data) override;

    // Listener callback to process an update event to the Service
    virtual void ProcessUpdate(AlgoExecutionOrder<T> &data) override;
    // SERVICELISTENER CLASS OVERRIDE ABOVE
};

template<typename T>
ExecutionService<T>::ExecutionService() {
    in_listener_ = new AlgoExecutionToExecutionListener<T>(this);
}

template<typename T>
ExecutionService<T>::~ExecutionService() {
    delete in_listener_;
}

template<typename T>
ExecutionOrder<T>& ExecutionService<T>::GetData(string product_id) {
    return execution_orders_.at(product_id);
}

template<typename T>
void ExecutionService<T>::OnMessage(ExecutionOrder<T>& data) {
    string product_id = data.GetProduct().GetProductId();
    
    execution_orders_.insert_or_assign(product_id, data);
    
    // Also notify listeners
    for (auto& listener : Service<string, OrderBook<T>>::listeners_) {
        listener->ProcessAdd(data);
    }
}

template<typename T>
void ExecutionService<T>::AddListener(ServiceListener<ExecutionOrder<T>>* listener) {
    this->Service<string, ExecutionOrder<T>>::AddListener(listener);
}

template<typename T>
const vector<ServiceListener<ExecutionOrder<T>>*>& ExecutionService<T>::GetListeners() const {
    return this->Service<string, ExecutionOrder<T>>::GetListeners();
}

template<typename T>
void ExecutionService<T>::ExecuteOrder(const ExecutionOrder<T>& order, Market market)
{
    string _productId = order.GetProduct().GetProductId();
    execution_orders_[_productId] = order;

    for (auto& l : Service<string, ExecutionOrder<T>>::listeners_)
    {
        l->ProcessAdd(order);
    }
}

template <typename T>
AlgoExecutionToExecutionListener<T>::AlgoExecutionToExecutionListener(ExecutionService<T>* service) : service_(service) {}

template<typename T>
void AlgoExecutionToExecutionListener<T>::ProcessAdd(AlgoExecutionOrder<T>& data)
{
    // Get the underlying execution order
    ExecutionOrder<T>* execution_order = data.GetExecutionOrder();
    
    // Push to underlying service (ExecutionService)
    service_->OnMessage(*execution_order);
    
    // Also request execution of the orde
    service_->ExecuteOrder(*execution_order);
}

template<typename T>
void AlgoExecutionToExecutionListener<T>::ProcessRemove(AlgoExecutionOrder<T>& data) {
    // Do nothing
}

template<typename T>
void AlgoExecutionToExecutionListener<T>::ProcessUpdate(AlgoExecutionOrder<T>& data) {
    // Do nothing
}

#endif
