#include "JSONPrinter.hh"

namespace cpp2c {
    JSONPrinter::JSONPrinter(std::string k) : kind(std::move(k)) {}


    void JSONPrinter::add(std::initializer_list<std::pair<std::string, VariantType>> pairs) {
        for(const auto& pair : pairs) {
            data.emplace_back(pair);
        }
    }

    void JSONPrinter::printJSONObject() const {

        // Open json object
        llvm::outs() << "{" << sep;

        // Emit kind
        llvm::outs() << generateJSONproperty("Kind", kind);

        // Emit data
        for(const auto& [key, value] : data) {
            // Lambda must capture this for generateJSONproperty to work,
            // and key must be captured by reference (cannot capture structured bindings before C++20)
            std::visit([this, &key = key](const auto& value) {
                llvm::outs() << ',' << sep << generateJSONproperty(key, value);
            }, value);
        }

        // Close json object
        llvm::outs() << sep << "}";
    }

    std::string JSONPrinter::generateJSONproperty(const std::string& key, const int& value) const {
        return "    \"" + key + "\" : " + std::to_string(value);
    }
    std::string JSONPrinter::generateJSONproperty(const std::string& key, const bool& value) const {
        return "    \"" + key + "\" : " + (value ? "true" : "false");
    }
    std::string JSONPrinter::generateJSONproperty(const std::string& key, const std::string& value) const {
        return "    \"" + key + "\" : \"" + value + "\"";
    }

    void JSONPrinter::printJSONArray(std::vector<JSONPrinter>& printers) {
        // Open json array
        llvm::outs() << "[" << sep;

        // Emit data
        for(const auto& printer : printers) {
            printer.printJSONObject();

            // Add comma if not last element
            if(&printer != &printers.back()) {
                llvm::outs() << ',';
            }
            llvm::outs() << sep;
        }

        // Close json array
        llvm::outs() << sep << "]\n";
    }
}
