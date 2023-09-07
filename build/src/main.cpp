#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <array>
#include <string>
#include <vector>
#include <cppsdns/dnspack.h>
#include <cppsdns/packetfs.h>
#include <cppsdns/bindserv.h>
#include <fstream>
#include <cstdint>
#include <iterator>
#include <tuple>

/// @brief Reads records from records.txt
/// @return 
std::vector<std::tuple<std::string, std::string, std::string>> readRecords()
{
    // Record File
    std::ifstream recordFs("./records.txt");
    auto records = std::vector<std::tuple<std::string, std::string, std::string>>();

    // Iterate through each line of configuration file
    std::string cline;
    int nline = 0;
    while (std::getline(recordFs, cline))
    {   
        std::istringstream iss(cline);
        std::string a;
        std::string b;
        std::string c;
        if (!(iss >> a >> b >> c))
        { 
            std::cerr << "Failure to process records line " << nline << "\n";
        }

        std::tuple<std::string, std::string, std::string> tuple(a, b, c);
        records.insert(records.end(), tuple);

        nline++;
    }

    return records;
}

int main()
{
    auto records = readRecords();    

    bindserv::bind_socket(53);

    while(true)
    {
        auto packet = bindserv::recv();

        std::shared_ptr<dnspack::dnspack_query> dnsquery;
        try
        {
            dnsquery = dnspack::parse_dns_payload(packet);
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }

        auto answers = std::vector<std::shared_ptr<dnspack::answer>>();

        // Iterate through each query
        for (auto &&q : dnsquery->queries)
        {
            auto a = std::make_shared<dnspack::answer>();
            a->name = q->name;
            a->label_count = dnspack::get_labelc_from_name(a->name);
            a->type = q->type;
            a->dns_class = q->dns_class;
            a->ttl = 0;

            auto queryClass = dnspack::get_dns_class(q->type); 

            // Iterate through each recorded record
            bool match = false;
            for (auto &&r : records)
            {
                // Check if record for current query exists
                if (std::get<1>(r) == q->name && std::get<0>(r) == queryClass)
                {
                    match = true;
                    auto s = std::get<2>(r);

                    if (queryClass == "A")
                    {
                        // If query is A record, format data to fit standard
                        // response by converting the IP address string
                        // to 4 bytes
                        auto ip = std::vector<char>();
                        
                        size_t pos = 0;
                        while ((pos = s.find(".")) != std::string::npos) {
                            std::string octet = s.substr(0, pos);
                            auto e = reinterpret_cast<const uint8_t*>(octet.data());
                            ip.push_back((uint8_t)(std::stoi(s)));
                            s.erase(0, pos + 1);
                        }
                        std::string octet = s.substr(0, pos);
                        auto e = reinterpret_cast<const uint8_t*>(octet.data());
                        ip.push_back((uint8_t)(std::stoi(s)));

                        a->data_len = 4;
                        a->data = ip;
                    }
                    else
                    {
                        a->data = std::vector<char>(s.data(), s.data() + s.length() + 1);
                        a->data_len = a->data.size();
                    }
                }
            }

            if (match)
            {
                answers.push_back(a);
            }
        }

        auto p = dnspack::get_dnspack_answer(dnsquery, answers);
        auto bytes = dnspack::get_dnspack_answer_bytes(p);

        bindserv::sendback(&bytes[0], bytes.size());
    }
}