#ifndef STOCK_HPP
#define STOCK_HPP

#include <string>

// A class to represent an item in stock
class Stock {
public:
    // Default constructor
    Stock();

    // Parameterized constructor
    Stock(int id, const std::string& name, int quantity, double price);

    // Getters for stock properties
    int getId() const;
    std::string getName() const;
    int getQuantity() const;
    double getPrice() const;

    // Setters for stock properties
    void setId(int id);
    void setName(const std::string& name);
    void setQuantity(int quantity);
    void setPrice(double price);

private:
    int m_id;
    std::string m_name;
    int m_quantity;
    double m_price;
};

#endif // STOCK_HPP
