import streamlit as st
import subprocess
import openrouteservice
import folium
from streamlit_folium import folium_static


ORS_API_KEY = '5b3ce3597851110001cf624815f1f06b0b404d7fb6625aab81cbbf84'
client = openrouteservice.Client(key=ORS_API_KEY)

def run_cpp_backend(mode, start, end=None):
    cmd = ["./evacuation", str(mode), start]
    if mode == 1 and end:
        cmd.append(end)
    try:
        output = subprocess.check_output(cmd, universal_newlines=True)
        return output
    except Exception as e:
        return f"Error running backend: {e}"

def parse_output(output):
    lines = output.strip().splitlines()
    result = {"path": [], "distance": 0, "time": 0, "hospital": None, "error": None}
    if "No valid path found." in output or "No hospital path found." in output:
        result["error"] = "No valid path found."
        return result
    for line in lines:
        if line.startswith("Nearest Hospital:"):
            result["hospital"] = line.split(":",1)[1].strip()
        elif line.startswith("Path:"):
            parts = line.split("Path:",1)[1].strip()
            if parts:
                result["path"] = [p.strip() for p in parts.split("->")]
        elif line.startswith("Distance:"):
            result["distance"] = float(line.split(":",1)[1].strip())
        elif line.startswith("Total Time:"):
            result["time"] = float(line.split(":",1)[1].replace("minutes","").strip())
    return result

def get_coordinates_for_nodes(node_list, coord_file="coordinate.csv"):
    coords_map = {}
    with open(coord_file, "r") as f:
        next(f)  
        for line in f:
            parts = line.strip().split(",")
            if len(parts) >= 3:
                node = parts[0]
                lat = float(parts[1])
                lon = float(parts[2])
                coords_map[node] = (lat, lon)
    coords = []
    for node in node_list:
        coords.append(coords_map.get(node, (None, None)))
    return coords

def get_route_geojson(coords):
    coords_lonlat = [[lon, lat] for lat, lon in coords if lat is not None and lon is not None]
    if len(coords_lonlat) < 2:
        return None
    try:
        routes = client.directions(
            coordinates=coords_lonlat,
            profile='driving-car',
            format='geojson'
        )
        return routes
    except Exception as e:
        st.error(f"OpenRouteService error: {e}")
        return None

def get_detailed_route_coords(geojson):
    if not geojson or "features" not in geojson or len(geojson["features"]) == 0:
        return []
    coords_lonlat = geojson["features"][0]["geometry"]["coordinates"]
    # convert [lon, lat] → [lat, lon]
    coords_latlon = [[lat, lon] for lon, lat in coords_lonlat]
    return coords_latlon

def main():
    st.set_page_config(page_title="🚨 Emergency Evacuation Pathfinding", layout="wide")
    st.markdown("""
        <h1 style='text-align: center; font-size: 56px; font-weight: 900; color: #d63031; margin-bottom: 10px;'>🚨 Emergency Evacuation Pathfinding</h1>
        <hr style='border: 1px solid #d63031; margin-bottom: 30px;'>
    """, unsafe_allow_html=True)

    st.markdown("<div style='font-size: 24px; font-weight: 700; margin-bottom: 10px;'>🧭 Select Mode:</div>", unsafe_allow_html=True)
    mode = st.radio("", ("1 - Start to End Location", "2 - Start to Nearest Hospital"), horizontal=True)

    # Load nodes from coordinate.csv
    nodes = []
    try:
        with open("coordinate.csv") as f:
            next(f)
            for line in f:
                node_name = line.strip().split(",")[0]
                nodes.append(node_name)
    except FileNotFoundError:
        st.error("coordinate.csv not found! Make sure the file is present in the app folder.")
        return

    st.markdown("<div style='font-size: 22px; font-weight: 600; margin-top: 20px;'>🏁 Select Start Location</div>", unsafe_allow_html=True)
    start_node = st.selectbox("", nodes, index=0)

    end_node = None
    if mode.startswith("1"):
        st.markdown("<div style='font-size: 22px; font-weight: 600; margin-top: 20px;'>🏁 Select End Location</div>", unsafe_allow_html=True)
        end_node = st.selectbox("", nodes, index=min(1, len(nodes)-1))

    if st.button("Find Path", key="find_path"):
        with st.spinner("Calculating route..."):
            output = run_cpp_backend(1, start_node, end_node) if mode.startswith("1") else run_cpp_backend(2, start_node)
            st.code(output, language="plaintext")

            result = parse_output(output)
            if result["error"]:
                st.error(result["error"])
                return

            path = result["path"]
            distance = result["distance"]
            time = result["time"]
            hospital = result.get("hospital")

            if hospital:
                st.success(f"Nearest Hospital: {hospital}")

            st.markdown(f"<div style='font-size: 22px; font-weight: 600; margin-top: 10px;'>📏 <b>Distance:</b> {distance} km</div>", unsafe_allow_html=True)
            st.markdown(f"<div style='font-size: 22px; font-weight: 600;'>⏱ <b>Estimated Time:</b> {time} minutes</div>", unsafe_allow_html=True)

            coords = get_coordinates_for_nodes(path)
            if any(lat is None or lon is None for lat, lon in coords):
                st.warning("Warning: Some node coordinates missing; map might not render correctly.")

            # Center map on start node
            start_lat, start_lon = coords[0]
            m = folium.Map(location=[start_lat, start_lon], zoom_start=14)

            # Get detailed route geometry from ORS
            geojson = get_route_geojson(coords)
            if geojson:
                detailed_route = get_detailed_route_coords(geojson)
                if detailed_route:
                    folium.PolyLine(
                        detailed_route,
                        color="blue",
                        weight=5,
                        opacity=0.8
                    ).add_to(m)
        

            # Add markers for all nodes in path
            for i in range(len(coords)):
                lat, lon = coords[i]
                if lat is None or lon is None:
                    continue
                color = "green" if i == 0 else "red" if i == len(coords)-1 else "blue"
                if i == 0:
                    label = f"Start: {path[i]}"
                elif i == len(coords) - 1:
                    label = f"End: {path[i]}"
                else:
                    label = path[i]

                folium.Marker(
                    location=[lat, lon],
                    popup=label,
                    icon=folium.Icon(color=color, icon="info-sign")
                ).add_to(m)

            folium_static(m, width=900, height=600)

if __name__ == "__main__":
    main()