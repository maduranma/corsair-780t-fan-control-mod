#include <libusb-1.0/libusb.h>
#include <curl/curl.h>
#include <iostream>
#include <unistd.h>
#include <thread>
#include <atomic>
#include <format>

#define VID 0xFFFF
#define PID 0x0001

#define OPEN_LINK_HUB_API_PORT 27003
#define OPEN_LINK_HUB_FANS_ID "BA056D93E799445DBBC984FF7D7E9305" // Commander / Link Hub / or similar Device ID on OpenLinkHub
#define OPEN_LINK_HUB_LEDS_OFF_TIME 2000

const std::string FAN_PROFILES[3] = {"Quiet", "Normal", "Performance"};

libusb_device_handle* dev;
uint8_t bulk_in_ep = 0;
uint8_t bulk_out_ep = 0;

uint8_t state = 0;

std::string get_open_link_hub_url(const std::string &endpoint) {
    return "http://127.0.0.1:" + std::to_string(OPEN_LINK_HUB_API_PORT) + "/api/" + endpoint;
}

void send_async(const std::string& endpoint, const std::string& json) {
    std::thread([endpoint, json] {
        CURL* curl = curl_easy_init();

        curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        const std::string url = get_open_link_hub_url(endpoint);
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, json.size());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 10000L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,
            +[](char*, size_t s, size_t n, void*) -> size_t  {
                return s * n;
            }
        );

        curl_easy_perform(curl);

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }).detach();
}


int8_t get_current_fan_profile() {
    CURL* curl = curl_easy_init();

    curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    const std::string url = get_open_link_hub_url(std::string("devices/") + OPEN_LINK_HUB_FANS_ID);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 10000L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,
        +[](char* ptr, size_t s, size_t n, void* data) -> size_t  {
            auto* out = static_cast<std::string*>(data);
            out->append(ptr, s * n);
            return s * n;
        }
    );
    std::string json;
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &json);

    curl_easy_perform(curl);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    size_t pos = json.find("\"MultiProfile\"");
    if (pos == std::string::npos) return -1;
    pos = json.find(':', pos);
    if (pos == std::string::npos) return -1;
    size_t start = json.find_first_not_of(" \t\n", pos + 1);
    size_t end = json.find_first_of(",}", start);
    std::string val = json.substr(start, end - start);
    if (!val.empty() && val.front() == '"') val.erase(0,1);
    if (!val.empty() && val.back() == '"') val.pop_back();

    std::cout << "Found profile: " << val << std::endl;

    for (int8_t i = 0; i < 3; i++) {
        if (FAN_PROFILES[i] == val) { return i; }
    }

    return -1;
}

void set_fans_brightness(uint8_t brightness) {
    send_async("brightness/gradual", std::format(R"(
{{
  "deviceId": "cluster",
  "brightness": {}
}}
)", brightness));
}

void set_fans_profile(const std::string& profile) {
    send_async("speed", std::format(R"(
{{
    "deviceId": "{}",
    "channelId": -1,
    "channelIds": [],
    "profile": "{}"
}}
)", OPEN_LINK_HUB_FANS_ID, profile));
}

// 0 to 100 brightness, -2 breathing effect
void set_button_light(const uint8_t light, const uint8_t brightness) {
    uint8_t hex_to_send[2];
    hex_to_send[0] = light;
    hex_to_send[1] = brightness;

    int send_result = libusb_bulk_transfer(dev, bulk_out_ep, hex_to_send, 2, nullptr, 100);
    if (send_result == LIBUSB_SUCCESS) {
        std::cout << "Sent: 0x" << std::hex << static_cast<int>(hex_to_send[0]) << " 0x" << std::hex << static_cast<int>(hex_to_send[1]) << std::endl;
    } else {
        std::cerr << "Error sending: " << libusb_error_name(send_result) << std::endl;
    }
}

std::atomic<bool> time_passed = false;
std::atomic<bool> is_hidden = false;
std::atomic<uint32_t> timer_current = 0;
void restart_button_timer() {
    uint32_t timer_id = ++timer_current;
    time_passed = false;
    std::thread([&, timer_id]{
        std::this_thread::sleep_for(std::chrono::milliseconds(OPEN_LINK_HUB_LEDS_OFF_TIME));
        if (timer_current == timer_id) {
            time_passed = true;
            is_hidden = true;
            set_fans_brightness(0);
            for (int i = 0; i < 3; i++) {
                set_button_light(i, i <= state ? 10 : 0);
            }
        }
    }).detach();
}

void cancel_button_timer() {
    ++timer_current;
}

void retry();
int main() {
    while (true) {
        libusb_init(nullptr);

        dev = libusb_open_device_with_vid_pid(nullptr, VID, PID);
        if (!dev) {
            std::cerr << "USB device not found!" << std::endl;
            libusb_release_interface(dev, 2);
            libusb_close(dev);
            libusb_exit(nullptr);
            retry();
            continue;
        }

        if (libusb_claim_interface(dev, 2) != 0) {
            std::cerr << "Device found but could not connect" << std::endl;
            libusb_release_interface(dev, 2);
            libusb_close(dev);
            libusb_exit(nullptr);
            retry();
            continue;
        }


        libusb_config_descriptor* cfg;
        libusb_get_config_descriptor(libusb_get_device(dev), 0, &cfg);

        const libusb_interface_descriptor& if2 = cfg->interface[2].altsetting[0];

        for (int k = 0; k < if2.bNumEndpoints; k++) {
            const auto& ep = if2.endpoint[k];

            if ((ep.bmAttributes & LIBUSB_TRANSFER_TYPE_MASK) == LIBUSB_TRANSFER_TYPE_BULK) {
                if (ep.bEndpointAddress & LIBUSB_ENDPOINT_IN)
                    bulk_in_ep = ep.bEndpointAddress;
                else
                    bulk_out_ep = ep.bEndpointAddress;
            }
        }

        libusb_free_config_descriptor(cfg);

        std::cout << "IN:  0x" << std::hex << static_cast<int>(bulk_in_ep) << std::endl;
        std::cout << "OUT: 0x" << std::hex << static_cast<int>(bulk_out_ep) << std::endl;


        std::cout << "Connected to (VID:PID " << std::hex << VID << ":" << std::hex << PID << ")" << std::endl;

        for (int i = 0; i < 3; i++) {
            set_button_light(i, -2);
        }

        int8_t current_fan_profile;
        while ((current_fan_profile = get_current_fan_profile()) == -1) {
            std::cerr << "Cannot establish connection to OpenLinkHub" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }

        state = current_fan_profile;

        set_fans_brightness(100);
        for (int i = 0; i < 3; i++) {
            set_button_light(i, i <= state ? 100 : 0);
        }

        uint8_t hex_value[1];

        uint8_t last_received = 0;
        while (true) {
            int receive_result = libusb_bulk_transfer(dev, bulk_in_ep, hex_value, 1, nullptr, 100);

            if (receive_result == LIBUSB_SUCCESS) {
                uint8_t btn = hex_value[0];
                std::cout << "Rcvd: " << static_cast<int>(btn) << std::endl;

                if (last_received == btn) {
                    std::cerr << "Duplicated input! Skipping." << std::endl;
                    continue;
                }

                last_received = btn;

                if (btn == 1) {
                    restart_button_timer();
                }
                if (btn == 0) {
                    if (time_passed) {
                        continue;
                    }

                    cancel_button_timer();

                    if (is_hidden) {
                        is_hidden = false;
                        set_fans_brightness(100);
                    } else {
                        state = (state + 1) % 3;
                        set_fans_profile(FAN_PROFILES[state]);
                    }


                    for (int i = 0; i < 3; i++) {
                        set_button_light(i, i <= state ? 100 : 0);
                    }
                }
                continue;
            }

            if (receive_result == LIBUSB_ERROR_TIMEOUT) {
                continue;
            }

            break;
        }

        std::cerr << "Got disconnected!" << std::endl;

        libusb_release_interface(dev, 2);
        libusb_close(dev);
        libusb_exit(nullptr);

        retry();
    }
}

void retry() {
    std::cout << "Waiting to reconnect..." << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    std::cout << "Reconnecting..." << std::endl;
}
