# Emergency Evacuation Pathfinding

## Project Overview

This project provides a system to find the fastest evacuation routes during emergencies, considering traffic conditions on roads. It uses a C++ backend implementing Dijkstra’s algorithm to calculate the shortest travel time paths, and a Python Streamlit frontend for interactive visualization and user interaction.

The system supports two main modes:

* Find shortest path from a selected start location to a selected end location.
* Automatically find the shortest path from a start location to the nearest hospital.

**Note:** This project and the underlying data (roads, hospitals, coordinates) are specific to the city of **Dehradun, India**.

## Features

* Calculates evacuation paths with traffic-adjusted travel times.
* Displays hospital details like phone number, capacity, specialization, and emergency services.
* Visualizes routes on interactive maps using Folium and OpenRouteService.
* User-friendly web interface built with Streamlit.
* Uses CSV files for static data inputs including roads, hospitals, and coordinates specific to Dehradun.

## Technologies & Tools Used

* C++ for core graph construction and Dijkstra’s shortest path algorithm.
* Python with Streamlit for frontend user interface and map visualization.
* CSV files for static traffic, hospital, and coordinate data.
* OpenRouteService API and Folium for map and routing visualizations.

## File Structure

EmergencyEvacuationProject/<br>
├── evacuation.cpp # C++ backend implementing Dijkstra’s algorithm and CLI<br>
├── roads.csv # Road network with distance, base travel time, and traffic multipliers<br>
├── safezone.csv # Hospital/safe zone info: location, phone, capacity, etc.<br>
├── coordinate.csv # Geographic coordinates (latitude, longitude) for nodes<br>
├── app.py # Streamlit frontend app to interact with backend and visualize routes<br>
├── README.md # Project documentation<br>
└── requirements.txt # Python dependencies for frontend<br>

## Setup & Installation

### Prerequisites

* g++ compiler (supporting C++11 or above)
* Python 3.7+
* Python packages: streamlit, openrouteservice, folium, streamlit-folium

### Steps

1. Clone the repository:
   `git clone https://github.com/yourusername/EmergencyEvacuationProject.git`
   `cd EmergencyEvacuationProject`

2. Compile the C++ backend:
   `g++ evacuation.cpp -o evacuation`

3. Install Python dependencies:
   `pip install -r requirements.txt`

4. Run the Streamlit app:
   `streamlit run app.py`

## Usage

1. Open the Streamlit app in your browser.
2. Select one of the modes:

   * Mode 1: Find shortest path from start to end location.
   * Mode 2: Find shortest path from start location to nearest hospital.
3. Select the start location (and end location if Mode 1).
4. Click **Find Path** to display the route on the map along with hospital details and travel metrics.

## CSV File Descriptions

* **roads.csv:** Contains road edges with fields: Node1, Node2, Distance (km), Base Travel Time (minutes), Traffic Multiplier.
* **safezone.csv:** Contains hospital/safe zone details: Location, Phone, Capacity, Specialization, EmergencyServices, Type.
* **coordinate.csv:** Contains node names and their latitude and longitude coordinates.

*All data is focused on locations within Dehradun city limits.*

## GUI Screenshots

### Home Screen

![image](https://github.com/user-attachments/assets/6cd0c20d-c7c1-435a-8aef-bbd8c07f9ccf)

### Route Visualization

![image](https://github.com/user-attachments/assets/5adf952a-8d7a-472d-acd6-c8c2c7ef13da)

### Hospital Details

![image](https://github.com/user-attachments/assets/53064551-6f37-4406-96fb-50b749241eaf)

---
