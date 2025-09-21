#ifndef INFLUX_CLIENT_H
#define INFLUX_CLIENT_H

#include "influxdb.hpp"
#include <string>

class DCIInfluxClient {
public:
    DCIInfluxClient(std::string &influxdb_url, uint32_t &influxdb_port,
                    std::string &influxdb_org, std::string &influxdb_token,
                    std::string &influxdb_bucket, std::string &identifier);
    ~DCIInfluxClient();

    void write_dci_metrics(int rnti, int al, double correlation, int bits_size,
                           const std::string &binary_dci);

private:
    unsigned long long get_epoch_time_nsec();

    influxdb_cpp::server_info server_info_;
    std::string sniffer_id_;
    bool init_success_ = false;
    
    // Statistics
    int total_writes_attempted = 0;
    int total_writes_successful = 0;
    int total_writes_failed = 0;
};

#endif // INFLUX_CLIENT_H
