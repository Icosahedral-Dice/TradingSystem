/**
 * positionservice.hpp
 * Defines the data types and Service for positions.
 *
 * @authors Breman Thuraisingham, Mingsen Wang
 */
#ifndef POSITION_SERVICE_HPP
#define POSITION_SERVICE_HPP

#include <string>
#include <map>
#include <unordered_map>
#include "soa.hpp"
#include "trade_booking_service.hpp"
#include <vector>

using namespace std;

/**
 * Position class in a particular book.
 * Type T is the product type.
 */
template<typename T>
class Position
{

public:

    Position() = default;
    // ctor for a position
    Position(const T &_product);

    // Get the product
    const T& GetProduct() const;

    // Get the position quantity
    long GetPosition(string &book);

    // Get the aggregate position
    long GetAggregatePosition();
    
    // Add position to designated book
    void AddPosition(string& book, long position, Side side);

    vector<string> ToString() const;
    
private:
    T product;
    map<string, long> positions;

};

template<typename T>
class TradeBookingToPositionListener;



/**
 * Position Service to manage positions across multiple books and secruties.
 * Keyed on product identifier.
 * Type T is the product type.
 */
template<typename T>
class PositionService : public Service<string,Position <T> >
{
private:
    unordered_map<string, Position<T>> positions_;
    TradeBookingToPositionListener<T>* in_listener_;

public:
    PositionService();
    ~PositionService();
    
    // MARK: SERVICE CLASS OVERRIDE BELOW
    // Get data on our service given a key (orderbook)
    virtual Position<T>& GetData(string product_id) override;
    
    // The callback that a Connector should invoke for any new or updated data
    virtual void OnMessage(Position<T>& data) override;
    
    // Add a listener to the Service for callbacks on add, remove, and update events
    // for data to the Service.
    virtual void AddListener(ServiceListener<Position<T>>* listener) override;
    
    // Get all listeners on the Service.
    virtual const vector<ServiceListener<Position<T>>*>& GetListeners() const override;
    // MARK: SERVICE CLASS OVERRIDE ABOVE
    
    TradeBookingToPositionListener<T>* GetInListener();

    // Add a trade to the service
    virtual void AddTrade(const Trade<T> &trade);
};

template<typename T>
class TradeBookingToPositionListener : public ServiceListener<Trade<T>> {
private:
    PositionService<T>* service_;
    
public:
    TradeBookingToPositionListener(PositionService<T>* service);
    ~TradeBookingToPositionListener() = default;
    
    // MARK: SERVICELISTENER CLASS OVERRIDE BELOW
    // Listener callback to process an add event to the Service
    virtual void ProcessAdd(Trade<T> &data) override;

    // Listener callback to process a remove event to the Service
    virtual void ProcessRemove(Trade<T> &data) override;

    // Listener callback to process an update event to the Service
    virtual void ProcessUpdate(Trade<T> &data) override;
    // MARK: SERVICELISTENER CLASS OVERRIDE ABOVE
};

template<typename T>
Position<T>::Position(const T &_product) :
  product(_product) {}

template<typename T>
const T& Position<T>::GetProduct() const
{
    return product;
}

template<typename T>
long Position<T>::GetPosition(string &book)
{
    return positions[book];
}

template<typename T>
void Position<T>::AddPosition(string& book, long position, Side side) {
    // Insert 0 if the book does not exist
    positions.try_emplace(book, 0);
    
    switch (side) {
        case BUY:
            positions[book] += position;
            break;
        case SELL:
            positions[book] -= position;
            break;
    }
}

template<typename T>
long Position<T>::GetAggregatePosition() {
    long res = 0;
    for (const auto& [book, pos] : positions) {
        res += pos;
    }
    
    return res;
}

template <typename T>
PositionService<T>::PositionService() {
    in_listener_ = new TradeBookingToPositionListener<T>(this);
}

template <typename T>
PositionService<T>::~PositionService() {
    delete in_listener_;
}

template <typename T>
Position<T>& PositionService<T>::GetData(string product_id) {
    return positions_[product_id];
}

template <typename T>
void PositionService<T>::OnMessage(Position<T>& data) {
    string product_id = data.GetProduct().GetProductId();
    
    positions_[product_id] = data;
    
//    // Also notify listeners
//    for (auto& listener : Service<string, Trade<T>>::listeners_) {
//        listener->ProcessAdd(data);
//    }
}

template <typename T>
void PositionService<T>::AddListener(ServiceListener<Position<T>>* listener) {
    this->Service<string, Position<T>>::AddListener(listener);
}

template <typename T>
const vector<ServiceListener<Position<T>>*>& PositionService<T>::GetListeners() const {
    return this->Service<string, Position<T>>::GetListeners();
}

template <typename T>
TradeBookingToPositionListener<T>* PositionService<T>::GetInListener() {
    return in_listener_;
}

template <typename T>
void PositionService<T>::AddTrade(const Trade<T> &trade) {
    
    // Get data from the trade
    T product = trade.GetProduct();
    string product_id = product.GetProductId();
    double price = trade.GetPrice();
    string book = trade.GetBook();
    long quantity = trade.GetQuantity();
    Side side = trade.GetSide();
    
    positions_.try_emplace(product_id, product);
    positions_[product_id].AddPosition(book, quantity, side);
    
    // Notify listeners
    for (auto& listener : Service<string, Position<T>>::listeners_) {
        listener->ProcessAdd(positions_[product_id]);
    }
}

template <typename T>
TradeBookingToPositionListener<T>::TradeBookingToPositionListener(PositionService<T>* service) : service_(service) {}

template<typename T>
void TradeBookingToPositionListener<T>::ProcessAdd(Trade<T>& data)
{
    service_->AddTrade(data);
}

template<typename T>
void TradeBookingToPositionListener<T>::ProcessRemove(Trade<T>& data) {}

template<typename T>
void TradeBookingToPositionListener<T>::ProcessUpdate(Trade<T>& data) {}

template<typename T>
vector<string> Position<T>::ToString() const
{
    string _product = product.GetProductId();
    vector<string> _positions;
    for (auto& p : positions)
    {
        string _book = p.first;
        string _position = to_string(p.second);
        _positions.push_back(_book);
        _positions.push_back(_position);
    }

    vector<string> _strings;
    _strings.push_back(_product);
    _strings.insert(_strings.end(), _positions.begin(), _positions.end());
    return _strings;
}



#endif
