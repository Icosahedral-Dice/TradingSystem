/**
 * riskservice.hpp
 * Defines the data types and Service for fixed income risk.
 *
 * @authors Breman Thuraisingham, Mingsen Wang
 */
#ifndef RISK_SERVICE_HPP
#define RISK_SERVICE_HPP

#include "soa.hpp"
#include "position_service.hpp"
#include <vector>
#include <unordered_map>
#include "utilities.hpp"

/**
 * PV01 risk.
 * Type T is the product type.
 */
template <typename T>
class PV01
{

public:
    
    PV01() = default;
    // ctor for a PV01 value
    PV01(const T &_product, double _pv01, long _quantity);

    // Get the product on this PV01 value
    const T& GetProduct() const;

    // Get the PV01 value
    double GetPV01() const;

    // Get the quantity that this risk value is associated with
    long GetQuantity() const;

private:
    T product;
    double pv01;
    long quantity;

};

/**
 * A bucket sector to bucket a group of securities.
 * We can then aggregate bucketed risk to this bucket.
 * Type T is the product type.
 */
template <typename T>
class BucketedSector
{

public:

    BucketedSector() = default;
    // ctor for a bucket sector
    BucketedSector(const vector<T> &_products, string _name);

    // Get the products associated with this bucket
    const vector<T>& GetProducts() const;

    // Get the name of the bucket
    const string& GetName() const;

private:
    vector<T> products;
    string name;

};

template <typename T>
class PositionToRiskListener;

/**
 * Risk Service to vend out risk for a particular security and across a risk bucketed sector.
 * Keyed on product identifier.
 * Type T is the product type.
 */
template <typename T>
class RiskService : public Service<string,PV01 <T> >
{
private:
    unordered_map<string, PV01<T>> pv01s_;
    PositionToRiskListener<T>* in_listener_;
    
public:
    RiskService();
    ~RiskService();
    
    // MARK: SERVICE CLASS OVERRIDE BELOW
    // Get data on our service given a key (orderbook)
    virtual PV01<T>& GetData(string product_id) override;
    
    // The callback that a Connector should invoke for any new or updated data
    virtual void OnMessage(PV01<T>& data) override;
    
    // Add a listener to the Service for callbacks on add, remove, and update events
    // for data to the Service.
    virtual void AddListener(ServiceListener<PV01<T>>* listener) override;
    
    // Get all listeners on the Service.
    virtual const vector<ServiceListener<PV01<T>>*>& GetListeners() const override;
    // MARK: SERVICE CLASS OVERRIDE ABOVE
    
    PositionToRiskListener<T>* GetInListener();

    // Add a position that the service will risk
    void AddPosition(Position<T> &position);

    // Get the bucketed risk for the bucket sector
    const PV01< BucketedSector<T> >& GetBucketedRisk(const BucketedSector<T> &sector) const;

};

template <typename T>
class PositionToRiskListener : public ServiceListener<Position<T>> {
private:
    RiskService<T>* service_;

public:

    PositionToRiskListener(RiskService<T>* _service);
    ~PositionToRiskListener() = default;

    // MARK: SERVICELISTENER CLASS OVERRIDE BELOW
    // Listener callback to process an add event to the Service
    virtual void ProcessAdd(Position<T> &data) override;

    // Listener callback to process a remove event to the Service
    virtual void ProcessRemove(Position<T> &data) override;

    // Listener callback to process an update event to the Service
    virtual void ProcessUpdate(Position<T> &data) override;
    // MARK: SERVICELISTENER CLASS OVERRIDE ABOVE

};

template <typename T>
PV01<T>::PV01(const T &_product, double _pv01, long _quantity) :
  product(_product), pv01(_pv01), quantity(_quantity) {}

template <typename T>
BucketedSector<T>::BucketedSector(const vector<T>& _products, string _name) :
  products(_products), name(_name) {}

template <typename T>
const vector<T>& BucketedSector<T>::GetProducts() const
{
    return products;
}

template <typename T>
const string& BucketedSector<T>::GetName() const
{
    return name;
}

template <typename T>
RiskService<T>::RiskService() {
    in_listener_ = new PositionToRiskListener<T>(this);
}

template <typename T>
RiskService<T>::~RiskService() {
    delete in_listener_;
}

template <typename T>
PV01<T>& RiskService<T>::GetData(string product_id) {
    return pv01s_[product_id];
}

template <typename T>
void RiskService<T>::OnMessage(PV01<T>& data) {
    string product_id = data.GetProduct().GetProductId();
    
    pv01s_[product_id] = data;
    
//    // Also notify listeners
//    for (auto& listener : Service<string, Trade<T>>::listeners_) {
//        listener->ProcessAdd(data);
//    }
}

template <typename T>
void RiskService<T>::AddListener(ServiceListener<PV01<T>>* listener) {
    this->Service<string, PV01<T>>::AddListener(listener);
}

template <typename T>
const vector<ServiceListener<PV01<T>>*>& RiskService<T>::GetListeners() const {
    return this->Service<string, PV01<T>>::GetListeners();
}

template <typename T>
PositionToRiskListener<T>* RiskService<T>::GetInListener() {
    return in_listener_;
}

// Add a position that the service will risk
template <typename T>
void RiskService<T>::AddPosition(Position<T>& position) {
    
    // Parse info from position
    T product = position.GetProduct();
    string product_id = product.GetProductId();
    long quantity = position.GetAggregatePosition();
    
    // Convert to PV01 obj
    double pv01_value = GetPV01Value(product_id);
    PV01<T> pv01(product, pv01_value, quantity);
    pv01s_[product_id] = pv01;

    // Notify listeners
    for (auto& listener : Service<string, PV01<T>>::listeners_)
    {
        listener->ProcessAdd(pv01);
    }
}

// Get the bucketed risk for the bucket sector
template <typename T>
const PV01<BucketedSector<T>>& RiskService<T>::GetBucketedRisk(const BucketedSector<T>& sector) const {
    
    BucketedSector<T> product = sector;
    
    double pv01 = 0.;
    long quantity = 1;  // Dummy

    vector<T>& products = sector.GetProducts();
    for (auto& product : products)
    {
        string product_id = product.GetProductId();
        pv01 += pv01s_[product_id].GetPV01() * pv01s_[product_id].GetQuantity();
    }

    return PV01<BucketedSector<T>>(product, pv01, quantity);
}

template<typename T>
PositionToRiskListener<T>::PositionToRiskListener(RiskService<T>* service) : service_(service) {}

template<typename T>
void PositionToRiskListener<T>::ProcessAdd(Position<T>& data)
{
    service_->AddPosition(data);
}

template<typename T>
void PositionToRiskListener<T>::ProcessRemove(Position<T>& data) {}

template<typename T>
void PositionToRiskListener<T>::ProcessUpdate(Position<T>& data) {}

#endif
