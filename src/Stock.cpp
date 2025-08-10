#include "../include/Stock.hpp"

// Default constructor implementation
Stock::Stock() : m_id(0), m_name(""), m_quantity(0), m_price(0.0) {}

// Parameterized constructor implementation
Stock::Stock(int id, const std::string& name, int quantity, double price)
    : m_id(id), m_name(name), m_quantity(quantity), m_price(price) {}

// Getter implementations
int Stock::getId() const {
    return m_id;
}

std::string Stock::getName() const {
    return m_name;
}

int Stock::getQuantity() const {
    return m_quantity;
}

double Stock::getPrice() const {
    return m_price;
}

// Setter implementations
void Stock::setId(int id) {
    m_id = id;
}

void Stock::setName(const std::string& name) {
    m_name = name;
}

void Stock::setQuantity(int quantity) {
    m_quantity = quantity;
}

void Stock::setPrice(double price) {
    m_price = price;
}
