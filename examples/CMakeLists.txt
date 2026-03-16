# examples/CMakeLists.txt
# Builds the SDK subscriber examples. This file is included only when
# BUILD_EXAMPLES=ON is passed to the parent CMakeLists.txt.
#
# Exactly one of BUILD_DDS or BUILD_ROS2 must be ON (enforced by the parent).

if(BUILD_DDS)
    message(STATUS "Building CycloneDDS examples...")

    add_executable(svan_sensor_echo_dds m2_sensor_echo_dds.cpp)
    target_link_libraries(svan_sensor_echo_dds
        svan_m2_idl_lib        # Generated IDL types + CycloneDDS transitive dependency
        CycloneDDS-CXX::ddscxx # CycloneDDS C++ API
    )

    # Place the built binary under build/bin/ for easy discovery.
    set_target_properties(svan_sensor_echo_dds PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )

elseif(BUILD_ROS2)
    message(STATUS "Building ROS 2 examples...")

    add_executable(svan_sensor_echo_ros2 m2_sensor_echo_ros2.cpp)
    ament_target_dependencies(svan_sensor_echo_ros2 rclcpp std_msgs)

    # Inject the generated xterra message type-support library at link time.
    rosidl_target_interfaces(svan_sensor_echo_ros2 ${PROJECT_NAME} "rosidl_typesupport_cpp")

    # Install into the ament package library tree so `ros2 run` can locate it.
    install(TARGETS svan_sensor_echo_ros2
        DESTINATION lib/${PROJECT_NAME}
    )
endif()