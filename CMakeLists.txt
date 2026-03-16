cmake_minimum_required(VERSION 3.16)
project(xterra)

option(BUILD_ROS2     "Build ROS 2 communication interfaces (requires sourced ROS 2 workspace)" OFF)
option(BUILD_DDS      "Build standalone CycloneDDS interfaces (requires setup_dds_env.sh)"      OFF)
option(BUILD_EXAMPLES "Build subscriber/publisher validation examples"                           OFF)

if(BUILD_ROS2 AND BUILD_DDS)
    message(FATAL_ERROR
        "BUILD_ROS2 and BUILD_DDS are mutually exclusive.\n"
        "Set exactly one of -DBUILD_ROS2=ON or -DBUILD_DDS=ON.")
endif()


if(BUILD_DDS)
    message(STATUS "--- Configuring Svan M2 SDK: CycloneDDS Pathway ---")

    # CycloneDDS is installed locally by setup_dds_env.sh into third_party/install/.
    set(THIRD_PARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/install")

    if(NOT EXISTS "${THIRD_PARTY_DIR}")
        message(FATAL_ERROR
            "CycloneDDS dependencies not found at: ${THIRD_PARTY_DIR}\n"
            "Run './setup_dds_env.sh' from the repository root to download and build them.")
    endif()

    list(APPEND CMAKE_PREFIX_PATH ${THIRD_PARTY_DIR})

    find_package(CycloneDDS     REQUIRED)
    find_package(CycloneDDS-CXX REQUIRED)

    # Collect all IDL definitions from the idl/ directory.
    file(GLOB IDL_FILES 
        "idl/JoyData.idl"
        "idl/SensorData.idl"
        "idl/JointData.idl")
    if(NOT IDL_FILES)
        message(FATAL_ERROR "No .idl files found in the idl/ directory.")
    endif()

    # Invoke the CycloneDDS IDL compiler to generate C++ source and header files.
    idlcxx_generate(TARGET svan_m2_idl_gen FILES ${IDL_FILES})

    get_target_property(_idl_sources svan_m2_idl_gen INTERFACE_SOURCES)
    add_library(svan_m2_idl_lib STATIC ${_idl_sources})
    add_dependencies(svan_m2_idl_lib svan_m2_idl_gen)
    target_include_directories(svan_m2_idl_lib PUBLIC
        $<TARGET_PROPERTY:svan_m2_idl_gen,INTERFACE_INCLUDE_DIRECTORIES>)
    target_link_libraries(svan_m2_idl_lib PUBLIC CycloneDDS-CXX::ddscxx)

    if(BUILD_EXAMPLES)
        add_subdirectory(examples)
    endif()

elseif(BUILD_ROS2)
    message(STATUS "--- Configuring Svan M2 SDK: ROS 2 Pathway ---")

    find_package(ament_cmake              REQUIRED)
    find_package(rosidl_default_generators REQUIRED)
    find_package(std_msgs                 REQUIRED)

    # Register all message definitions for code generation.
    set(msg_files
        "msg/JoyData.msg"
        "msg/SensorData.msg"
        "msg/JointData.msg"
    )

    rosidl_generate_interfaces(${PROJECT_NAME}
        ${msg_files}
        DEPENDENCIES std_msgs
    )

    if(BUILD_EXAMPLES)
        find_package(rclcpp REQUIRED)
        add_subdirectory(examples)
    endif()

    ament_package()
endif()
