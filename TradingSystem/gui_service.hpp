//
//  gui_service.hpp
//  TradingSystem
//
//  Created by 王明森 on 12/23/22.
//

#ifndef gui_service_hpp
#define gui_service_hpp

#include "pricing_service.hpp"
#include "utilities.hpp"
#include <unordered_map>

template<typename T>
class GUIConnector;
template<typename T>
class PricingToGUIListener;

template<typename T>
class GUIService : Service<string, Price<T>> {
private:
    unordered_map<string, Price<T>> guis_;
    GUIConnector<T>* out_connector_;
    ServiceListener<Price<T>>* in_listener_;
    int throttle_;
    long millisec_;

public:
    GUIService();
    ~GUIService();

    // MARK: SERVICE CLASS OVERRIDE BELOW
    // Get data on our service given a key (orderbook)
    virtual Price<T>& GetData(string product_id) override;
    
    // The callback that a Connector should invoke for any new or updated data
    virtual void OnMessage(Price<T>& data) override;
    
    // Add a listener to the Service for callbacks on add, remove, and update events
    // for data to the Service.
    virtual void AddListener(ServiceListener<Price<T>>* listener) override;
    
    // Get all listeners on the Service.
    virtual const vector<ServiceListener<Price<T>>*>& GetListeners() const override;
    // MARK: SERVICE CLASS OVERRIDE ABOVE
    
    PricingConnector<T>* GetConnector();

    // Get the listener of the service
    ServiceListener<Price<T>>* GetInListener();

    // Get the throttle of the service
    int GetThrottle() const;

    // Get the millisec of the service
    long GetMillisec() const;

    // Set the millisec of the service
    void SetMillisec(long _millisec);

};

template<typename T>
class GUIConnector : public Connector<Price<T>> {
private:
    GUIService<T>* service_;

public:
    GUIConnector(GUIService<T>* _service);
    ~GUIConnector() = default;

    // Publish data to the Connector
    void Publish(Price<T>& data);

    // Subscribe data from the Connector
    void Subscribe(ifstream& data);

};

template<typename T>
class PricingToGUIListener : public ServiceListener<Price<T>> {
private:
    GUIService<T>* service_;

public:
    // Connector and Destructor
    PricingToGUIListener(GUIService<T>* service);
    ~PricingToGUIListener() = default;

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
GUIService<T>::GUIService() : throttle_(300), millisec_(0) {
    out_connector_ = new GUIConnector<T>(this);
    in_listener_ = new PricingToGUIListener<T>(this);
}

template<typename T>
GUIService<T>::~GUIService() {
    delete out_connector_;
    delete in_listener_;
}

template <typename T>
Price<T>& GUIService<T>::GetData(string product_id) {
    return guis_[product_id];
}

template <typename T>
void GUIService<T>::OnMessage(Price<T>& data) {
    string product_id = data.GetProduct().GetProductId();
    
    guis_.insert_or_assign(product_id, data);
//    guis_[product_id] = data;
    out_connector_->Publish(data);
//    // Also notify listeners
//    for (auto& listener : Service<string, Price<T>>::listeners_) {
//        listener->ProcessAdd(data);
//    }
}

template <typename T>
void GUIService<T>::AddListener(ServiceListener<Price<T>>* listener) {
    this->Service<string, Price<T>>::AddListener(listener);
}

template <typename T>
const vector<ServiceListener<Price<T>>*>& GUIService<T>::GetListeners() const {
    return this->Service<string, Price<T>>::GetListeners();
}

template <typename T>
PricingConnector<T>* GUIService<T>::GetConnector() {
    return out_connector_;
}

template <typename T>
ServiceListener<Price<T>>* GUIService<T>::GetInListener() {
    return in_listener_;
}

template<typename T>
int GUIService<T>::GetThrottle() const {
    return throttle_;
}

template<typename T>
long GUIService<T>::GetMillisec() const {
    return millisec_;
}

template<typename T>
void GUIService<T>::SetMillisec(long millisec) {
    millisec_ = millisec;
}

template<typename T>
GUIConnector<T>::GUIConnector(GUIService<T>* service) : service_(service) {}

template<typename T>
void GUIConnector<T>::Publish(Price<T>& data)
{
    int throttle = service_->GetThrottle();
    long millisec = service_->GetMillisec();
    long millisec_now = GetMillisecond();
    while (millisec_now < millisec) millisec_now += 1000;
    if (millisec_now - millisec >= throttle)
    {
        service_->SetMillisec(millisec_now);
        ofstream file;
        file.open("gui.txt", ios::app);

        file << GetTimestamp() << ",";
        vector<string> strings = data.ToString();
        for (auto& s : strings)
        {
            file << s << ",";
        }
        file << endl;
    }
}

template<typename T>
void GUIConnector<T>::Subscribe(ifstream& data) {}

template<typename T>
PricingToGUIListener<T>::PricingToGUIListener(GUIService<T>* service) : service_(service) {}

template<typename T>
void PricingToGUIListener<T>::ProcessAdd(Price<T>& data) {
    service_->OnMessage(data);
}

template<typename T>
void PricingToGUIListener<T>::ProcessRemove(Price<T>& data) {}

template<typename T>
void PricingToGUIListener<T>::ProcessUpdate(Price<T>& data) {}

#endif /* gui_service_hpp */
