/**
 * pricingservice.hpp
 * Defines the data types and Service for internal prices.
 *
 * @authors Breman Thuraisingham, Mingsen Wang
 */
#ifndef PRICING_SERVICE_HPP
#define PRICING_SERVICE_HPP

#include <string>
#include "soa.hpp"
#include <unordered_map>
#include <vector>

/**
 * A price object consisting of mid and bid/offer spread.
 * Type T is the product type.
 */
template <typename T>
class Price
{

public:

    Price() = default;
    // ctor for a price
    Price(const T &_product, double _mid, double _bidOfferSpread);
    Price(const Price<T>& price);
    
    Price<T>& operator = (const Price<T>& price);

    // Get the product
    const T& GetProduct() const;

    // Get the mid price
    double GetMid() const;

    // Get the bid/offer spread around the mid
    double GetBidOfferSpread() const;
    
    vector<string> ToString() const;

private:
    T product;
    double mid;
    double bidOfferSpread;

};

template <typename T>
class PricingConnector;

/**
 * Pricing Service managing mid prices and bid/offers.
 * Keyed on product identifier.
 * Type T is the product type.
 */
template <typename T>
class PricingService : public Service<string,Price <T> > {
private:
    unordered_map<string, Price<T>> prices_;
    PricingConnector<T>* in_connector_;
    
public:
    PricingService();
    ~PricingService();
    
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
};

template <typename T>
class PricingConnector : public Connector<Price<T>> {
private:
    PricingService<T>* service_;

public:
    PricingConnector(PricingService<T>* service);
    ~PricingConnector() = default;

    // Publish data to the Connector (does nothing here)
    virtual void Publish(Price<T>& data) override;

    // Subscribe data from the Connector
    virtual void Subscribe(ifstream& data) override;
};

template <typename T>
Price<T>::Price(const T &_product, double _mid, double _bidOfferSpread) :
  product(_product)
{
  mid = _mid;
  bidOfferSpread = _bidOfferSpread;
}

template <typename T>
Price<T>::Price(const Price<T>& price) :
product(price.product), mid(price.mid), bidOfferSpread(price.bidOfferSpread) {}

template <typename T>
Price<T>& Price<T>::operator = (const Price<T>& price) {
    if (&price != this) {
        product = price.product;
        mid = price.mid;
        bidOfferSpread = price.bidOfferSpread;
    }
    return *this;
}

template <typename T>
const T& Price<T>::GetProduct() const
{
  return product;
}

template <typename T>
double Price<T>::GetMid() const
{
  return mid;
}

template <typename T>
double Price<T>::GetBidOfferSpread() const
{
  return bidOfferSpread;
}

template <typename T>
PricingService<T>::PricingService() {
    in_connector_ = new PricingConnector<T>(this);
}

template <typename T>
PricingService<T>::~PricingService() {
    delete in_connector_;
}

template <typename T>
Price<T>& PricingService<T>::GetData(string product_id) {
    return prices_[product_id];
}

template <typename T>
void PricingService<T>::OnMessage(Price<T>& data) {
    string product_id = data.GetProduct().GetProductId();
    
    // Also notify listeners
    for (auto& listener : Service<string, Price<T>>::listeners_) {
        listener->ProcessAdd(data);
    }
}

template <typename T>
void PricingService<T>::AddListener(ServiceListener<Price<T>>* listener) {
    this->Service<string, Price<T>>::AddListener(listener);
}

template <typename T>
const vector<ServiceListener<Price<T>>*>& PricingService<T>::GetListeners() const {
    return this->Service<string, Price<T>>::GetListeners();
}

template <typename T>
PricingConnector<T>* PricingService<T>::GetConnector() {
    return in_connector_;
}

template<typename T>
PricingConnector<T>::PricingConnector(PricingService<T>* service) : service_(service) {}

template<typename T>
void PricingConnector<T>::Publish(Price<T>& data) {}

template<typename T>
void PricingConnector<T>::Subscribe(ifstream& data)
{
    string line;
    while (getline(data, line))
    {
        // Separate line with delimiter ','
        stringstream line_stream(line);
        string line_entry;
        vector<string> line_entries;
        while (getline(line_stream, line_entry, ',')) {
            line_entries.push_back(line_entry);
        }

        // Parse data into Price
        string product_id = line_entries[0];
        double bid_price = ConvertPrice(line_entries[1]);
        double offer_price = ConvertPrice(line_entries[2]);
        double mid_price = (bid_price + offer_price) / 2.;
        double spread = offer_price - bid_price;
        T product = FetchBond(product_id);
        Price<T> price(product, mid_price, spread);
        
        // Push price to connecting service
        service_->OnMessage(price);
    }
}

template<typename T>
vector<string> Price<T>::ToString() const
{
    string _product = product.GetProductId();
    string _mid = ConvertPrice(mid);
    string _bidOfferSpread = ConvertPrice(bidOfferSpread);

    vector<string> _strings;
    _strings.push_back(_product);
    _strings.push_back(_mid);
    _strings.push_back(_bidOfferSpread);
    return _strings;
}

#endif
