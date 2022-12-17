/**
 * tradebookingservice.hpp
 * Defines the data types and Service for trade booking.
 *
 * @author Breman Thuraisingham
 */
#ifndef TRADE_BOOKING_SERVICE_HPP
#define TRADE_BOOKING_SERVICE_HPP

#include <string>
#include <vector>
#include "soa.hpp"
#include "execution_service.hpp"

// Trade sides
enum Side { BUY, SELL };

/**
 * Trade object with a price, side, and quantity on a particular book.
 * Type T is the product type.
 */
template<typename T>
class Trade
{

public:

    // ctor for a trade
    Trade(const T &_product, string _tradeId, double _price, string _book, long _quantity, Side _side);

    // Get the product
    const T& GetProduct() const;

    // Get the trade ID
    const string& GetTradeId() const;

    // Get the mid price
    double GetPrice() const;

    // Get the book
    const string& GetBook() const;

    // Get the quantity
    long GetQuantity() const;

    // Get the side
    Side GetSide() const;

private:
    T product;
    string tradeId;
    double price;
    string book;
    long quantity;
    Side side;

};

template <typename T>
class TradeBookingConnector;
template <typename T>
class ExecutionToTradeBookingListener;

/**
 * Trade Booking Service to book trades to a particular book.
 * Keyed on trade id.
 * Type T is the product type.
 */
template<typename T>
class TradeBookingService : public Service<string,Trade <T> >
{
private:
    unordered_map<string, Trade<T>> trades_;
    TradeBookingConnector<T>* out_connector_;
    ExecutionToTradeBookingListener<T>* in_listener_;
    
public:
    // Constructor and destructor
    TradeBookingService();
    ~TradeBookingService();
    
    // MARK: SERVICE CLASS OVERRIDE BELOW
    // Get data on our service given a key (orderbook)
    virtual Trade<T>& GetData(string product_id) override;
    
    // The callback that a Connector should invoke for any new or updated data
    virtual void OnMessage(Trade<T>& data) override;
    
    // Add a listener to the Service for callbacks on add, remove, and update events
    // for data to the Service.
    virtual void AddListener(ServiceListener<Trade<T>>* listener) override;
    
    // Get all listeners on the Service.
    virtual const vector<ServiceListener<Trade<T>>*>& GetListeners() const override;
    // MARK: SERVICE CLASS OVERRIDE ABOVE
    
    ExecutionToTradeBookingListener<T>* GetInListener();
    
    // Book the trade
    void BookTrade(const Trade<T> &trade);
};

template<typename T>
class TradeBookingConnector : public Connector<Trade<T>>
{

private:

    TradeBookingService<T>* service;

public:

    // Connector and Destructor
    TradeBookingConnector(TradeBookingService<T>* _service);
    ~TradeBookingConnector() = default;

    // Publish data to the Connector
    virtual void Publish(Trade<T>& data) override;

    // Subscribe data from the Connector
    virtual void Subscribe(ifstream& data) override;

};

template <typename T>
class ExecutionToTradeBookingListener : public ServiceListener<ExecutionOrder<T>> {
private:
    TradeBookingService<T>* service_;
    long count_;
    
public:
    // Connector and Destructor
    ExecutionToTradeBookingListener(TradeBookingService<T>* _service);
    ~ExecutionToTradeBookingListener() = default;

    // MARK: SERVICELISTENER CLASS OVERRIDE BELOW
    // Listener callback to process an add event to the Service
    virtual void ProcessAdd(ExecutionOrder<T> &data) override;

    // Listener callback to process a remove event to the Service
    virtual void ProcessRemove(ExecutionOrder<T> &data) override;

    // Listener callback to process an update event to the Service
    virtual void ProcessUpdate(ExecutionOrder<T> &data) override;
    // MARK: SERVICELISTENER CLASS OVERRIDE ABOVE
};

template<typename T>
Trade<T>::Trade(const T &_product, string _tradeId, double _price, string _book, long _quantity, Side _side) :
  product(_product)
{
    tradeId = _tradeId;
    price = _price;
    book = _book;
    quantity = _quantity;
    side = _side;
}

template<typename T>
const T& Trade<T>::GetProduct() const
{
    return product;
}

template<typename T>
const string& Trade<T>::GetTradeId() const
{
    return tradeId;
}

template<typename T>
double Trade<T>::GetPrice() const
{
    return price;
}

template<typename T>
const string& Trade<T>::GetBook() const
{
    return book;
}

template<typename T>
long Trade<T>::GetQuantity() const
{
    return quantity;
}

template<typename T>
Side Trade<T>::GetSide() const
{
    return side;
}

template <typename T>
TradeBookingService<T>::TradeBookingService() {
    out_connector_ = new TradeBookingConnector<T>(this);
    in_listener_ = new ExecutionToTradeBookingListener<T>(this);
}

template <typename T>
TradeBookingService<T>::~TradeBookingService() {
    delete in_listener_;
    delete out_connector_;
}

template <typename T>
Trade<T>& TradeBookingService<T>::GetData(string product_id) {
    return trades_.at(product_id);
}

template <typename T>
void TradeBookingService<T>::OnMessage(Trade<T>& data) {
    string trade_id = data.GetTradeId();
    
    trades_[trade_id] = data;
    
    // Also notify listeners
    for (auto& listener : Service<string, Trade<T>>::listeners_) {
        listener->ProcessAdd(data);
    }
}

template <typename T>
void TradeBookingService<T>::AddListener(ServiceListener<Trade<T>>* listener) {
    this->Service<string, Trade<T>>::AddListener(listener);
}

template <typename T>
const vector<ServiceListener<Trade<T>>*>& TradeBookingService<T>::GetListeners() const {
    return this->Service<string, Trade<T>>::GetListeners();
}

template <typename T>
ExecutionToTradeBookingListener<T>* TradeBookingService<T>::GetInListener() {
    return in_listener_;
}

template <typename T>
void TradeBookingService<T>::BookTrade(const Trade<T> &trade) {
    for (auto& listener : Service<string, Trade<T>>::listeners_) {
        listener->ProcessAdd(trade);
    }
}

template <typename T>
TradeBookingConnector<T>::TradeBookingConnector(TradeBookingService<T>* _service) {
    service = _service;
}

template <typename T>
void TradeBookingConnector<T>::Publish(Trade<T>& data) {
    // Does nothing. The connector is subscribe only.
}

template <typename T>
void TradeBookingConnector<T>::Subscribe(ifstream& data) {
    string line;
    while (getline(data, line)) {
        // Separate line with delimiter ','
        stringstream line_stream(line);
        string line_entry;
        vector<string> line_entries;
        while (getline(line_stream, line_entry, ',')) {
            line_entries.push_back(line_entry);
        }
        
        // Parse data into Trade
        string product_id = line_entries[0];
        string trade_id = line_entries[1];
        double price = ConvertPrice(line_entries[2]);
        string book = line_entries[3];
        long quantity = stol(line_entries[4]);
        Side side = (line_entries[5] == "BUY") ? BUY : SELL;
        
        T product = FetchBond(product_id);
        Trade<T> trade(product, trade_id, price, book, quantity, side);
        
        // Notify connected service
        service->OnMessage(trade);
    }
}

template <typename T>
ExecutionToTradeBookingListener<T>::ExecutionToTradeBookingListener(TradeBookingService<T>* service) : service_(service), count_(0) {}

template <typename T>
void ExecutionToTradeBookingListener<T>::ProcessAdd(ExecutionOrder<T>& data) {
    
    count_++;
    
    // Get data from execution order
    T product = data.GetProduct();
    PricingSide pricing_side = data.GetPricingSide();
    string order_id = data.GetOrderId();
    double price = data.GetPrice();
    long visible_quantity = data.GetVisibleQuantity();
    long hidden_quantity = data.GetHiddenQuantity();

    // Generate trade
    // Sell to bids and buy to offers
    Side side = (pricing_side == BID) ? SELL : BUY;

    string book;
    switch (count_ % 3)
    {
    case 0:
        book = "TRSY1";
        break;
    case 1:
        book = "TRSY2";
        break;
    case 2:
        book = "TRSY3";
        break;
    }
    long quantity = visible_quantity + hidden_quantity;

    Trade<T> trade(product, order_id, price, book, quantity, side);
    
    // Request connected service to book the trade
    service_->OnMessage(trade);
    service_->BookTrade(trade);
}

template <typename T>
void ExecutionToTradeBookingListener<T>::ProcessRemove(ExecutionOrder<T>& data) {}

template <typename T>
void ExecutionToTradeBookingListener<T>::ProcessUpdate(ExecutionOrder<T>& data) {}

#endif
