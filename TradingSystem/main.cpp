//
//  main.cpp
//  TradingSystem
//
//  Created by 王明森 on 12/5/22.
//

#include <iostream>
#include <iomanip>
#include "initialization.hpp"

#include "bond_services.hpp"

using namespace std;

void TestUtilities() {
    cout << FetchCusip(2) << endl;
    cout << FetchCusip(3) << endl;
    
    Bond bond = FetchBond(3);
    cout << bond.GetProductId() << endl;
    cout << bond.GetMaturityDate() << endl;
    
    cout << GetTimestamp() << endl;
}

//void TestMarketDataService() {
//
//    vector<Order> bid_stack;
//    vector<Order> offer_stack;
//
//    for (unsigned i = 1; i < 10; i++) {
//        Order bid(100. - i / 256., i * i, BID);
//        Order offer(100. + i / 256., i * i, OFFER);
//
//        bid_stack.push_back(bid);
//        offer_stack.push_back(offer);
//    }
//
//    for (unsigned i = 9; i > 0; i--) {
//        Order bid(100. - i / 256., i * i, BID);
//        Order offer(100. + i / 256., i * i, OFFER);
//
//        bid_stack.push_back(bid);
//        offer_stack.push_back(offer);
//    }
//
//
//    cout << kBondMapMaturity[2].first << endl;
//    Bond two_year_note = FetchBond(2);
//    BondOrderBook bond_order_book(two_year_note, bid_stack, offer_stack);
//    BidOffer best_bid_offer = bond_order_book.GetBidOffer();
//    cout << ConvertPrice(best_bid_offer.GetBidOrder().GetPrice()) << " - " << ConvertPrice(best_bid_offer.GetOfferOrder().GetPrice()) << endl;
//
//
//    BondMarketDataService bond_market_data_service;
//
//    bond_market_data_service.OnMessage(bond_order_book);
//
//    cout << bond_market_data_service.GetData(FetchCusip(2)).GetBidOffer().GetBidOrder().GetQuantity() << endl;
//
//    bond_market_data_service.AggregateDepth(FetchCusip(2));
//
//    cout << bond_market_data_service.GetData(FetchCusip(2)).GetBidOffer().GetBidOrder().GetQuantity() << endl;
//
//    bond_market_data_service.GetListeners();
//
//    bond_market_data_service.OnMessage(bond_order_book);
//
//}

void Test() {
    cout << GetTimestamp() << " Program Starting..." << endl;
    cout << GetTimestamp() << " Program Started." << endl;

    cout << GetTimestamp() << " Services Initializing..." << endl;
    PricingService<Bond> pricing_service;
    TradeBookingService<Bond> trade_booking_service;
    PositionService<Bond> position_service;
    RiskService<Bond> risk_service;
    MarketDataService<Bond> market_data_service;
    AlgoExecutionService<Bond> algo_execution_service;
    AlgoStreamingService<Bond> algo_streaming_service;
    GUIService<Bond> gui_service;
    ExecutionService<Bond> execution_service;
    StreamingService<Bond> streaming_service;
    InquiryService<Bond> inquiry_service;
    HistoricalDataService<Position<Bond>> historical_position_service(POSITION);
    HistoricalDataService<PV01<Bond>> historical_risk_service(RISK);
    HistoricalDataService<ExecutionOrder<Bond>> historical_execution_service(EXECUTION);
    HistoricalDataService<PriceStream<Bond>> historical_streaming_service(STREAMING);
    HistoricalDataService<Inquiry<Bond>> historical_inquiry_service(INQUIRY);
    cout << GetTimestamp() << " Services Initialized." << endl;
    
    cout << GetTimestamp() << " Services Linking..." << endl;
    pricing_service.AddListener(algo_streaming_service.GetInListener());
    pricing_service.AddListener(gui_service.GetInListener());
    algo_streaming_service.AddListener(streaming_service.GetInListener());
    streaming_service.AddListener(historical_streaming_service.GetInListener());
    market_data_service.AddListener(algo_execution_service.GetInListener());
    algo_execution_service.AddListener(execution_service.GetInListener());
    execution_service.AddListener(trade_booking_service.GetInListener());
    execution_service.AddListener(historical_execution_service.GetInListener());
    trade_booking_service.AddListener(position_service.GetInListener());
    position_service.AddListener(risk_service.GetInListener());
    position_service.AddListener(historical_position_service.GetInListener());
    risk_service.AddListener(historical_risk_service.GetInListener());
    inquiry_service.AddListener(historical_inquiry_service.GetInListener());
    cout << GetTimestamp() << " Services Linked." << endl;
    
    cout << GetTimestamp() << " Price Data Processing..." << endl;
    ifstream price_data("prices.txt");
    pricing_service.GetConnector()->Subscribe(price_data);
    cout << GetTimestamp() << " Price Data Processed." << endl;

    cout << GetTimestamp() << " Trade Data Processing..." << endl;
    ifstream trade_data("trades.txt");
    trade_booking_service.GetConnector()->Subscribe(trade_data);
    cout << GetTimestamp() << " Trade Data Processed." << endl;

    cout << GetTimestamp() << " Market Data Processing..." << endl;
    ifstream market_data("marketdata.txt");
    market_data_service.GetConnector()->Subscribe(market_data);
    cout << GetTimestamp() << " Market Data Processed." << endl;

    cout << GetTimestamp() << " Inquiry Data Processing..." << endl;
    ifstream inquiry_data("inquiries.txt");
    inquiry_service.GetConnector()->Subscribe(inquiry_data);
    cout << GetTimestamp() << " Inquiry Data Processed." << endl;
    
}

int main(int argc, const char * argv[]) {
    
    // TestUtilities();
//    TestMarketDataService();
    
    initialization::GenerateAllBondPrices();
    initialization::GenerateAllMarketData();
    initialization::GenerateAllTrades();
    initialization::GenerateAllInquiries();
    
    Test();
    
    
    return 0;
}
