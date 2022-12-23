# TradingSystem
MTH9815 final project

Implemented the requested trading system for bonds.

Notes:
- This project is written with Xcode, but can be compiled using GCC (with C++ 20 functionalities).
- The services are built with bond trading in mind, but most functionalities can be adapted to other products easily. (For example, MarketDataService<AnyProductIsOkay>))
- `const` qualifiers are dropped in several places since the base class defined in `soa.hpp` has abstrac methods without the qualifier.
- Due to time constraints, io is implemented using `fstream` instead of `asio`. Can be fixed if more time is given.
- More subtle changes are documented in respective files.
