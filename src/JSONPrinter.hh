#pragma once
#include "Logging.hh"

#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace cpp2c {
class JSONPrinter {
    using VariantType = std::variant<int, bool, std::string>;
    using KeyValuePair = std::pair<std::string, VariantType>;

private:
    // Only pretty print JSON if debug is on
    static const char sep = Debug ? '\n' : ' ';
    std::string kind;
    std::vector<KeyValuePair> data;

public:
    JSONPrinter(std::string kind);

    void add(std::initializer_list<std::pair<std::string, VariantType> > pairs);
    void printJSONObject() const;
    std::string generateJSONproperty(const std::string &key,
                                     const int &value) const;
    std::string generateJSONproperty(const std::string &key,
                                     const bool &value) const;
    std::string generateJSONproperty(const std::string &key,
                                     const std::string &value) const;

    static void printJSONArray(std::vector<JSONPrinter> &printers);
};
}
