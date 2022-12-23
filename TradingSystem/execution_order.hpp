/**
 * execution order.hpp
 * Defines an execution order for AlgoExecutionService and ExecutionService
 *
 * @authors Breman Thuraisingham, Mingsen Wang
 */

#ifndef execution_order_hpp
#define execution_order_hpp

#include <vector>
#include <string>

enum OrderType { FOK, IOC, MARKET, LIMIT, STOP };

enum Market { BROKERTEC, ESPEED, CME };

/**
 * An execution order that can be placed on an exchange.
 * Type T is the product type.
 */
template<typename T>
class ExecutionOrder
{

public:

    ExecutionOrder() = default;
    // ctor for an order
    ExecutionOrder(const T &_product, PricingSide _side, string _orderId, OrderType _orderType, double _price, double _visibleQuantity, double _hiddenQuantity, string _parentOrderId, bool _isChildOrder);

    // Get the product
    const T& GetProduct() const;
    
    // Get pricing side
    PricingSide GetPricingSide() const;

    // Get the order ID
    const string& GetOrderId() const;

    // Get the order type on this order
    OrderType GetOrderType() const;

    // Get the price on this order
    double GetPrice() const;

    // Get the visible quantity on this order
    long GetVisibleQuantity() const;

    // Get the hidden quantity
    long GetHiddenQuantity() const;

    // Get the parent order ID
    const string& GetParentOrderId() const;

    // Is child order?
    bool IsChildOrder() const;
    
    vector<string> ToString() const;

private:
    T product;
    PricingSide side;
    string orderId;
    OrderType orderType;
    double price;
    double visibleQuantity;
    double hiddenQuantity;
    string parentOrderId;
    bool isChildOrder;

};

template<typename T>
ExecutionOrder<T>::ExecutionOrder(const T &_product, PricingSide _side, string _orderId, OrderType _orderType, double _price, double _visibleQuantity, double _hiddenQuantity, string _parentOrderId, bool _isChildOrder) :
  product(_product)
{
    side = _side;
    orderId = _orderId;
    orderType = _orderType;
    price = _price;
    visibleQuantity = _visibleQuantity;
    hiddenQuantity = _hiddenQuantity;
    parentOrderId = _parentOrderId;
    isChildOrder = _isChildOrder;
}

template<typename T>
const T& ExecutionOrder<T>::GetProduct() const
{
    return product;
}

template<typename T>
PricingSide ExecutionOrder<T>::GetPricingSide() const {
    return side;
}

template<typename T>
const string& ExecutionOrder<T>::GetOrderId() const
{
    return orderId;
}

template<typename T>
OrderType ExecutionOrder<T>::GetOrderType() const
{
    return orderType;
}

template<typename T>
double ExecutionOrder<T>::GetPrice() const
{
    return price;
}

template<typename T>
long ExecutionOrder<T>::GetVisibleQuantity() const
{
    return visibleQuantity;
}

template<typename T>
long ExecutionOrder<T>::GetHiddenQuantity() const
{
    return hiddenQuantity;
}

template<typename T>
const string& ExecutionOrder<T>::GetParentOrderId() const
{
    return parentOrderId;
}

template<typename T>
bool ExecutionOrder<T>::IsChildOrder() const
{
    return isChildOrder;
}

template<typename T>
vector<string> ExecutionOrder<T>::ToString() const
{
    string _product = product.GetProductId();
    string _side;
    switch (side)
    {
    case BID:
        _side = "BID";
        break;
    case OFFER:
        _side = "OFFER";
        break;
    }
    string _orderId = orderId;
    string _orderType;
    switch (orderType)
    {
    case FOK:
        _orderType = "FOK";
        break;
    case IOC:
        _orderType = "IOC";
        break;
    case MARKET:
        _orderType = "MARKET";
        break;
    case LIMIT:
        _orderType = "LIMIT";
        break;
    case STOP:
        _orderType = "STOP";
        break;
    }
    string _price = ConvertPrice(price);
    string _visibleQuantity = to_string(visibleQuantity);
    string _hiddenQuantity = to_string(hiddenQuantity);
    string _parentOrderId = parentOrderId;
    string _isChildOrder = isChildOrder ? "YES" : "NO";

    vector<string> _strings;
    _strings.push_back(_product);
    _strings.push_back(_side);
    _strings.push_back(_orderId);
    _strings.push_back(_orderType);
    _strings.push_back(_price);
    _strings.push_back(_visibleQuantity);
    _strings.push_back(_hiddenQuantity);
    _strings.push_back(_parentOrderId);
    _strings.push_back(_isChildOrder);
    return _strings;
    
}

#endif /* execution_order_hpp */
