//
//  bond_services.hpp
//  TradingSystem
//
//  Created by 王明森 on 12/11/22.
//

#ifndef bond_services_hpp
#define bond_services_hpp

#include "products.hpp"

#include "market_data_service.hpp"

typedef OrderBook<Bond> BondOrderBook;
typedef MarketDataService<Bond> BondMarketDataService;

#endif /* bond_services_hpp */
