/**
 * price stream.hpp
 * Defines a price stream for AlgoStreamingService and StreamingService
 *
 * @authors Breman Thuraisingham, Mingsen Wang
 */

#ifndef price_stream_hpp
#define price_stream_hpp

#include <string>
#include "soa.hpp"
#include "pricing_service.hpp"
#include <vector>

/**
 * A price stream order with price and quantity (visible and hidden)
 */
class PriceStreamOrder
{

public:

    PriceStreamOrder() = default;
    // ctor for an order
    PriceStreamOrder(double _price, long _visibleQuantity, long _hiddenQuantity, PricingSide _side);

    // The side on this order
    PricingSide GetSide() const;

    // Get the price on this order
    double GetPrice() const;

    // Get the visible quantity on this order
    long GetVisibleQuantity() const;

    // Get the hidden quantity on this order
    long GetHiddenQuantity() const;
    
    vector<string> ToString() const;

private:
    double price;
    long visibleQuantity;
    long hiddenQuantity;
    PricingSide side;

};

/**
 * Price Stream with a two-way market.
 * Type T is the product type.
 */
template<typename T>
class PriceStream
{

public:

    PriceStream() = default;
    // ctor
    PriceStream(const T &_product, const PriceStreamOrder &_bidOrder, const PriceStreamOrder &_offerOrder);

    // Get the product
    const T& GetProduct() const;

    // Get the bid order
    const PriceStreamOrder& GetBidOrder() const;

    // Get the offer order
    const PriceStreamOrder& GetOfferOrder() const;
    
    vector<string> ToString() const;

private:
    T product;
    PriceStreamOrder bidOrder;
    PriceStreamOrder offerOrder;

};

PriceStreamOrder::PriceStreamOrder(double _price, long _visibleQuantity, long _hiddenQuantity, PricingSide _side)
{
    price = _price;
    visibleQuantity = _visibleQuantity;
    hiddenQuantity = _hiddenQuantity;
    side = _side;
}

PricingSide PriceStreamOrder::GetSide() const
{
    return side;
}

double PriceStreamOrder::GetPrice() const
{
    return price;
}

long PriceStreamOrder::GetVisibleQuantity() const
{
    return visibleQuantity;
}

long PriceStreamOrder::GetHiddenQuantity() const
{
    return hiddenQuantity;
}

template<typename T>
PriceStream<T>::PriceStream(const T &_product, const PriceStreamOrder &_bidOrder, const PriceStreamOrder &_offerOrder) :
  product(_product), bidOrder(_bidOrder), offerOrder(_offerOrder) {}

template<typename T>
const T& PriceStream<T>::GetProduct() const
{
    return product;
}

template<typename T>
const PriceStreamOrder& PriceStream<T>::GetBidOrder() const
{
    return bidOrder;
}

template<typename T>
const PriceStreamOrder& PriceStream<T>::GetOfferOrder() const
{
    return offerOrder;
}

vector<string> PriceStreamOrder::ToString() const
{
    string _price = ConvertPrice(price);
    string _visibleQuantity = to_string(visibleQuantity);
    string _hiddenQuantity = to_string(hiddenQuantity);
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

    vector<string> _strings;
    _strings.push_back(_price);
    _strings.push_back(_visibleQuantity);
    _strings.push_back(_hiddenQuantity);
    _strings.push_back(_side);
    return _strings;
}

template<typename T>
vector<string> PriceStream<T>::ToString() const
{
    string _product = product.GetProductId();
    vector<string> _bidOrder = bidOrder.ToString();
    vector<string> _offerOrder = offerOrder.ToString();

    vector<string> _strings;
    _strings.push_back(_product);
    _strings.insert(_strings.end(), _bidOrder.begin(), _bidOrder.end());
    _strings.insert(_strings.end(), _offerOrder.begin(), _offerOrder.end());
    return _strings;
}

#endif /* price_stream_hpp */
