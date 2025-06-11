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
    result = {"paths": [], "distance": 0, "time": 0, "hospital": None, "error": None}

    if "No valid path found." in output or "No hospital path found." in output:
        result["error"] = "No valid path found."
        return result

    for i, line in enumerate(lines):
        line = line.strip()
        if line.lower().startswith("fastest evacuation time"):
            try:
                time_str = line.split(":", 1)[1].strip().replace("minutes", "").strip()
                result["time"] = float(time_str)
            except:
                pass
        elif line.lower().startswith("shortest distance"):
            try:
                dist_str = line.split(":", 1)[1].strip().replace("km", "").strip()
                result["distance"] = float(dist_str)
            except:
                pass
        elif line.lower().startswith("nearest hospital"):
            result["hospital"] = line.split(":", 1)[1].strip()
        elif line.lower().startswith("all optimal paths"):
            j = i + 1
            paths = []
            while j < len(lines) and lines[j].strip():
                path = [node.strip() for node in lines[j].split("->")]
                paths.append(path)
                j += 1
            result["paths"] = paths
        elif line.lower().startswith("path:"):
            path_line = line.split(":", 1)[1].strip()
            if path_line:
                path = [node.strip() for node in path_line.split("->")]
                result["paths"] = [path]

    return result

def load_nodes():
    nodes = []
    try:
        with open("coordinate.csv") as f:
            next(f)
            for line in f:
                node_name = line.strip().split(",")[0]
                nodes.append(node_name)
    except FileNotFoundError:
        st.error("coordinate.csv not found! Make sure the file is present.")
    return nodes

def load_coordinates():
    coords_map = {}
    try:
        with open("coordinate.csv", "r") as f:
            next(f)
            for line in f:
                parts = line.strip().split(",")
                if len(parts) >= 3:
                    coords_map[parts[0]] = (float(parts[1]), float(parts[2]))
    except FileNotFoundError:
        st.error("coordinate.csv not found!")
    return coords_map

def load_hospitals():
    hospital_info = {}
    try:
        with open("safezone.csv", "r") as f:
            header = next(f).strip().split(",")
            for line in f:
                parts = line.strip().split(",")
                if len(parts) >= len(header):
                    data = dict(zip(header, parts))
                    hospital_info[data["Location"].strip()] = data
    except FileNotFoundError:
        st.warning("safezone.csv not found.")
    return hospital_info

def plot_route_on_map(paths, coords_map, hospital=None):
    colors = ["blue", "green", "red", "purple", "orange", "darkred", "lightblue", "cadetblue", "pink", "lightgreen"]
    color_idx = 0

    if hospital and hospital in coords_map:
        center = coords_map[hospital]
    elif paths and paths[0] and paths[0][0] in coords_map:
        center = coords_map[paths[0][0]]
    else:
        center = (30.3165, 78.0322)

    m = folium.Map(location=center, zoom_start=14)

    for path in paths:
        coords = []
        missing_coords = False
        for node in path:
            if node in coords_map:
                coords.append(coords_map[node])
            else:
                missing_coords = True
                st.warning(f"Missing coordinates for node '{node}'")
        if missing_coords or len(coords) < 1:
            continue

        try:
            coords_lonlat = [[lon, lat] for lat, lon in coords]
            geojson = client.directions(
                coordinates=coords_lonlat,
                profile='driving-car',
                format='geojson'
            )
            detailed_route = geojson["features"][0]["geometry"]["coordinates"]
            detailed_route_latlon = [[lat, lon] for lon, lat in detailed_route]
        except Exception as e:
            st.error(f"OpenRouteService error: {e}")
            detailed_route_latlon = coords

        folium.PolyLine(
            detailed_route_latlon,
            color=colors[color_idx % len(colors)],
            weight=5,
            opacity=0.8,
            tooltip=" -> ".join(path)
        ).add_to(m)

        folium.Marker(
            location=coords[0],
            popup=f"Start: {path[0]}",
            icon=folium.Icon(color="green", icon="play")
        ).add_to(m)
        folium.Marker(
            location=coords[-1],
            popup=f"End: {path[-1]}",
            icon=folium.Icon(color="red", icon="stop")
        ).add_to(m)

        color_idx += 1

    if hospital and hospital in coords_map:
        folium.Marker(
            location=coords_map[hospital],
            popup=f"Hospital: {hospital}",
            icon=folium.Icon(color="red", icon="plus-sign")
        ).add_to(m)

    folium_static(m, width=900, height=600)

def main():
    st.set_page_config(page_title="🚨 Emergency Evacuation Pathfinding", layout="wide")
    st.markdown("""
        <h1 style='text-align: center; font-size: 56px; font-weight: 900; color: #d63031;'>🚨 Emergency Evacuation Pathfinding</h1>
        <hr style='border: 1px solid #d63031; margin-bottom: 30px;'>
    """, unsafe_allow_html=True)

    st.markdown("<div style='font-size: 24px; font-weight: 700;'>🧭 Select Mode:</div>", unsafe_allow_html=True)
    mode_str = st.radio("", ("1 - Start to End Location", "2 - Start to Nearest Hospital"), horizontal=True)
    mode = 1 if mode_str.startswith("1") else 2

    nodes = load_nodes()
    if not nodes:
        st.stop()

    st.markdown("<div style='font-size: 22px; font-weight: 600;'>🏁 Select Start Location</div>", unsafe_allow_html=True)
    start_node = st.selectbox("", nodes, index=0)

    end_node = None
    if mode == 1:
        st.markdown("<div style='font-size: 22px; font-weight: 600;'>🏁 Select End Location</div>", unsafe_allow_html=True)
        end_node = st.selectbox("", nodes, index=min(1, len(nodes)-1))

    hospital_info = load_hospitals()
    coords_map = load_coordinates()
    if not coords_map:
        st.stop()

    if st.button("Find Path", key="find_path"):
        with st.spinner("Calculating route..."):
            output = run_cpp_backend(mode, start_node, end_node)
            st.code(output, language="plaintext")

            result = parse_output(output)
            if result["error"]:
                st.error(result["error"])
                return

            paths = result["paths"]
            distance = result["distance"]
            time = result["time"]
            hospital = result.get("hospital")

            if hospital:
                info = hospital_info.get(hospital)
                st.success(f"🏥 Nearest Hospital: {hospital}")
                if info:
                    st.markdown(f"""
                        📞 **Phone:** {info.get('Phone', 'N/A')}  
                        🏥 **Capacity:** {info.get('Capacity', 'N/A')}  
                        🩺 **Specialization:** {info.get('Specialization', 'N/A')}  
                        🚨 **Emergency Services:** {info.get('EmergencyServices', 'N/A')}  
                        🏷️ **Type:** {info.get('Type', 'N/A')}
                    """)
                else:
                    st.info("Hospital info not available.")

                if distance > 0 and time > 0:
                    st.markdown(f"<div style='font-size: 22px; font-weight: 600;'>📏 Distance: {distance} km</div>", unsafe_allow_html=True)
                    st.markdown(f"<div style='font-size: 22px; font-weight: 600;'>⏱ Estimated Time: {time} minutes</div>", unsafe_allow_html=True)
                
            else:
                st.markdown(f"<div style='font-size: 22px; font-weight: 600;'>📏 Distance: {distance} km</div>", unsafe_allow_html=True)
                st.markdown(f"<div style='font-size: 22px; font-weight: 600;'>⏱ Estimated Time: {time} minutes</div>", unsafe_allow_html=True)

            if not paths or len(paths[0]) == 0:
                st.warning("No path found or missing coordinates to display map.")
                return

            plot_route_on_map(paths, coords_map, hospital=hospital)

if __name__ == "__main__":
    main()
