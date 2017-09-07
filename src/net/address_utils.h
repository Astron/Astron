#include <deps/uvw/uvw.hpp>

bool is_valid_address(const std::string &hostspec);

std::vector<uvw::Addr> resolve_address(const std::string &hostspec, uint16_t port, const std::shared_ptr<uvw::Loop> &loop);
