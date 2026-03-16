#include <memory>
#include <functional>

#include "rclcpp/rclcpp.hpp"
#include "xterra/msg/sensor_data.hpp"

class SensorEchoNode : public rclcpp::Node
{
public:
    SensorEchoNode() : Node("svan_sensor_echo_ros2")
    {
        // Set QoS depth to 1 for high-frequency, low-latency telemetry
        sub_ = this->create_subscription<xterra::msg::SensorData>(
            "/m2_metal/sim/sensor_data", 1,
            std::bind(&SensorEchoNode::sensor_callback, this, std::placeholders::_1));

        RCLCPP_INFO(this->get_logger(), "Successfully connected to ROS 2 network.");
        RCLCPP_INFO(this->get_logger(), "Listening on topic: /m2_metal/sim/sensor_data...");
    }

private:
    void sensor_callback(const xterra::msg::SensorData::SharedPtr msg)
    {
        // Throttle output to 20Hz (50ms) to avoid terminal spam
        RCLCPP_INFO_THROTTLE(this->get_logger(), *this->get_clock(), 50,
            "---\nVoltage: %.1f V | Temp: %.1f C\nQuat: [%.3f, %.3f, %.3f, %.3f]\nJoint[0] Pos: %.3f rad",
            msg->driver_voltage[0],
            msg->fet_temp[0],
            msg->quat[0], msg->quat[1], msg->quat[2], msg->quat[3],
            msg->q[0]);
    }

    rclcpp::Subscription<xterra::msg::SensorData>::SharedPtr sub_;
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<SensorEchoNode>());
    rclcpp::shutdown();
    return 0;
}
