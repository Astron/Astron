#pragma once
#include <string>
#include <yaml-cpp/yaml.h>

// Compatibility with namespace YAML primitives
namespace YAML
{
template<>
struct convert<uint128_t> {
    static Node encode(const uint128_t& rhs)
    {
        char buffer[35];
        sprintf(buffer, "0x%016lx%016lx", rhs.high, rhs.low);
        return Node(std::string(buffer));
    }

    static bool decode(const Node& node, uint128_t& rhs)
    {
        // Get string value of node
        if(!node.IsScalar()) {
            return false;
        }
        std::string input = node.Scalar();

        // Check if string is hex
        int mode = 10;
        if(input.substr(0, 2) == "0x") {
            mode = 16;
            input = input.substr(2);
        }

        // Initialize the uint128 to 0
        rhs = 0;

        if(mode == 16) {
            // Check to make sure its a valid length for a uint128
            if(input.length() > 32) {
                return false;
            }


            // If length is greater than 16, use the upper 64 bits of uint128
            char* end;
            int low_start = 0;
            if(input.length() > 16) {
                low_start = input.length() - 16;
                std::string high = input.substr(0, low_start);
                rhs.high = strtoull(high.c_str(), &end, mode);
                if(*end != '\0' || (rhs.high == 0 && end == high.c_str())) {
                    // Not a valid number
                    return false;
                }
            }

            // Then get the lower 64 bits
            std::string low = input.substr(low_start);
            rhs.low = strtoull(low.c_str(), &end, mode);
            if(*end != '\0' || (rhs.low == 0 && end == low.c_str())) {
                // Not a valid number
                return false;
            }
        } else {
            char* end;
            rhs.low = strtoull(input.c_str(), &end, mode);
            if(*end != '\0' || (rhs.low == 0 && end == input.c_str())) {
                // Not a valid number
                return false;
            }
        }
        return true;
    }
};
}
