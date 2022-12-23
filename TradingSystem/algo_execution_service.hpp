//
//  algo_execution_service.hpp
//  TradingSystem
//
//  Created by 王明森 on 12/14/22.
//

#ifndef algo_execution_service_hpp
#define algo_execution_service_hpp

#include "soa.hpp"
#include <string>
#include "market_data_service.hpp"
#include "execution_order.hpp"

// Wrapper for ExecutionOrder for use by AlgoExecutionService
// Specifies the ExecutionOrder and the market on which the ExecutionOrder is executed.
template <typename T>
class AlgoExecutionOrder {
private:
    ExecutionOrder<T>* order_;
    Market market_;
    
public:
    AlgoExecutionOrder() = default;
    AlgoExecutionOrder(const T &_product, PricingSide _side, string _orderId, OrderType _orderType, double _price, double _visibleQuantity, double _hiddenQuantity, string _parentOrderId, bool _isChildOrder, Market market);
    AlgoExecutionOrder(ExecutionOrder<T>& order, Market market);
    ~AlgoExecutionOrder();
    
    // Fetch the order
    ExecutionOrder<T>* GetExecutionOrder() const;
    
    // Fetch the market
    Market GetMarket() const;
};

template <typename T>
class MarketDataToAlgoExecutionListener;

template <typename T>
class AlgoExecutionService : public Service<string, AlgoExecutionOrder<T>> {
private:
    unordered_map<string, AlgoExecutionOrder<T>> algo_execution_orders_;
    MarketDataToAlgoExecutionListener<T>* in_listener_;
    double spread_;
    long execution_count_;
    
public:
    AlgoExecutionService();
    ~AlgoExecutionService();
    
    // MARK: SERVICE CLASS OVERRIDE BELOW
    // Get data on our service given a key (orderbook)
    virtual AlgoExecutionOrder<T>& GetData(string product_id) override;
    
    // The callback that a Connector should invoke for any new or updated data
    virtual void OnMessage(AlgoExecutionOrder<T>& data) override;
    
    // Add a listener to the Service for callbacks on add, remove, and update events
    // for data to the Service.
    virtual void AddListener(ServiceListener<AlgoExecutionOrder<T>>* listener) override;
    
    // Get all listeners on the Service.
    virtual const vector<ServiceListener<AlgoExecutionOrder<T>>*>& GetListeners() const override;
    // MARK: SERVICE CLASS OVERRIDE ABOVE
    
    MarketDataToAlgoExecutionListener<T>* GetInListener();
    
    // Execute an order on a market
    void AlgoExecute(OrderBook<T>& order_book, Market market = BROKERTEC);
};

template <typename T>
class MarketDataToAlgoExecutionListener : public ServiceListener<OrderBook<T>> {
private:
    AlgoExecutionService<T>* service_;
    
public:
    MarketDataToAlgoExecutionListener(AlgoExecutionService<T>* service);
    ~MarketDataToAlgoExecutionListener() = default;
    
    // MARK: SERVICELISTENER CLASS OVERRIDE BELOW
    // Listener callback to process an add event to the Service
    virtual void ProcessAdd(OrderBook<T> &data) override;

    // Listener callback to process a remove event to the Service
    virtual void ProcessRemove(OrderBook<T> &data) override;

    // Listener callback to process an update event to the Service
    virtual void ProcessUpdate(OrderBook<T> &data) override;
    // MARK: SERVICELISTENER CLASS OVERRIDE ABOVE
};

template <typename T>
AlgoExecutionOrder<T>::AlgoExecutionOrder(const T &_product, PricingSide _side, string _orderId, OrderType _orderType, double _price, double _visibleQuantity, double _hiddenQuantity, string _parentOrderId, bool _isChildOrder, Market market) : market_(market) {
    order_ = new ExecutionOrder<T>(_product, _side, _orderId, _orderType, _price, _visibleQuantity, _hiddenQuantity, _parentOrderId, _isChildOrder);
}

template <typename T>
AlgoExecutionOrder<T>::AlgoExecutionOrder(ExecutionOrder<T>& order, Market market) : order_(&order), market_(market) {}

template <typename T>
AlgoExecutionOrder<T>::~AlgoExecutionOrder() {
    delete order_;
}

template <typename T>
ExecutionOrder<T>* AlgoExecutionOrder<T>::GetExecutionOrder() const {
    return order_;
}

template <typename T>
Market AlgoExecutionOrder<T>::GetMarket() const {
    return market_;
}

template <typename T>
AlgoExecutionService<T>::AlgoExecutionService() : spread_(1. / 128.), execution_count_(0) {
    in_listener_ = new MarketDataToAlgoExecutionListener<T>(this);
}

template <typename T>
AlgoExecutionService<T>::~AlgoExecutionService() {
    delete in_listener_;
}

template <typename T>
AlgoExecutionOrder<T>& AlgoExecutionService<T>::GetData(string product_id) {
    return algo_execution_orders_[product_id];
}

template <typename T>
void AlgoExecutionService<T>::OnMessage(AlgoExecutionOrder<T>& data) {
    string product_id = data.GetExecutionOrder()->GetProduct().GetProductId();
    
    algo_execution_orders_.insert_or_assign(product_id, data);
    
//    // Also notify listeners
//    for (auto& listener : Service<string, AlgoExecutionOrder<T>>::listeners_) {
//        listener->ProcessAdd(data);
//    }
}

template <typename T>
void AlgoExecutionService<T>::AddListener(ServiceListener<AlgoExecutionOrder<T>>* listener) {
    this->Service<string, AlgoExecutionOrder<T>>::AddListener(listener);
}

template <typename T>
const vector<ServiceListener<AlgoExecutionOrder<T>>*>& AlgoExecutionService<T>::GetListeners() const {
    return this->Service<string, AlgoExecutionOrder<T>>::GetListeners();
}

template <typename T>
MarketDataToAlgoExecutionListener<T>* AlgoExecutionService<T>::GetInListener() {
    return in_listener_;
}

template <typename T>
void AlgoExecutionService<T>::AlgoExecute(OrderBook<T>& order_book, Market market) {
    T product = order_book.GetProduct();
    string product_id = product.GetProductId();
    PricingSide side;
    // TODO: Generate order id
    string order_id = "";
    double price;
    long quantity;
    
    // Get the current best bid/offer
    BidOffer bid_offer = order_book.GetBidOffer();
    Order bid_order = bid_offer.GetBidOrder();
    double bid_price = bid_order.GetPrice();
    long bid_quantity = bid_order.GetQuantity();
    Order offer_order = bid_offer.GetOfferOrder();
    double offer_price = offer_order.GetPrice();
    long offer_quantity = offer_order.GetQuantity();
    
    // If the spread is no more than the designated threshold, cross the spread alternatingly
    if (offer_price - bid_price <= spread_) {
        if (execution_count_ % 2) {
            price = offer_price;
            quantity = offer_quantity;
            side = OFFER;
        } else {
            price = bid_price;
            quantity = bid_quantity;
            side = BID;
        }
        execution_count_++;
        
        AlgoExecutionOrder<T> algo_execution_order(product, side, order_id, MARKET, price, quantity, 0, "", false, market);
        
        // Notify listeners
        for (auto& l : Service<string, AlgoExecutionOrder<T>>::listeners_) {
            l->ProcessAdd(algo_execution_order);
        }
    }
}

template<typename T>
MarketDataToAlgoExecutionListener<T>::MarketDataToAlgoExecutionListener(AlgoExecutionService<T>* service) : service_(service) {}

template<typename T>
void MarketDataToAlgoExecutionListener<T>::ProcessAdd(OrderBook<T>& data)
{
    // Request execution of the order
    service_->AlgoExecute(data);
}

template<typename T>
void MarketDataToAlgoExecutionListener<T>::ProcessRemove(OrderBook<T>& data) {
    // Do nothing
}

template<typename T>
void MarketDataToAlgoExecutionListener<T>::ProcessUpdate(OrderBook<T>& data) {
    // Do nothing
}

#endif /* algo_execution_service_hpp */
