//
//  main.cpp
//  TradingSystem
//
//  Created by 王明森 on 12/5/22.
//

#include <iostream>
#include <iomanip>
//#include "initialization.hpp"
#include "soa.hpp"
#include "products.hpp"
#include "market_data_service.hpp"
#include "bond_services.hpp"
#include "utilities.hpp"

#include "algo_streaming_service.hpp"
#include "streaming_service.hpp"
#include "historical_data_service.hpp"
#include "inquiry_service.hpp"

using namespace std;

void TestUtilities() {
    cout << FetchCusip(2) << endl;
    cout << FetchCusip(3) << endl;
    
    Bond bond = FetchBond(3);
    cout << bond.GetProductId() << endl;
    cout << bond.GetMaturityDate() << endl;
    
    cout << GetTimestamp() << endl;
}

void TestMarketDataService() {
    
    vector<Order> bid_stack;
    vector<Order> offer_stack;
    
    for (unsigned i = 1; i < 10; i++) {
        Order bid(100. - i / 256., i * i, BID);
        Order offer(100. + i / 256., i * i, OFFER);
        
        bid_stack.push_back(bid);
        offer_stack.push_back(offer);
    }
    
    for (unsigned i = 9; i > 0; i--) {
        Order bid(100. - i / 256., i * i, BID);
        Order offer(100. + i / 256., i * i, OFFER);
        
        bid_stack.push_back(bid);
        offer_stack.push_back(offer);
    }
    

    cout << kBondMapMaturity[2].first << endl;
    Bond two_year_note = FetchBond(2);
    BondOrderBook bond_order_book(two_year_note, bid_stack, offer_stack);
    BidOffer best_bid_offer = bond_order_book.GetBidOffer();
    cout << ConvertPrice(best_bid_offer.GetBidOrder().GetPrice()) << " - " << ConvertPrice(best_bid_offer.GetOfferOrder().GetPrice()) << endl;
    
    
    BondMarketDataService bond_market_data_service;
    
    bond_market_data_service.OnMessage(bond_order_book);
    
    cout << bond_market_data_service.GetData(FetchCusip(2)).GetBidOffer().GetBidOrder().GetQuantity() << endl;
    
    bond_market_data_service.AggregateDepth(FetchCusip(2));
    
    cout << bond_market_data_service.GetData(FetchCusip(2)).GetBidOffer().GetBidOrder().GetQuantity() << endl;
    
    bond_market_data_service.GetListeners();
    
    bond_market_data_service.OnMessage(bond_order_book);
    
}

int main(int argc, const char * argv[]) {
    
    // TestUtilities();
    TestMarketDataService();
    
    return 0;
}
