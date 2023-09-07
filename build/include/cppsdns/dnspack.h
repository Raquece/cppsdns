#include <string>
#include <array>
#include <vector>
#include <memory>

namespace dnspack
{
    class query
    {
    public:
        uint8_t label_count;
        std::string name;
        uint16_t type;
        uint16_t dns_class;
        uint16_t offset;
        std::vector<std::tuple<uint8_t, std::string>> segments;
    };

    class answer
    {
    public:
        uint8_t label_count;
        std::string name;
        uint16_t type;
        uint16_t dns_class;
        uint32_t ttl;
        uint16_t data_len;
        std::vector<char> data;
    };

    class dnspack_query
    {
    public:
        uint16_t transaction_id;
        uint16_t flags;
        uint16_t questions;
        uint16_t answer_rr;
        uint16_t authority_rr;
        uint16_t additional_rr;
        std::vector<std::shared_ptr<query>> queries;
    };

    class dnspack_ans
    {
    public:
        uint16_t transaction_id;
        uint16_t flags;
        uint16_t questions;
        uint16_t answer_rr;
        uint16_t authority_rr;
        uint16_t additional_rr;
        std::vector<std::shared_ptr<query>> queries;
        std::vector<std::shared_ptr<answer>> answers;
    };

    uint16_t parse_word(std::vector<char> bytes, int &offset);
    std::string parse_host_name(std::vector<char> bytes, int &offset);
    std::shared_ptr<query> parse_query(std::vector<char> bytes, int &offset);
    std::shared_ptr<dnspack_query> parse_dns_payload(std::vector<char> bytes);
    std::vector<char> get_dnspack_query_bytes(dnspack_query *packet);
    std::shared_ptr<dnspack_ans> get_dnspack_answer(std::shared_ptr<dnspack_query> packet, std::vector<std::shared_ptr<answer>> answers);
    std::vector<char> get_dnspack_answer_bytes(std::shared_ptr<dnspack_ans> packet);
    uint16_t get_dns_class(std::string c);
    std::string get_dns_class(uint16_t c);
    uint8_t get_labelc_from_name(std::string name);
}
