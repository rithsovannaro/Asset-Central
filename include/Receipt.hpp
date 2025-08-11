#ifndef RECEIPT_HPP
#define RECEIPT_HPP

#include <string>
#include <vector>
#include <chrono>
#include "Stock.hpp"

class Receipt {
public:
    // A pair of Stock item and its purchased quantity
    using Item = std::pair<Stock, int>;

    // Constructor
    Receipt(int receiptId, const std::vector<Item>& items, const std::string& username);

    // Getters
    int getReceiptId() const;
    const std::vector<Item>& getItems() const;
    double getTotalPrice() const;
    std::time_t getTransactionTime() const;
    std::string getUsername() const; // Added getter for username
    
private:
    int receiptId_;
    std::vector<Item> items_;
    double totalPrice_;
    std::time_t transactionTime_;
    std::string username_; // Added username member

    // Private helper function to calculate the total price
    double calculateTotalPrice() const;
};

#endif // RECEIPT_HPP
