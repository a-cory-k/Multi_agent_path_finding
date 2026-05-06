# Multi-Agent Pathfinding (MAPF) Simulation

<img width="800" height="509" alt="2026-05-0417 04 07-ezgif com-gif-maker" src="https://github.com/user-attachments/assets/7fcc4145-99fa-4945-9aa7-8153ece50bbe" />

## 1. Project Overview
This project is a high-performance simulation of an **autonomous warehouse environment** where multiple robots coordinate to pick up objects and deliver them to designated points. The system implements complex pathfinding algorithms and a decentralized coordination mechanism to ensure collision-free movement in a constrained grid environment.

---

## 2. Technical Implementation

###  Pathfinding Algorithms
The system supports two primary search algorithms which can be toggled in real-time:

*   **A-Star (A\*):** Utilizes Manhattan distance as a heuristic $h(n) = |x_1 - x_2| + |y_1 - y_2|$ to optimize the search space and find the shortest path efficiently.
*   **Dijkstra:** A special case of A* where the heuristic is set to zero, ensuring an exhaustive search of the shortest path.

###  Collision Avoidance (Space-Time Reservoir)
To prevent agents from colliding, the system implements a **3D Space-Time Occupancy Map**:

*   **Vertex Conflicts:** Prevents two robots from occupying the same $(x, y)$ coordinate at the same time $t$.
*   **Edge Conflicts:** Prevents "swapping" moves where two robots attempt to pass through each other in a single time step.
*   **Dynamic Reservation:** When a robot plans a path, it reserves its intended coordinates in a global `std::set<std::tuple<int, int, int>> reserved` to alert other agents.

###  Simulation Logic
*   **Multithreading:** The logic engine runs on a dedicated worker thread (`worker.cpp`), decoupled from the SFML rendering loop to ensure a consistent simulation delta time (200ms steps).
*   **State Machine:** Robots automatically cycle through states: searching for the nearest available object, navigating to the target, picking up the item, and delivering it to the owner's delivery point.

---

## 3. Results & Experiments

| Metric | Observation |
| :--- | :--- |
| **Path Optimality** | A* significantly reduces the number of evaluated nodes compared to Dijkstra while maintaining identical path lengths. |
| **Conflict Resolution** | The inclusion of a "wait" move (staying in current cell) allows robots to wait if all adjacent paths are reserved by higher-priority agents. |
| **Grid Scalability** | The system handles complex maps loaded from `.txt` files, automatically identifying walls (`#`), robots (`R`), and delivery zones. |

---

## 4. Setup & Requirements

*   **Language:** C++17 or higher.
*   **Library:** [SFML](https://www.sfml-dev.org/) (Simple and Fast Multimedia Library) for graphics and window management.
*   **Assets:** Requires a `maps/` directory containing `.txt` grid files and an `arial.ttf` font file.

###  How to use the UI:
1.  **Select Map:** Click on any loaded map name in the right panel to reset the environment.
2.  **Select Algorithm:** Click **"Dijkstra"** or **"A-Star"** to begin the simulation using that specific logic.
3.  **Visuals:** 
    *   🟨 **Yellow boxes** represent objects.
    *   🔵 **Colored circles** are robots.
    *   ⬜ **Outlined squares** are delivery points.






