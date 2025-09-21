#include "influx_client.h"
#include "../hdr/influxdb.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <time.h>
#include <cstdio>

DCIInfluxClient::DCIInfluxClient(std::string &influxdb_url,
                                 uint32_t    &influxdb_port,
                                 std::string &influxdb_org,
                                 std::string &influxdb_token,
                                 std::string &influxdb_bucket,
                                 std::string &identifier)
    : server_info_(influxdb_url, influxdb_port, influxdb_bucket, "", "",
                   "ns", influxdb_token),
      sniffer_id_(identifier) {

    init_success_ = true;
    std::cout << "InfluxDB Client: " << influxdb_url << ":" << influxdb_port 
              << " -> " << influxdb_bucket << std::endl;
}

DCIInfluxClient::~DCIInfluxClient() {
    std::cout << "InfluxDB Statistics: Attempted=" << total_writes_attempted 
              << ", Successful=" << total_writes_successful 
              << ", Failed=" << total_writes_failed << std::endl;
}

void DCIInfluxClient::write_dci_metrics(int rnti, int al, double correlation,
                                        int bits_size,
                                        const std::string &binary_dci) {
    if (!init_success_) {
        total_writes_failed++;
        return;
    }
    
    total_writes_attempted++;
    
    std::ostringstream line;
    line << "dci_metrics,sniffer_id=" << sniffer_id_ << ",rnti=" << rnti 
         << ",aggregation_level=" << al
         << " correlation=" << correlation << ",bits_size=" << bits_size 
         << ",binary_dci=\"" << binary_dci << "\" " << get_epoch_time_nsec();
    
    // Execute curl command
    std::ostringstream cmd;
    std::string host = server_info_.host_;
    // Remove http:// prefix if present
    if (host.find("http://") == 0) {
        host = host.substr(7);
    }
    cmd << "curl -s -XPOST \"" << host << ":" << server_info_.port_ 
        << "/api/v2/write?bucket=" << server_info_.db_ << "&org=MSU&precision=ns\" "
        << "-H \"Authorization: Token " << server_info_.token_ << "\" "
        << "-d '" << line.str() << "' >/dev/null 2>&1";
    
    int result = system(cmd.str().c_str());
    
    if (result == 0) {
        total_writes_successful++;
    } else {
        total_writes_failed++;
        std::cout << "INFLUXDB FAILED: curl returned " << result << std::endl;
        std::cout << "Command: " << cmd.str() << std::endl;
    }
}

unsigned long long DCIInfluxClient::get_epoch_time_nsec() {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (unsigned long long)ts.tv_sec * 1000000000 + ts.tv_nsec;
}