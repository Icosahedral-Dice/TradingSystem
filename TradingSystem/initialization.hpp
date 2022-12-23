/**
 * initialization.hpp
 * Generates bond price data
 *
 * @author Mingsen Wang
 */

#ifndef initialization_hpp
#define initialization_hpp

#include "utilities.hpp"

#include <random>
#include <fstream>

namespace initialization {

// Generate Bernoulli(0.5), used in price generation (during initialization of `price.txt`)
class BernoulliRng {
private:
    // Random device
    static thread_local random_device rv_;
    
    // Random engine initially seeded with rv
    static thread_local mt19937_64 eng_;
    
    // Standard uniform distribution
    static thread_local bernoulli_distribution dist_;
    
public:
    // No instances of UniformRng is needed.
    BernoulliRng() = delete;
    ~BernoulliRng() = default;
    
    // Generate new standard uniform random variable
    static bool gen();
    
    static void reseed(unsigned new_seed);
};

// Random device (used to seed the engine)
thread_local random_device BernoulliRng::rv_ = std::random_device();

// Seed engine with std::random_device
thread_local mt19937_64 BernoulliRng::eng_ = mt19937_64(BernoulliRng::rv_());

// Standard normal distribution N(0, 1)
thread_local bernoulli_distribution BernoulliRng::dist_ = bernoulli_distribution(.5);

// Generate a new standard uniform random variable
bool BernoulliRng::gen() {
    return ((BernoulliRng::dist_)(BernoulliRng::eng_));
}

// Re-seed the engine with custom seed
void BernoulliRng::reseed(unsigned new_seed) {
    BernoulliRng::eng_.seed(new_seed);
}

void GeneratePrice(string bond_id, unsigned number_of_prices, ofstream& file) {
    
    // Minimal increment
    float increment = 1. / 256.;
    
    // Mid price oscillates between 99-002 and 100-316
    float mid_price_lower_limit = 99. + increment * 2.;
    float mid_price_upper_limit = 101. - increment * 2.;
    float mid_price = mid_price_lower_limit;
    
    // Goes up and down and up and down...
    bool price_goes_up = true;
    
    // Generate bid and ask around central price and write to file.
    // Bid has 50% prob to be (central price - 1/256)
    //  and 50% prob to be (central price - 2/256)
    // Ask has 50% prob to be (central price + 1/256)
    //  and 50% prob to be (central price + 2/256)
    for (unsigned i = 0; i < number_of_prices; i++) {
        float bid = mid_price - increment;
        float ask = mid_price + increment;
        
        if (BernoulliRng::gen()) {
            bid -= increment;
        }
        
        if (BernoulliRng::gen()) {
            ask += increment;
        }
        
        // Change mid price
        if (price_goes_up) {
            mid_price += increment;
            if (mid_price == mid_price_upper_limit) {
                price_goes_up = false;
            }
        } else {
            mid_price -= increment;
            if (mid_price == mid_price_lower_limit) {
                price_goes_up = true;
            }
        }
        
        // Write to file!
        file << bond_id << ',' << ConvertPrice(bid) << ',' << ConvertPrice(ask) << '\n';
//        file << bond_id << ',' << bid * 256. << ',' << ask * 256. << '\n';
    }
}

void GenerateAllBondPrices () {
    // !!!: ADDRESS MAY VARY
    const string file_address("prices.txt");
    ofstream file(file_address);
    
    // Number of prices per bond
    unsigned number_of_prices = 10000;
    for (const auto& [year, bond] : kBondMapMaturity) {
        cout << "Generating prices for " << bond.first << "... ";
        GeneratePrice(bond.first, number_of_prices, file);
        cout << "Success!" << endl;
    }
}

void GenerateMarketData(string bond_id, unsigned number_of_books, ofstream& file) {
    
    // Minimal increment
    float increment = 1. / 256.;
    
    // Mid price oscillates between 99-010 and 100-310
    float mid_price_lower_limit = 99. + increment * 8.;
    float mid_price_upper_limit = 101. - increment * 8.;
    float mid_price = mid_price_lower_limit;
    
    // Goes up and down and up and down...
    bool price_goes_up = true;
    
    for (unsigned i = 0; i < number_of_books; i++) {
        unsigned spread = i % 4 + 1;
        for (int j = 0; j < 5; j++) {
            float bid = mid_price - increment * (spread + j);
            float ask = mid_price + increment * (spread + j);
            // Write to file!
            file << bond_id << ',' << ConvertPrice(bid) << ',' << ((j+1) * 10000000) << ',' << "BID" << '\n';
            file << bond_id << ',' << ConvertPrice(ask) << ',' << ((j+1) * 10000000) << ',' << "OFFER" << '\n';
        }
        
        // Change mid price
        if (price_goes_up) {
            mid_price += increment;
            if (mid_price == mid_price_upper_limit) {
                price_goes_up = false;
            }
        } else {
            mid_price -= increment;
            if (mid_price == mid_price_lower_limit) {
                price_goes_up = true;
            }
        }
    }
}

void GenerateAllMarketData() {
    // !!!: ADDRESS MAY VARY
    const string file_address("marketdata.txt");
    ofstream file(file_address);
    
    // Number of prices per bond
    unsigned number_of_books = 10000;
    for (const auto& [year, bond] : kBondMapMaturity) {
        cout << "Generating market data for " << bond.first << "... ";
        GenerateMarketData(bond.first, number_of_books, file);
        cout << "Success!" << endl;
    }
}

void GenerateInquiries(string bond_id, unsigned number_of_inquiries, ofstream& file) {
    for (unsigned i = 0; i < number_of_inquiries; i++) {
        float price = 100.;
        if (i % 2) {
            price -= (i << 1) / 256.;
        } else {
            price += (i << 1) / 256.;
        }
        file << bond_id << '0' << i << ',' << bond_id << ',' << ((i % 2) ? "BUY" : "SELL") << ',' << ((i << 1) + 1) * 1000000 << ',' << ConvertPrice(price) << ',' << "RECEIVED\n";
    }
}

void GenerateAllInquiries() {
    // !!!: ADDRESS MAY VARY
    const string file_address("inquiries.txt");
    ofstream file(file_address);
    
    // Number of prices per bond
    unsigned number_of_inquiries = 10;
    for (const auto& [year, bond] : kBondMapMaturity) {
        cout << "Generating inquiries for " << bond.first << "... ";
        GenerateInquiries(bond.first, number_of_inquiries, file);
        cout << "Success!" << endl;
    }
}

void GenerateTrades(string bond_id, unsigned number_of_trades, ofstream& file) {
    for (unsigned i = 0; i < number_of_trades; i++) {
        float price = 100.;
        if (i % 2) {
            price -= (i << 1) / 256.;
        } else {
            price += (i << 1) / 256.;
        }
        file << bond_id << ',' << bond_id << '0' << i << ',' << ConvertPrice(price) << ',' << "TRSY" << (i % 3 + 1) << ',' << ((i << 1) + 1) * 1000000 << ',' << ((i % 2) ? "BUY" : "SELL") << '\n';
    }
}

void GenerateAllTrades() {
    // !!!: ADDRESS MAY VARY
    const string file_address("trades.txt");
    ofstream file(file_address);
    
    // Number of prices per bond
    unsigned number_of_trades = 10;
    for (const auto& [year, bond] : kBondMapMaturity) {
        cout << "Generating trades for " << bond.first << "... ";
        GenerateTrades(bond.first, number_of_trades, file);
        cout << "Success!" << endl;
    }
}


}

#endif /* initialization_h */
