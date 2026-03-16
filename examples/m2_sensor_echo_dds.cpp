#include <iostream>
#include <chrono>
#include <thread>

// Native CycloneDDS C++ API
#include "dds/dds.hpp"

// Generated header from our IDL files
#include "SensorData.hpp"

using namespace dds::core;
using namespace dds::domain;
using namespace dds::topic;
using namespace dds::sub;
using xterra::msg::dds_::SensorData_;

int main() {
    try {
        // 1. Initialize Domain Participant (Domain 0)
        DomainParticipant participant(0);

        // 2. Create the Topic matching the hardware layer
        Topic<SensorData_> topic(participant, "rt/m2_metal/sim/sensor_data");

        // 3. Create Subscriber and DataReader
        Subscriber subscriber(participant);
        DataReader<SensorData_> reader(subscriber, topic);

        std::cout << "[DDS Sub] Successfully connected to Domain 0." << std::endl;
        std::cout << "[DDS Sub] Listening on topic: rt/m2_metal/sim/sensor_data..." << std::endl;
        
        // 4. Polling loop to continuously read and echo packets
        while (true) {
            // Take available samples from the DDS queue
            auto samples = reader.take();
            
            for (const auto& sample : samples) {
                if (sample.info().valid()) {
                    const SensorData_& msg = sample.data();
                    
                    std::cout << "---\n";
                    std::cout << "Voltage[0]: " << msg.driver_voltage()[0] << " V | "
                              << "Temp[0]: " << msg.fet_temp()[0] << " C\n";
                    std::cout << "Quat: [" << msg.quat()[0] << ", " 
                              << msg.quat()[1] << ", " << msg.quat()[2] << ", " 
                              << msg.quat()[3] << "]\n";
                    std::cout << "Joint[0] Pos: " << msg.q()[0] << " rad\n";
                }
            }
            
            // Sleep to prevent maxing out the CPU core
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    } catch (const dds::core::Exception& e) {
        std::cerr << "DDS Exception: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}