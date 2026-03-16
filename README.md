# m2_sdk
# ORION V2.0 API Reference — Svan M2 Quadruped Platform

**Version:** 2.0.0 | **March 2026**

This directory contains the source for the Svan M2 API documentation, published at `docs.xterrarobotics.com/api`. The documentation covers the complete ORION V2.0 communication interface — from environment setup through real-time sensor reading and joint-level actuation.

---

## Directory Structure

```
api/
├── page.js                  # Overview, architecture, topic reference, appendices
├── quickstart/
│   └── page.js              # Sections 3–4: ROS 2 and native CycloneDDS setup
├── sensors/
│   └── page.js              # Section 5: Sensor Data Interface (SensorData msg)
├── robot-control/
│   └── page.js              # Sections 6–7: High-Level and Low-Level control
├── dhav/
│   └── page.js              # Legacy V1 (dhav platform) — kept for reference
└── README.md                # This file
```

---

## Page Summary

### `page.js` — ORION V2.0 Overview

Entry point for the API section. Covers:

- **Section 1:** System architecture and access layers (High-Level vs. Direct Joint Control)
- **Section 2:** Middleware communication pathways (ROS 2 vs. Native CycloneDDS)
- **Appendix A:** Topic quick reference — all three interfaces in a single table
- **Appendix B:** Communication requirements (domain ID, subnet, watchdog, `cyclonedds.xml`)
- **Appendix C:** Joint index mapping — all 12 joints (FR/FL/RR/RL × HAA/HFE/KFE)

### `quickstart/page.js` — Quickstart Guide

Step-by-step environment setup for both communication pathways.

**Section 3 — ROS 2:**
1. Install `rmw-cyclonedds-cpp`
2. Clone SDK into a `colcon` workspace and build
3. Source the overlay and configure `cyclonedds.xml` with the correct network interface
4. Validate with `ros2 topic echo /m2_metal/hw/sensor_data`

**Section 4 — Native CycloneDDS:**
1. Build CycloneDDS core and CXX bindings from source
2. Compile the SDK with CMake (`idlc` generates `.hpp` headers)
3. Configure `cyclonedds.xml`
4. Validate by running the `svan_sensor_echo` binary

### `sensors/page.js` — Sensor Data Interface

**Section 5.** The robot publishes `xterra_msgs/SensorData` at ~500 Hz.

| Group | Fields | Notes |
|---|---|---|
| Power & Diagnostics | `driver_fault`, `driver_voltage`, `driver_power`, `fet_temp_c` | Fault = 0 means nominal |
| Actuator Kinematics | `q[12]`, `dq[12]`, `ddq[12]`, `tau_est[12]`, `q_current[12]`, `d_current[12]` | All arrays index 0–11 per Appendix C |
| Body IMU | `quat[4]`, `gyro[3]`, `accel[3]`, `rpy[3]` | Quaternion [x, y, z, w] |

Topics:

| Middleware | Topic |
|---|---|
| ROS 2 | `/m2_metal/hw/sensor_data` |
| Native CycloneDDS | `rt/m2_metal/hw/sensor_data` |

Includes C++ subscriber examples for both pathways.

### `robot-control/page.js` — Control Interfaces

**Section 6 — High-Level Control Interface** (`xterra_msgs/JoyData`):

Commands the native locomotion controller via state transitions and velocity vectors. Must be published at **200 Hz** (mandatory — safety watchdog enforced).

`JoyData` message fields:

| Field | Type | Description |
|---|---|---|
| `priority` | `uint8` | Command priority (higher value wins) |
| `axes[6]` | `float32[6]` | Velocity/posture targets (axes 0, 1, 3, 4 are inverted) |
| `buttons[12]` | `uint8[12]` | State transition triggers |

Axes:

| Index | Description | Inversion |
|---|---|---|
| 0 | Lateral (left/right) | Inverted |
| 1 | Longitudinal (forward/backward) | Inverted (`-1.0` = max forward) |
| 3 | Yaw (rotation about Z) | Inverted |
| 4 | Body height | Inverted |
| 5 | Step height (0.0–1.0) | Normal |

Buttons / State Machine:

```
STATE SLEEP
     |
 [BUTTON 1]
     |
     v
STATE FIXED STAND  ← [BUTTON 0]   (guard: Roll < 1.5 & Pitch < 1.5)
     |         |
 [BUTTON 2]  [BUTTON 3]
     |         |
     v         v
STATE MOVE ↔ STATE FREESTAND
```

Topics:

| Middleware | Topic |
|---|---|
| ROS 2 | `/mission/joystick_data` |
| Native CycloneDDS | `rt/mission/joystick_data` |

---

**Section 7 — Low-Level Control Interface** (`xterra_msgs/JointData`):

Bypasses the native controller. The developer streams explicit actuator targets to all 12 joints simultaneously at ≥200 Hz. The onboard motor driver applies:

```
τ = τ_ff + k_p × (q_ref − q_act) + k_d × (dq_ref − dq_act)
```

`JointData` message fields:

| Field | Type | Units | Description |
|---|---|---|---|
| `q[12]` | float32[12] | rad | Target joint position |
| `dq[12]` | float32[12] | rad/s | Target joint velocity |
| `kp[12]` | float32[12] | N·m/rad | Stiffness gain |
| `kd[12]` | float32[12] | N·m·s/rad | Damping gain |
| `tau[12]` | float32[12] | N·m | Feed-forward torque |

Control mode shortcuts:
- **Pure torque:** set `kp = kd = 0`, populate only `tau`
- **Pure position:** set `tau = 0`, populate `q`, `kp`, `kd`
- **Impedance:** tune all five fields

Topics:

| Middleware | Topic |
|---|---|
| ROS 2 | `/m2_metal/hw/joint_command` |
| Native CycloneDDS | `rt/m2_metal/hw/joint_command` |

> **WARNING:** This interface disables ALL built-in locomotion, balancing, and safety controllers. Incorrect commands can cause hardware damage or dangerous robot behavior.

---

## Joint Index Mapping (Appendix C)

| Index | Leg | Joint | Positive Direction |
|---|---|---|---|
| 0 | Front-Right (FR) | Abduction (HAA) | Abduct outward |
| 1 | Front-Right (FR) | Hip (HFE) | Flex forward |
| 2 | Front-Right (FR) | Knee (KFE) | Extend |
| 3 | Front-Left (FL) | Abduction (HAA) | Abduct outward |
| 4 | Front-Left (FL) | Hip (HFE) | Flex forward |
| 5 | Front-Left (FL) | Knee (KFE) | Extend |
| 6 | Rear-Right (RR) | Abduction (HAA) | Abduct outward |
| 7 | Rear-Right (RR) | Hip (HFE) | Flex forward |
| 8 | Rear-Right (RR) | Knee (KFE) | Extend |
| 9 | Rear-Left (RL) | Abduction (HAA) | Abduct outward |
| 10 | Rear-Left (RL) | Hip (HFE) | Flex forward |
| 11 | Rear-Left (RL) | Knee (KFE) | Extend |

Ordering convention: **FR → FL → RR → RL**, each leg **[HAA, HFE, KFE]**.

---

## Communication Summary

| Parameter | Value | Notes |
|---|---|---|
| DDS Domain ID | 0 | Must match on robot and client |
| Network topology | Same subnet | Host and robot on same LAN segment |
| RMW (ROS 2) | `rmw_cyclonedds_cpp` | Mandatory |
| Command rate | ≥200 Hz | Prevents safety watchdog timeout |
| Watchdog timeout | ~15 ms | Zero-order hold on missed deadline |
| DDS config file | `cyclonedds.xml` | Must set `NetworkInterfaceAddress` |
