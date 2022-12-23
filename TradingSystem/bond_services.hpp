//
//  bond_services.hpp
//  TradingSystem
//
//  Created by 王明森 on 12/11/22.
//

#ifndef bond_services_hpp
#define bond_services_hpp

#include "soa.hpp"
#include "products.hpp"
#include "utilities.hpp"

#include "market_data_service.hpp"
#include "execution_order.hpp"
#include "algo_execution_service.hpp"
#include "execution_service.hpp"
#include "trade_booking_service.hpp"
#include "position_service.hpp"
#include "risk_service.hpp"
#include "pricing_service.hpp"
#include "price_stream.hpp"
#include "algo_streaming_service.hpp"
#include "streaming_service.hpp"
#include "historical_data_service.hpp"
#include "inquiry_service.hpp"
#include "gui_service.hpp"

typedef OrderBook<Bond> BondOrderBook;
typedef MarketDataService<Bond> BondMarketDataService;

#endif /* bond_services_hpp */
