#include "../include/Receipt.hpp"
#include <numeric>

// Constructor for the Receipt class
// Receipt::Receipt(int receiptId, const std::vector<std::pair<Stock, int>>& items, const std::string& username)
//     : receiptId_(receiptId), items_(items), username_(username) {
Receipt::Receipt(int receiptId, const std::vector<std::pair<Stock, int>>& items, const std::string& username)
    : receiptId_(receiptId), items_(items), username_(username) {
    // Calculate total price during object creation
    totalPrice_ = calculateTotalPrice();
    
    // Set transaction time to the current time
    auto now = std::chrono::system_clock::now();
    transactionTime_ = std::chrono::system_clock::to_time_t(now);
}

// Getters for the Receipt class members
int Receipt::getReceiptId() const {
    return receiptId_;
}

const std::vector<std::pair<Stock, int>>& Receipt::getItems() const {
    return items_;
}

double Receipt::getTotalPrice() const {
    return totalPrice_;
}

std::time_t Receipt::getTransactionTime() const {
    return transactionTime_;
}

std::string Receipt::getUsername() const { // Implementation for username getter
    return username_;
}

// Helper function to calculate the total price of all items in the receipt
double Receipt::calculateTotalPrice() const {
    double total = 0.0;
    for (const auto& item_pair : items_) {
        const Stock& stock_item = item_pair.first;
        int quantity = item_pair.second;
        total += stock_item.getPrice() * quantity;
    }
    return total;
}
