/**
 * marketdataservice.hpp
 * Defines the data types and Service for order book market data.
 *
 * @authors Breman Thuraisingham, Mingsen Wang
 */

/*
 Design modification:
 (1) BidOffer now stores const reference to best bid/offer orders rather than copies of them. In a related decision, MarketDataService::GetBestBidOffer returns a copy of a BidOffer object instead of a reference to it. EXPLANATION: The original design makes MarketDataService::GetBestBidOffer return a reference of a BidOffer object, which is temporary, while the best bid and offer, as orders, exceeds(?) the lifetime of the aggregating BidOffer object.
 
 Addition:
 (1) Added a GetBestBidOffer method to OrderBook for easuer implementation of the synonymous method of MarketDataService.
 (2) Added class MarketDataServiceConnector
 */
#ifndef MARKET_DATA_SERVICE_HPP
#define MARKET_DATA_SERVICE_HPP

#include <string>
#include <vector>
#include <map>
#include "soa.hpp"

#include <unordered_map>

using namespace std;

// Side for market data
enum PricingSide { BID, OFFER };

/**
 * A market data order with price, quantity, and side.
 */
class Order
{

public:

    // ctor for an order
    Order(double _price, long _quantity, PricingSide _side);

    // Get the price on the order
    double GetPrice() const;

    // Get the quantity on the order
    long GetQuantity() const;

    // Get the side on the order
    PricingSide GetSide() const;

private:
    double price;
    long quantity;
    PricingSide side;

};

/**
 * Class representing a bid and offer order
 */
class BidOffer
{

public:

    // ctor for bid/offer
    BidOffer(const Order &_bidOrder, const Order &_offerOrder);

    // Get the bid order
    const Order& GetBidOrder() const;

    // Get the offer order
    const Order& GetOfferOrder() const;

private:
    const Order& bidOrder;
    const Order& offerOrder;
};

/**
 * Order book with a bid and offer stack.
 * Type T is the product type.
 */
template<typename T>
class OrderBook
{

public:

    // ctor for the order book
    OrderBook(const T &_product, const vector<Order> &_bidStack, const vector<Order> &_offerStack);

    // Get the product
    const T& GetProduct() const;

    // Get the bid stack
    const vector<Order>& GetBidStack() const;

    // Get the offer stack
    const vector<Order>& GetOfferStack() const;
    
    // Get the best bid/offer order
    const BidOffer GetBidOffer() const;

private:
    T product;
    vector<Order> bidStack;
    vector<Order> offerStack;

};

template <typename T>
class MarketDataConnector;

/**
 * Market Data Service which distributes market data
 * Keyed on product identifier.
 * Type T is the product type.
 */
template<typename T>
class MarketDataService : public Service<string,OrderBook <T> >
{
private:
    
    map<string, OrderBook<T>> order_books_;
    vector<ServiceListener<OrderBook<T>>*> listeners_;
    MarketDataConnector<T>* connector_;
    int book_depth_;
    
public:

    MarketDataService();
    ~MarketDataService();
    
    // Get order book with product id (modificiation allowed)
    OrderBook<T>& GetOrderBook(const string& product_id);
    
    // Get order book with product id (modificiation proscribed)
    const OrderBook<T>& GetOrderBook(const string& product_id) const;
    
    // Callback function for the MarketDataConnector
    void OnMessage(OrderBook<T>& book);
    
    // Add a listener to the list of listeners
    void AddListener(ServiceListener<OrderBook<T>>* listener);
    
    // Get the list of all listeners
    const vector<ServiceListener<OrderBook<T>>*>& GetListeners() const;
    
    // Get the MarketDataConnector
    MarketDataConnector<T>* GetConnector();
    
    // Get the book depth
    int GetBookDepth() const;
    
    // Get the best bid/offer order
    virtual const BidOffer GetBestBidOffer(const string &productId) const = 0;

    // Aggregate the order book
    virtual const OrderBook<T>& AggregateDepth(const string &productId) = 0;
    
    // AggregateDepth helper function
    vector<Order> AggregateStack(const vector<Order>& original_stack) const;

};

Order::Order(double _price, long _quantity, PricingSide _side)
{
    price = _price;
    quantity = _quantity;
    side = _side;
}

double Order::GetPrice() const
{
    return price;
}
 
long Order::GetQuantity() const
{
    return quantity;
}
 
PricingSide Order::GetSide() const
{
    return side;
}

BidOffer::BidOffer(const Order &_bidOrder, const Order &_offerOrder) :
  bidOrder(_bidOrder), offerOrder(_offerOrder)
{
}

const Order& BidOffer::GetBidOrder() const
{
    return bidOrder;
}

const Order& BidOffer::GetOfferOrder() const
{
    return offerOrder;
}

template<typename T>
OrderBook<T>::OrderBook(const T &_product, const vector<Order> &_bidStack, const vector<Order> &_offerStack) :
  product(_product), bidStack(_bidStack), offerStack(_offerStack)
{
}

template<typename T>
const T& OrderBook<T>::GetProduct() const
{
    return product;
}

template<typename T>
const vector<Order>& OrderBook<T>::GetBidStack() const
{
    return bidStack;
}

template<typename T>
const vector<Order>& OrderBook<T>::GetOfferStack() const
{
    return offerStack;
}

template<typename T>
const BidOffer OrderBook<T>::GetBidOffer() const {
    // Get highest bid order
    Order& highest_bid_order(bidStack[0]);
    double highest_bid_price = highest_bid_order.GetPrice();
    for (std::size_t i = 1; i < bidStack.size(); i++) {
        if (bidStack[i].GetPrice() > highest_bid_price) {
            highest_bid_order = bidStack[i];
            highest_bid_price = highest_bid_order.GetPrice();
        }
    }

    // Get lowest offer order
    Order& lowest_offer_order(offerStack[0]);
    double lowest_offer_price = lowest_offer_order.GetPrice();
    for (std::size_t i = 1; i < offerStack.size(); i++) {
        if (offerStack[i].GetPrice() > lowest_offer_price) {
            lowest_offer_order = offerStack[i];
            lowest_offer_price = lowest_offer_order.GetPrice();
        }
    }

    return BidOffer(highest_bid_order, lowest_offer_order);
}

template<typename T>
MarketDataService<T>::MarketDataService() : order_books_(), listeners_(), connector_(new MarketDataConnector<T>(this)), book_depth_(10) {}

template<typename T>
MarketDataService<T>::~MarketDataService() {
    delete connector_;
}

// Get order book with product id (modificiation allowed)
template<typename T>
OrderBook<T>& MarketDataService<T>::GetOrderBook(const string& product_id) {
    return *order_books_.find(product_id);
}

// Get order book with product id (modificiation proscribed)
template<typename T>
const OrderBook<T>& MarketDataService<T>::GetOrderBook(const string& product_id) const {
    return *order_books_.find(product_id);
}

// Callback function for the MarketDataConnector
template<typename T>
void MarketDataService<T>::OnMessage(OrderBook<T>& book) {
    string product_id = book.GetProduct().GetProductID();
    
    order_books_[product_id] = book;
    
    // Also notify listeners
    for (auto& listener : listeners_) {
        listener->ProcessAdd(book);
    }
}

// Add a listener to the list of listeners
template<typename T>
void MarketDataService<T>::AddListener(ServiceListener<OrderBook<T>>* listener) {
    listeners_.push_back(listener);
}

// Get the list of all listeners
template<typename T>
const vector<ServiceListener<OrderBook<T>>*>& MarketDataService<T>::GetListeners() const {
    return listeners_;
}

// Get the MarketDataConnector
template<typename T>
MarketDataConnector<T>* MarketDataService<T>::GetConnector() {
    return connector_;
}

// Get the book depth
template<typename T>
int MarketDataService<T>::GetBookDepth() const {
    return book_depth_;
}

// Get the best bid/offer order
template <typename T>
const BidOffer MarketDataService<T>::GetBestBidOffer(const string &productId) const {
    return order_books_.find(productId)->GetBestBidOffer();
}

// AggregateDepth helper function
template <typename T>
vector<Order> MarketDataService<T>::AggregateStack(const vector<Order>& original_stack) const {
    
    unordered_map<double, long> order_map;
    
    for (const auto& order : original_stack) {
        if (order_map.find(order.GetPrice()) == order_map.end()) {
            // The price does not exist yet
            order_map[order.GetPrice()] = order.GetQuantity();
        } else {
            // The price already exists
            order_map[order.GetPrice()] += order.GetQuantity();
        }
    }
    
    vector<Order> aggregated_stack;
    aggregated_stack.reserve(order_map.size());
    for (auto [price, quantity] : order_map) {
        Order order(price, quantity, BID);
        aggregated_stack.push_back(order);
    }
    
    return aggregated_stack;
}

// Aggregate the order book
template <typename T>
const OrderBook<T>& MarketDataService<T>::AggregateDepth(const string &productId) {
    const T& product = order_books_.find(productId)->GetProduct();
    
    // Aggregate bid orders
    const vector<Order>& original_bid_stack = order_books_.find(productId)->GetBidStack();
    
    vector<Order>& aggregated_bid_stack = this->AggregateStack(original_bid_stack);
    
    // Aggregate offer orders
    const vector<Order>& original_offer_stack = order_books_.find(productId)->GetOfferStack();
    
    vector<Order>& aggregated_offer_stack = this->AggregateStack(original_offer_stack);
    
    return OrderBook<T>(product, aggregated_bid_stack, aggregated_offer_stack);
}

#endif /* MARKET_DATA_SERVICE_HPP */
