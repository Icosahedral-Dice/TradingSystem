//
//  algo_streaming_service.h
//  TradingSystem
//
//  Created by 王明森 on 12/22/22.
//

#ifndef algo_streaming_service_hpp
#define algo_streaming_service_hpp

#include "price_stream.hpp"
#include "soa.hpp"
#include <unordered_map>

// Wrapper for price stream for AlgoStreamingService,
//   just like AlgoExecutionOrder for AlgoExecutionService
template<typename T>
class AlgoStream {
private:
    PriceStream<T>* price_stream_;

public:
    AlgoStream() = default;
    AlgoStream(const T& product, const PriceStreamOrder& bid_order, const PriceStreamOrder& offer_order);
    ~AlgoStream();

    
    PriceStream<T>* GetPriceStream() const;
};

template<typename T>
AlgoStream<T>::AlgoStream(const T& product, const PriceStreamOrder& bid_order, const PriceStreamOrder& offer_order) {
    price_stream_ = new PriceStream<T>(product, bid_order, offer_order);
}

template<typename T>
AlgoStream<T>::~AlgoStream() {
    // delete price_stream_;
}

template<typename T>
PriceStream<T>* AlgoStream<T>::GetPriceStream() const {
    return price_stream_;
}

template<typename T>
class PricingToAlgoStreamingListener;

template<typename T>
class AlgoStreamingService : public Service<string, AlgoStream<T>> {
private:
    unordered_map<string, AlgoStream<T>> algo_streams_;
    ServiceListener<Price<T>>* in_listener_;
    long count_;
    
public:
    AlgoStreamingService();
    ~AlgoStreamingService();
    
    // MARK: SERVICE CLASS OVERRIDE BELOW
    // Get data on our service given a key (orderbook)
    virtual AlgoStream<T>& GetData(string product_id) override;
    
    // The callback that a Connector should invoke for any new or updated data
    virtual void OnMessage(AlgoStream<T>& data) override;
    
    // Add a listener to the Service for callbacks on add, remove, and update events
    // for data to the Service.
    virtual void AddListener(ServiceListener<AlgoStream<T>>* listener) override;
    
    // Get all listeners on the Service.
    virtual const vector<ServiceListener<AlgoStream<T>>*>& GetListeners() const override;
    // MARK: SERVICE CLASS OVERRIDE ABOVE
    
    // Get the listener of the service
    ServiceListener<Price<T>>* GetInListener();
    
    void AlgoPublishPrice(Price<T>& price);
};

template<typename T>
class PricingToAlgoStreamingListener : public ServiceListener<Price<T>> {
private:
    AlgoStreamingService<T>* service_;

public:
    PricingToAlgoStreamingListener(AlgoStreamingService<T>* service);
    ~PricingToAlgoStreamingListener() = default;

    // MARK: SERVICELISTENER CLASS OVERRIDE BELOW
    // Listener callback to process an add event to the Service
    virtual void ProcessAdd(Price<T> &data) override;

    // Listener callback to process a remove event to the Service
    virtual void ProcessRemove(Price<T> &data) override;

    // Listener callback to process an update event to the Service
    virtual void ProcessUpdate(Price<T> &data) override;
    // MARK: SERVICELISTENER CLASS OVERRIDE ABOVE

};

template<typename T>
AlgoStreamingService<T>::AlgoStreamingService() {
    in_listener_ = new PricingToAlgoStreamingListener<T>(this);
    count_ = 0;
}

template<typename T>
AlgoStreamingService<T>::~AlgoStreamingService() {
    delete in_listener_;
}

template <typename T>
AlgoStream<T>& AlgoStreamingService<T>::GetData(string product_id) {
    return algo_streams_[product_id];
}

template <typename T>
void AlgoStreamingService<T>::OnMessage(AlgoStream<T>& data) {
    string product_id = data.GetPriceStream()->GetProduct().GetProductId();
    
    algo_streams_[product_id] = data;
    
//    // Also notify listeners
//    for (auto& listener : Service<string, Price<T>>::listeners_) {
//        listener->ProcessAdd(data);
//    }
}

template <typename T>
void AlgoStreamingService<T>::AddListener(ServiceListener<AlgoStream<T>>* listener) {
    this->Service<string, AlgoStream<T>>::AddListener(listener);
}

template <typename T>
const vector<ServiceListener<AlgoStream<T>>*>& AlgoStreamingService<T>::GetListeners() const {
    return this->Service<string, AlgoStream<T>>::GetListeners();
}

template <typename T>
ServiceListener<Price<T>>* AlgoStreamingService<T>::GetInListener() {
    return in_listener_;
}

template<typename T>
void AlgoStreamingService<T>::AlgoPublishPrice(Price<T>& price)
{
    T product = price.GetProduct();
    string product_id = product.GetProductId();

    double mid = price.GetMid();
    double spread = price.GetBidOfferSpread();
    double bid_price = mid - spread / 2.;
    double offer_price = mid + spread / 2.;
    long visible_quantity = ((count_++) % 2 + 1) * 1000000;  // Alternate visble sizes
    long hidden_quantity = visible_quantity * 2;

    PriceStreamOrder bid_order(bid_price, visible_quantity, hidden_quantity, BID);
    PriceStreamOrder offer_order(offer_price, visible_quantity, hidden_quantity, OFFER);
    AlgoStream<T> algo_stream(product, bid_order, offer_order);
    algo_streams_[product_id] = algo_stream;

    for (auto& listener : this->GetListeners())
    {
        listener->ProcessAdd(algo_stream);
    }
}

template<typename T>
PricingToAlgoStreamingListener<T>::PricingToAlgoStreamingListener(AlgoStreamingService<T>* service) : service_(service) {}

template<typename T>
void PricingToAlgoStreamingListener<T>::ProcessAdd(Price<T>& data) {
    service_->AlgoPublishPrice(data);
}

template<typename T>
void PricingToAlgoStreamingListener<T>::ProcessRemove(Price<T>& data) {}

template<typename T>
void PricingToAlgoStreamingListener<T>::ProcessUpdate(Price<T>& data) {}

#endif /* algo_streaming_service_hpp */
