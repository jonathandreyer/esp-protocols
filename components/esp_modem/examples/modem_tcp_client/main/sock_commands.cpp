// Copyright 2022 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <charconv>
#include <cstring>
#include "sock_commands.hpp"

namespace sock_commands {

static const char *TAG = "sock_commands";

using namespace esp_modem;

command_result net_open(CommandableIf *t)
{
    ESP_LOGV(TAG, "%s", __func__ );
    std::string_view out;
    auto ret = dce_commands::generic_get_string(t, "AT+QISTATE?\r", out, 1000);
    if (ret != command_result::OK) {
        return ret;
    }
    if (out.find("+QISTATE: 0") != std::string::npos) {
        ESP_LOGV(TAG, "%s", out.data() );
        ESP_LOGD(TAG, "Already there");
        return command_result::OK;
    } else if (out.empty()) {
        return dce_commands::generic_command(t, "AT+QIACT=1\r", "OK", "ERROR", 150000);
    }
    return command_result::FAIL;
}

command_result net_close(CommandableIf *t)
{
    ESP_LOGV(TAG, "%s", __func__ );
    return dce_commands::generic_command(t, "AT+QIDEACT=1\r", "OK", "ERROR", 40000);
}

command_result tcp_open(CommandableIf *t, const std::string& host, int port, int timeout)
{
    ESP_LOGV(TAG, "%s", __func__ );
    std::string ip_open = R"(AT+QIOPEN=1,0,"TCP",")" + host + "\"," + std::to_string(port) + "\r";
    auto ret = dce_commands::generic_command(t, ip_open, "+QIOPEN: 0,0", "ERROR", timeout);
    if (ret != command_result::OK) {
        ESP_LOGE(TAG, "%s Failed", __func__ );
        return ret;
    }
    return command_result::OK;
}

command_result tcp_close(CommandableIf *t)
{
    ESP_LOGV(TAG, "%s", __func__ );
    return dce_commands::generic_command(t, "AT+QICLOSE=0\r", "OK", "ERROR", 10000);
}

command_result tcp_send(CommandableIf *t, uint8_t *data, size_t len)
{
    ESP_LOGV(TAG, "%s", __func__ );
    assert(0);      // Remove when fix done
    return command_result::FAIL;
}

command_result tcp_recv(CommandableIf *t, uint8_t *data, size_t len, size_t &out_len)
{
    ESP_LOGV(TAG, "%s", __func__ );
    assert(0);      // Remove when fix done
    return command_result::FAIL;
}

command_result get_ip(CommandableIf *t, std::string& ip)
{
    ESP_LOGV(TAG, "%s", __func__ );
    std::string_view out;
    auto ret = dce_commands::generic_get_string(t, "AT+QIACT?\r", out, 5000);
    if (ret != command_result::OK) {
        return ret;
    }
    auto pos = out.find("+QIACT: 1");
    auto property = 0;
    while (pos != std::string::npos) {
        // Looking for: +QIACT: <contextID>,<context_state>,<context_type>,<IP_address>
        if (property++ == 3) {  // ip is after 3rd comma (as a 4rd property of QIACT string)
            ip = out.substr(++pos);
            // strip quotes if present
            auto quote1 = ip.find('"');
            auto quote2 = ip.rfind('"');
            if (quote1 != std::string::npos && quote2 != std::string::npos) {
                ip = ip.substr(quote1 + 1, quote2 - 1);
            }
            return command_result::OK;
        }
        pos = out.find(',', ++pos);
    }
    return command_result::FAIL;
}



}

