/**
 * streamingservice.hpp
 * Defines the data types and Service for price streams.
 *
 * @authors Breman Thuraisingham, Mingsen Wang
 */
#ifndef STREAMING_SERVICE_HPP
#define STREAMING_SERVICE_HPP

#include "soa.hpp"
#include "algo_streaming_service.hpp"
#include <unordered_map>
#include <string>

template<typename T>
class AlgoStreamingToStreamingListener;

/**
 * Streaming service to publish two-way prices.
 * Keyed on product identifier.
 * Type T is the product type.
 */
template<typename T>
class StreamingService : public Service<string,PriceStream <T> > {
private:
    unordered_map<string, PriceStream<T>> price_streams_;
    ServiceListener<AlgoStream<T>>* in_listener_;

public:
    
    StreamingService();
    ~StreamingService();
    
    // MARK: SERVICE CLASS OVERRIDE BELOW
    // Get data on our service given a key (orderbook)
    virtual PriceStream<T>& GetData(string product_id) override;
    
    // The callback that a Connector should invoke for any new or updated data
    virtual void OnMessage(PriceStream<T>& data) override;
    
    // Add a listener to the Service for callbacks on add, remove, and update events
    // for data to the Service.
    virtual void AddListener(ServiceListener<PriceStream<T>>* listener) override;
    
    // Get all listeners on the Service.
    virtual const vector<ServiceListener<PriceStream<T>>*>& GetListeners() const override;
    // MARK: SERVICE CLASS OVERRIDE ABOVE
    
    // Get the listener of the service
    ServiceListener<AlgoStream<T>>* GetInListener();

    // Publish two-way prices
    void PublishPrice(const PriceStream<T>& priceStream);

};

template<typename T>
class AlgoStreamingToStreamingListener : public ServiceListener<AlgoStream<T>> {
private:
    StreamingService<T>* service_;
    
public:
    AlgoStreamingToStreamingListener(StreamingService<T>* service);
    ~AlgoStreamingToStreamingListener() = default;
    
    // MARK: SERVICELISTENER CLASS OVERRIDE BELOW
    // Listener callback to process an add event to the Service
    virtual void ProcessAdd(AlgoStream<T> &data) override;

    // Listener callback to process a remove event to the Service
    virtual void ProcessRemove(AlgoStream<T> &data) override;

    // Listener callback to process an update event to the Service
    virtual void ProcessUpdate(AlgoStream<T> &data) override;
    // MARK: SERVICELISTENER CLASS OVERRIDE ABOVE
};

template<typename T>
StreamingService<T>::StreamingService() {
    in_listener_ = new AlgoStreamingToStreamingListener<T>(this);
}

template<typename T>
StreamingService<T>::~StreamingService() {
    delete in_listener_;
}

template <typename T>
PriceStream<T>& StreamingService<T>::GetData(string product_id) {
    return price_streams_[product_id];
}

template <typename T>
void StreamingService<T>::OnMessage(PriceStream<T>& data) {
    string product_id = data.GetProduct().GetProductId();
    
    price_streams_[product_id] = data;
    
//    // Also notify listeners
//    for (auto& listener : Service<string, Price<T>>::listeners_) {
//        listener->ProcessAdd(data);
//    }
}

template <typename T>
void StreamingService<T>::AddListener(ServiceListener<PriceStream<T>>* listener) {
    this->Service<string, PriceStream<T>>::AddListener(listener);
}

template <typename T>
const vector<ServiceListener<PriceStream<T>>*>& StreamingService<T>::GetListeners() const {
    return this->Service<string, PriceStream<T>>::GetListeners();
}

template <typename T>
ServiceListener<AlgoStream<T>>* StreamingService<T>::GetInListener() {
    return in_listener_;
}

template <typename T>
void StreamingService<T>::PublishPrice(const PriceStream<T> &price_stream) {
    for (auto& listener : this->GetListeners()) {
        listener.ProcessAdd(price_stream);
    }
}

template<typename T>
AlgoStreamingToStreamingListener<T>::AlgoStreamingToStreamingListener(StreamingService<T>* service) : service_(service) {}

template<typename T>
void AlgoStreamingToStreamingListener<T>::ProcessAdd(AlgoStream<T>& data) {
    PriceStream<T>* price_stream = data.GetPriceStream();
    service_->OnMessage(*price_stream);
    service_->PublishPrice(*price_stream);
}

template<typename T>
void AlgoStreamingToStreamingListener<T>::ProcessRemove(AlgoStream<T>& data) {}

template<typename T>
void AlgoStreamingToStreamingListener<T>::ProcessUpdate(AlgoStream<T>& data) {}

#endif
