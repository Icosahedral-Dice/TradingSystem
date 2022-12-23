/**
 * inquiryservice.hpp
 * Defines the data types and Service for customer inquiries.
 *
 * @authors Breman Thuraisingham, Mingsen Wang
 */
#ifndef INQUIRY_SERVICE_HPP
#define INQUIRY_SERVICE_HPP

#include "soa.hpp"
#include "trade_booking_service.hpp"
#include <unordered_map>

// Various inqyury states
enum InquiryState { RECEIVED, QUOTED, DONE, REJECTED, CUSTOMER_REJECTED };

/**
 * Inquiry object modeling a customer inquiry from a client.
 * Type T is the product type.
 */
template<typename T>
class Inquiry
{

public:

    Inquiry() = default;
    // ctor for an inquiry
    Inquiry(string _inquiryId, const T &_product, Side _side, long _quantity, double _price, InquiryState _state);

    // Get the inquiry ID
    const string& GetInquiryId() const;

    // Get the product
    const T& GetProduct() const;

    // Get the side on the inquiry
    Side GetSide() const;

    // Get the quantity that the client is inquiring for
    long GetQuantity() const;

    // Get the price that we have responded back with
    double GetPrice() const;

    // Get the current state on the inquiry
    InquiryState GetState() const;
    
    void SetState(InquiryState new_state);
    
    vector<string> ToString() const;

private:
    string inquiryId;
    T product;
    Side side;
    long quantity;
    double price;
    InquiryState state;

};

template<typename T>
class InquiryConnector;



/**
 * Service for customer inquirry objects.
 * Keyed on inquiry identifier (NOTE: this is NOT a product identifier since each inquiry must be unique).
 * Type T is the product type.
 */
template<typename T>
class InquiryService : public Service<string,Inquiry <T> >
{
private:
    unordered_map<string, Inquiry<T>> inquiries_;
    InquiryConnector<T>* connector_;    // Both in and out
    
public:
    
    InquiryService();
    ~InquiryService();
    
    // MARK: SERVICE CLASS OVERRIDE BELOW
    // Get data on our service given a key (orderbook)
    virtual Inquiry<T>& GetData(string product_id) override;
    
    // The callback that a Connector should invoke for any new or updated data
    virtual void OnMessage(Inquiry<T>& data) override;
    
    // Add a listener to the Service for callbacks on add, remove, and update events
    // for data to the Service.
    virtual void AddListener(ServiceListener<Inquiry<T>>* listener) override;
    
    // Get all listeners on the Service.
    virtual const vector<ServiceListener<Inquiry<T>>*>& GetListeners() const override;
    // MARK: SERVICE CLASS OVERRIDE ABOVE
    
    // Get the connector of the service
    InquiryConnector<T>* GetConnector();

    // Send a quote back to the client
    void SendQuote(const string &inquiryId, double price);

    // Reject an inquiry from the client
    void RejectInquiry(const string &inquiryId);

};

template<typename T>
class InquiryConnector : public Connector<Inquiry<T>> {
private:
    InquiryService<T>* service_;

public:
    InquiryConnector(InquiryService<T>* service);
    ~InquiryConnector() = default;

    // Publish data to the Connector
    void Publish(Inquiry<T>& data);

    // Subscribe data from the Connector
    void Subscribe(ifstream& data);
    
    // Re-subscribe data from the Connector
    void Subscribe(Inquiry<T>& data);

};

template<typename T>
Inquiry<T>::Inquiry(string _inquiryId, const T &_product, Side _side, long _quantity, double _price, InquiryState _state) :
  product(_product)
{
    inquiryId = _inquiryId;
    side = _side;
    quantity = _quantity;
    price = _price;
    state = _state;
}

template<typename T>
const string& Inquiry<T>::GetInquiryId() const {
    return inquiryId;
}

template<typename T>
const T& Inquiry<T>::GetProduct() const
{
    return product;
}

template<typename T>
Side Inquiry<T>::GetSide() const
{
    return side;
}

template<typename T>
long Inquiry<T>::GetQuantity() const
{
    return quantity;
}

template<typename T>
double Inquiry<T>::GetPrice() const
{
    return price;
}

template<typename T>
InquiryState Inquiry<T>::GetState() const
{
    return state;
}

template<typename T>
void Inquiry<T>::SetState(InquiryState new_state)
{
    state = new_state;
}

template<typename T>
InquiryService<T>::InquiryService() {
    connector_ = new InquiryConnector<T>(this);
}

template<typename T>
InquiryService<T>::~InquiryService() {
    delete connector_;
}

template<typename T>
Inquiry<T>& InquiryService<T>::GetData(string key)
{
    return inquiries_[key];
}

template<typename T>
void InquiryService<T>::OnMessage(Inquiry<T>& data)
{
    InquiryState state = data.GetState();
    string inquiry_id = data.GetInquiryId();
    switch (state) {
        case RECEIVED:
            inquiries_.insert_or_assign(inquiry_id, data);
            connector_->Publish(data);
            break;
        case QUOTED:
            data.SetState(DONE);
            inquiries_.insert_or_assign(inquiry_id, data);

            for (auto& listener : this->GetListeners())
            {
                listener->ProcessAdd(data);
            }
            break;
        default:
            break;
    }
}

template<typename T>
void InquiryService<T>::AddListener(ServiceListener<Inquiry<T>>* listener)
{
    this->Service<string, Inquiry<T>>::AddListener(listener);
}

template<typename T>
const vector<ServiceListener<Inquiry<T>>*>& InquiryService<T>::GetListeners() const
{
    return this->Service<string, Inquiry<T>>::GetListeners();
}

template<typename T>
InquiryConnector<T>* InquiryService<T>::GetConnector()
{
    return connector_;
}

template<typename T>
void InquiryService<T>::SendQuote(const string& inquiryId, double price)
{
    Inquiry<T>& inquiry = inquiries_[inquiryId];
//    InquiryState state = inquiry.GetState();
    inquiry.SetPrice(price);
    for (auto& listener : this->GetListeners())
    {
        listener->ProcessAdd(inquiry);
    }
}

template<typename T>
void InquiryService<T>::RejectInquiry(const string& inquiryId) {
    inquiries_[inquiryId].SetState(REJECTED);
}

template<typename T>
InquiryConnector<T>::InquiryConnector(InquiryService<T>* service) {
    service_ = service;
}

template<typename T>
void InquiryConnector<T>::Publish(Inquiry<T>& data)
{
    InquiryState state = data.GetState();
    if (state == RECEIVED)
    {
        data.SetState(QUOTED);
        this->Subscribe(data);
    }
}

template<typename T>
void InquiryConnector<T>::Subscribe(ifstream& data)
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

        // Parse data into Inquiry
        string inquiry_id = line_entries[0];
        string product_id = line_entries[1];
        Side side = (line_entries[2] == "BUY") ? BUY : SELL;
        long quantity = stol(line_entries[3]);
        double price = ConvertPrice(line_entries[4]);
        InquiryState state;
        if (line_entries[5] == "RECEIVED") state = RECEIVED;
        else if (line_entries[5] == "QUOTED") state = QUOTED;
        else if (line_entries[5] == "DONE") state = DONE;
        else if (line_entries[5] == "REJECTED") state = REJECTED;
        else if (line_entries[5] == "CUSTOMER_REJECTED") state = CUSTOMER_REJECTED;
        T product = FetchBond(product_id);
        Inquiry<T> inquiry(inquiry_id, product, side, quantity, price, state);
        service_->OnMessage(inquiry);
    }
}

template<typename T>
void InquiryConnector<T>::Subscribe(Inquiry<T>& data)
{
    service_->OnMessage(data);
}

template<typename T>
vector<string> Inquiry<T>::ToString() const
{
    string _inquiryId = inquiryId;
    string _product = product.GetProductId();
    string _side;
    switch (side)
    {
    case BUY:
        _side = "BUY";
        break;
    case SELL:
        _side = "SELL";
        break;
    }
    string _quantity = to_string(quantity);
    string _price = ConvertPrice(price);
    string _state;
    switch (state)
    {
    case RECEIVED:
        _state = "RECEIVED";
        break;
    case QUOTED:
        _state = "QUOTED";
        break;
    case DONE:
        _state = "DONE";
        break;
    case REJECTED:
        _state = "REJECTED";
        break;
    case CUSTOMER_REJECTED:
        _state = "CUSTOMER_REJECTED";
        break;
    }

    vector<string> _strings;
    _strings.push_back(_inquiryId);
    _strings.push_back(_product);
    _strings.push_back(_side);
    _strings.push_back(_quantity);
    _strings.push_back(_price);
    _strings.push_back(_state);
    return _strings;
}

#endif
