#include <cstring>
#include <iostream>
#include <optional>

#ifdef _WIN32
    #include <ws2tcpip.h>
#else
    #include <arpa/inet.h>
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netdb.h>
#endif

bool net_init()
{
#ifdef _WIN32
    WSAData data;
    if (WSAStartup(MAKEWORD(2, 2), &data)) {
        return false;
    }

    if (HIBYTE(data.wVersion) != 2 || LOBYTE(data.wVersion) != 2) {
        WSACleanup();
        return false;
    }
#endif

    return true;
}

std::optional<in_addr> resolve(const std::string& name)
{
    addrinfo* addrs;
    addrinfo hints {};
    hints.ai_family = AF_INET;
    if (getaddrinfo(name.c_str(), nullptr, &hints, &addrs)) {
        return std::nullopt;
    }

    bool ok = false;
    in_addr result {};
    for (auto iter = addrs; iter != nullptr; iter = iter->ai_next) {
        if (iter->ai_family == AF_INET) {
            auto& addr = reinterpret_cast<sockaddr_in&>(*iter->ai_addr);
            result = addr.sin_addr;
            ok = true;
            break;
        }
    }

    freeaddrinfo(addrs);
    return ok ? std::make_optional<in_addr>(result) : std::nullopt;
}

constexpr char alphabet[37] = {
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h',
    'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p',
    'q', 'r', 's', 't', 'u', 'v', 'w', 'x',
    'y', 'z', '0', '1', '2', '3', '4', '5',
    '6', '7', '8', '9', '-'
};

constexpr std::int8_t next_char[256] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0, -1, -1,
    27, 28, 29, 30, 31, 32, 33, 34, 35, 36, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
};

int main(int argc, char** argv)
{
    auto usage = []() {
        std::cout << "usage: dns-checker [--verbose] <main-domain>" << '\n';
        return 0;
    };

    if (argc < 2 || argc > 3) {
        return usage();
    }

    bool verbose = false;
    if (!std::strcmp(argv[1], "--verbose")) {
        if (argc != 3) {
            return usage();
        }

        verbose = true;
    } else {
        if (argc == 3) {
            return usage();
        }
    }

    std::string domain = argv[1 + verbose];
    if (domain.empty() || domain.size() > 128) {
        return usage();
    }

    if (!net_init()) {
        std::cout << "cannot initialize network subsystem" << '\n';
        return 0;
    }

    domain.insert(domain.begin(), '.');
    std::size_t length = 0;

    while (length++ < 10) {
        domain.insert(domain.begin(), alphabet[0]);

        auto shift = [&](char& c) {
            std::uint8_t aux = c;
            aux = next_char[aux];
            c = alphabet[aux];
        };

        auto is_done = [&]() {
            for (std::size_t i = 0; i < length; i++) {
                if (domain[i] != alphabet[0]) {
                    return false;
                }
            }
            return true;
        };

        auto k = length - 1;
        std::uint64_t counter = 0;

        do {
            if (verbose) {
                std::cout << "gonna resolve " << domain << std::flush;
            }

            if (auto addr = resolve(domain)) {
                if (!verbose) {
                    std::cout << domain;
                }

                std::cout << " --> " << inet_ntoa(*addr) << '\n';
            } else {
                if (verbose) {
                    std::cout << '\n';
                }
            }

            counter++;
            shift(domain[k]);
            std::uint64_t value = 1;
            for (std::size_t i = 1; i < length; i++) {
                value *= sizeof(alphabet);
                if (!(counter % value)) {
                    shift(domain[k - i]);
                }
            }
        } while (!is_done());
    }

    return 0;
}
