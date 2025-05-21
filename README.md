# 🚨 Emergency Evacuation Pathfinding

This project provides a robust emergency evacuation pathfinding system that computes the shortest and safest path from any location to a designated destination or the nearest safe zone (e.g., hospital or shelter) considering traffic conditions and road risks.

## 🔍 Project Overview

During emergencies such as natural disasters, fires, or road blockages, rapid and safe evacuation is crucial. This project uses a **C++ backend** to compute optimal evacuation routes based on traffic-adjusted time using **Dijkstra's algorithm**, and a **Python Streamlit frontend** for interactive map visualization.

### Key Features
- Shortest path calculation based on **real traffic data**.
- Evacuation routing to a specified destination or **nearest safe zone**.
- Map-based visualization using **Streamlit** and **OpenRouteService**.
- Supports both manual and automatic location detection.

## 📁 File Structure

```
emergency-evacuation/
├── evacuation.cpp              # C++ backend for Dijkstra's algorithm
├── roads.csv             # Road network with traffic data
├── coordinate.csv       # Coordinates for locations
├──safezone.csv         # List of hospitals/shelters with coordinates
├──app.py                # Streamlit frontend
├── requirements.txt          # Python dependencies
└── README.md                 # Project documentation (this file)
```

## ⚙️ Backend (C++11)

### Functionality

- **Mode 1**: Find the shortest path from **source** to **destination**.
- **Mode 2**: Automatically find and route to the **nearest hospital/shelter** from a given source.

### Compile and Run

```bash
g++ -std=c++11 main.cpp -o evacuation
./evacuation mode start [destination]
```

#### Examples:

```bash
./evacuation 1 ClockTower ISBT
./evacuation 2 RajpurRoad
```

## 🌍 Frontend (Streamlit)

### Features
- Interactive map showing path with risk levels (color-coded).
- User input for source/destination or auto-location.
- Dynamic route display with time and distance info.

### Run the App

```bash
cd frontend
streamlit run app.py
```

## 📊 CSV File Format

### `roads.csv`

```
source,destination,distance_km,time_min,traffic_level
ClockTower,ISBT,5.0,10,2
ISBT,SubhashNagar,3.0,7,1
...
```

### `coordinates.csv`

```
location,latitude,longitude
ClockTower,30.3256,78.0437
ISBT,30.3079,78.0483
...
```

### `safezones.csv`

```
name,latitude,longitude,type
MaxHospital,30.3182,78.0535,hospital
Shelter1,30.3290,78.0421,shelter
...
```

## ✅ Requirements

Install Python dependencies using:

```bash
pip install -r requirements.txt
```

### `requirements.txt`

```
streamlit
folium
openrouteservice
pandas
```

## 🧠 Algorithm Used

This project uses a **modified Dijkstra's Algorithm** that prioritizes:

- Adjusted travel time based on traffic level.
- Shortest safe path to reduce risk during emergency conditions.

## 📌 Future Improvements

- Live traffic updates via API.
- Real-time user location from mobile devices.
- Multi-path options with comparative analysis.
- Risk-level-based edge coloring on maps.

## 📚 Credits

Developed as part of a **Design and Analysis of Algorithms** project.

- C++ Backend: [Your Name]
- Python + Streamlit Frontend: [Your Name or Collaborators]
