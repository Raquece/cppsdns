#include <cppsdns/dnspack.h>
#include <iostream>
#include <cstring>
#include <stdlib.h>
#include <vector>
#include <memory>
#include <algorithm>

#define BITSWAP_WORD(var) (var >> 8) | (var << 8)
#define BITSWAP_DWORD(dword) (dword & 0x000000ff) << 24 | (dword & 0x0000ff00) << 8 | (dword & 0x00ff0000) >> 8 | (dword & 0xff000000) >> 24

namespace dnspack
{
    uint8_t get_labelc_from_name(std::string name)
    {
        return (uint8_t)std::count(name.begin(), name.end(), '.') + 1;
    }

    std::vector<std::tuple<uint8_t, std::string>> get_labels_from_name(std::string n)
    {
        auto vec = std::vector<std::tuple<uint8_t, std::string>>();

        size_t pos = 0;
        while ((pos = n.find(".")) != std::string::npos)
        {
            std::string segment = n.substr(0, pos);
            auto t = std::tuple<uint8_t, std::string>((uint8_t)segment.length(), segment);
            vec.push_back(t);
            n.erase(0, pos + 1);
        }

        std::string segment = n.substr(0, pos);
        auto t = std::tuple<uint8_t, std::string>((uint8_t)segment.length(), segment);
        vec.push_back(t);

        return vec;
    }

    /// @brief Parse a WORD from DNS payload
    /// @param bytes The DNS payload
    /// @param offset Current offset to begin reading from
    /// @return The parsed WORD
    uint16_t parse_word(std::vector<char> bytes, int &offset)
    {
        uint16_t word_rev = 0;
        
        memcpy(&word_rev, &bytes[offset], sizeof(uint16_t));

        offset += sizeof(uint16_t);

        return BITSWAP_WORD(word_rev);
    }

    /// @brief Parse a string as host name from DNS payload
    /// @param bytes The DNS payload
    /// @param offset Current offset to begin reading from
    /// @return The parsed string
    std::string parse_host_name(std::vector<char> bytes, int &offset)
    {
        std::string p = "";

        // Iterate through payload until hostname ends
        int i = 0; // String index
        int n_val = 0; // Characters remaining in segment
        while(true)
        {
            auto byte = bytes[offset];
            offset++;
            // Check if reached end of segment
            if (n_val == 0)
            {
                // Set characters remaining in segment to current value
                n_val = (int)byte; 

                // If next byte is null terminator, string has
                // ended so break from loop
                if (n_val == 0)
                {
                    break;
                }

                // If next byte is not null terminator, hostname
                // continues as a new segment
                if (i != 0)
                {
                    p.append(".");
                }
            }
            else
            {
                // Append character to hostname
                char c = (char)byte;
                p += c;
                n_val--;
            }

            i++;
        }
        
        return p;
    }

    /// @brief Parse a query from a DNS payload
    /// @param bytes The DNS payload
    /// @param offset Current offset to begin reading from
    /// @return The parsed query
    std::shared_ptr<query> parse_query(std::vector<char> bytes, int &offset)
    {
        auto q = std::make_unique<query>();
        q->name = parse_host_name(bytes, offset);
        q->type = parse_word(bytes, offset);
        q->dns_class = parse_word(bytes, offset);
        q->label_count = get_labelc_from_name(q->name);
        q->segments = get_labels_from_name(q->name);

        return q;
    }

    /// @brief Parse a DNS packet from a UDP payload
    /// @param bytes The DNS payload
    /// @return The parsed DNS payload
    std::shared_ptr<dnspack_query> parse_dns_payload(std::vector<char> bytes)
    {
        int offset = 0;
        auto packet = std::make_shared<dnspack_query>();

        packet->transaction_id = parse_word(bytes, offset);
        packet->flags = parse_word(bytes, offset);
        packet->questions = parse_word(bytes, offset);
        packet->answer_rr = parse_word(bytes, offset);
        packet->authority_rr = parse_word(bytes, offset);
        packet->additional_rr = parse_word(bytes, offset);
        packet->queries = std::vector<std::shared_ptr<query>>();

        for (uint16_t i = 0; i < packet->questions; i++)
        {
            packet->queries.push_back(parse_query(bytes, offset));
        }

        return packet;
    }

    /// @brief Convert a WORD to byte array
    /// @param object WORD to convert
    /// @return The WORD as bytes
    std::array<char, sizeof(uint16_t)> to_bytes(uint16_t object)
    {
        std::array<char, sizeof(uint16_t)> bytes;

        const char *begin = reinterpret_cast<const char*>(std::addressof(object));
        const char *end = begin + sizeof(uint16_t);
        std::copy(begin, end, std::begin(bytes));

        return bytes;
    }

    /// @brief Convert a byte to byte array
    /// @param object Byte to convert
    /// @return The byte as an array
    std::array<char, sizeof(uint8_t)> to_bytes_1(uint8_t object)
    {
        std::array<char, sizeof(uint8_t)> bytes;

        const char *begin = reinterpret_cast<const char*>(std::addressof(object));
        const char *end = begin + sizeof(uint8_t);
        std::copy(begin, end, std::begin(bytes));

        return bytes;
    }

    /// @brief Convert a DWORD to byte array
    /// @param object DWORD to convert
    /// @return The word as bytes
    std::array<char, sizeof(uint32_t)> to_bytes_4(uint32_t object)
    {
        std::array<char, sizeof(uint32_t)> bytes;

        const char *begin = reinterpret_cast<const char*>(std::addressof(object));
        const char *end = begin + sizeof(uint32_t);
        std::copy(begin, end, std::begin(bytes));

        return bytes;
    }

    /// @brief Appends a byte to a vector
    /// @param vec The vector to append to
    /// @param object The byte to append
    void append_object_to_vector_1(std::vector<char> &vec, uint8_t object)
    {
        auto bytes = to_bytes_1(object);

        vec.insert(vec.end(), bytes.begin(), bytes.end());
    }

    /// @brief Appends a WORD to a vector
    /// @param vec The vector to append to
    /// @param object The WORD to append
    void append_object_to_vector(std::vector<char> &vec, uint16_t object)
    {
        auto bytes = to_bytes(object);

        vec.insert(vec.end(), bytes.begin(), bytes.end());
    }

    /// @brief Appends a DWORD to a vector
    /// @param vec The vector to append to
    /// @param object The DWORD to append
    void append_object_to_vector_4(std::vector<char> &vec, uint32_t object)
    {
        auto bytes = to_bytes_4(object);

        vec.insert(vec.end(), bytes.begin(), bytes.end());
    }

    /// @brief Creates a DNS answer object
    /// @param packet The query that is being responded to
    /// @param answers The list of answers to the queries
    /// @return The resulting DNS answer object
    std::shared_ptr<dnspack_ans> get_dnspack_answer(std::shared_ptr<dnspack_query> packet, std::vector<std::shared_ptr<answer>> answers)
    {
        auto pack = std::make_unique<dnspack_ans>();
        
        pack->transaction_id = packet->transaction_id;
        pack->flags = 0x8580;
        pack->questions = packet->questions;
        pack->answer_rr = packet->answer_rr + answers.size();
        pack->authority_rr = packet->authority_rr;
        pack->additional_rr = packet->additional_rr;
        pack->queries = packet->queries;
        pack->answers = answers;

        return pack;
    }

    /// @brief Converts a DNS answer object to a byte vector
    /// @param packet The DNS answer
    /// @return The resulting byte array
    std::vector<char> get_dnspack_answer_bytes(std::shared_ptr<dnspack_ans> packet)
    {
        auto vec = std::vector<char>();

        append_object_to_vector(vec, BITSWAP_WORD(packet->transaction_id));
        append_object_to_vector(vec, BITSWAP_WORD(packet->flags));
        append_object_to_vector(vec, BITSWAP_WORD(packet->questions));
        append_object_to_vector(vec, BITSWAP_WORD(packet->answer_rr));
        append_object_to_vector(vec, BITSWAP_WORD(packet->authority_rr));
        append_object_to_vector(vec, BITSWAP_WORD(packet->additional_rr));

        // Iterates through each query in packet
        for (auto &&q : packet->queries)
        {
            // Create pointer to query and set first to bits to 1 (RFC 1035 4.1.4)
            uint16_t pointer_bitmask {0b11'00'00'00'00'00'00'00};
            uint16_t offset = (uint16_t)vec.size();
            q->offset = offset |= pointer_bitmask;

            // Iterate through each segment of a domain name
            for (auto &&s : q->segments)
            {   
                // Append the length of the segment followed by the segment
                append_object_to_vector_1(vec, std::get<0>(s));
                auto name = std::get<1>(s);
                vec.insert(vec.end(), name.begin(), name.end());
            }
            vec.push_back('\00'); // Null terminator on domain name string

            append_object_to_vector(vec, BITSWAP_WORD(q->type));
            append_object_to_vector(vec, BITSWAP_WORD(q->dns_class));
        }

        int i = 0;
        for (auto &&a : packet->answers)
        {
            append_object_to_vector(vec, BITSWAP_WORD(packet->queries[i]->offset));
            append_object_to_vector(vec, BITSWAP_WORD(a->type));
            append_object_to_vector(vec, BITSWAP_WORD(a->dns_class));

            // TTL: 0s
            vec.push_back('\00');
            vec.push_back('\00');
            vec.push_back('\00');
            vec.push_back('\00');
            
            append_object_to_vector(vec, BITSWAP_WORD(a->data_len));
            vec.insert(vec.end(), a->data.begin(), a->data.end());
            i++;
        }

        return vec;
    }

    /// @brief Converts the string value of a record to its WORD value
    /// @param c The string record
    /// @return The value of the record
    uint16_t get_dns_class(std::string c)
    {
        if (c == "A")
        {
            return 1;
        }

        return 0;
    }

    /// @brief Converts the WORD value of a record to its string value
    /// @param c The WORD record
    /// @return The value of the record
    std::string get_dns_class(uint16_t c)
    {
        switch (c)
        {
        case 1:
            return "A";
        }

        return "NULL";
    }
}


