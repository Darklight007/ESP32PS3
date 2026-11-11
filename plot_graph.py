#!/usr/bin/env python3
"""Power Supply Graph Viewer.
Load CSV files dumped via: pio device monitor | tee graph_dump.csv
Then type DATA:GRAPH? and Ctrl+C.
"""

import sys
import threading
import webbrowser
from http.server import HTTPServer, BaseHTTPRequestHandler

PORT = 8787

HTML_PAGE = """<!DOCTYPE html>
<html><head>
<title>Power Supply Graph Viewer</title>
<script src="https://cdn.plot.ly/plotly-2.35.0.min.js"></script>
<style>
body { font-family: sans-serif; background: #1e1e1e; color: #ccc; margin: 0; padding: 15px; }
h2 { margin: 0 0 8px 0; display: inline; }
#toolbar { margin-bottom: 8px; display: flex; align-items: center; gap: 15px; }
button { background: #4fc3f7; color: #1e1e1e; border: none; padding: 8px 18px;
         border-radius: 4px; cursor: pointer; font-size: 14px; font-weight: bold; }
button:hover { background: #81d4fa; }
#status { color: #888; font-size: 13px; }
#chart { width: 100%; height: calc(100vh - 70px); }
</style>
</head><body>
<div id="toolbar">
    <h2>Power Supply Graph</h2>
    <input type="file" id="fileInput" accept=".csv" style="display:none" onchange="loadFromFile(this)">
    <button onclick="document.getElementById('fileInput').click()">Load CSV File</button>
    <span id="status">Load a CSV file to plot</span>
</div>
<div id="chart"></div>
<script>
function plotData(idx, voltage, current) {
    const traceV = {
        x: idx, y: voltage, name: 'Voltage (V)',
        type: 'scatter', mode: 'lines+markers',
        line: { color: '#4fc3f7', width: 1.2 },
        marker: { color: '#4fc3f7', size: 3 },
        yaxis: 'y1'
    };
    const traceI = {
        x: idx, y: current, name: 'Current (mA)',
        type: 'scatter', mode: 'lines+markers',
        line: { color: '#ffb74d', width: 1.2 },
        marker: { color: '#ffb74d', size: 3 },
        yaxis: 'y2'
    };
    const layout = {
        paper_bgcolor: '#1e1e1e', plot_bgcolor: '#2d2d2d',
        font: { color: '#ccc' },
        legend: { x: 0.01, y: 0.99, bgcolor: 'rgba(0,0,0,0.5)' },
        xaxis: {
            title: 'Sample', gridcolor: 'rgba(255,255,255,0.08)',
            rangeslider: { visible: true, bgcolor: '#1e1e1e', thickness: 0.08 }
        },
        yaxis: {
            title: 'Voltage (V)', titlefont: { color: '#4fc3f7' },
            tickfont: { color: '#4fc3f7' }, gridcolor: 'rgba(79,195,247,0.15)', side: 'left'
        },
        yaxis2: {
            title: 'Current (mA)', titlefont: { color: '#ffb74d' },
            tickfont: { color: '#ffb74d' }, overlaying: 'y', side: 'right',
            gridcolor: 'rgba(255,183,77,0.08)'
        },
        margin: { l: 60, r: 60, t: 10, b: 40 },
        hovermode: 'x unified'
    };
    const config = { scrollZoom: true, displayModeBar: true, responsive: true };
    Plotly.newPlot('chart', [traceV, traceI], layout, config);
}

function loadFromFile(input) {
    const file = input.files[0];
    if (!file) return;
    const reader = new FileReader();
    reader.onload = function(e) {
        const lines = e.target.result.trim().split('\\n');
        const idx = [], voltage = [], current = [];
        for (const line of lines) {
            const parts = line.split(',');
            if (parts.length >= 3) {
                const i = parseInt(parts[0]), v = parseFloat(parts[1]), c = parseFloat(parts[2]);
                if (!isNaN(i) && !isNaN(v) && !isNaN(c)) {
                    idx.push(i);
                    voltage.push(v / 1000.0);
                    current.push(c);
                }
            }
        }
        plotData(idx, voltage, current);
        document.getElementById('status').textContent =
            idx.length + ' points loaded from ' + file.name;
    };
    reader.readAsText(file);
    input.value = '';
}
</script>
</body></html>"""


class Handler(BaseHTTPRequestHandler):
    def do_GET(self):
        self.send_response(200)
        self.send_header("Content-Type", "text/html")
        self.end_headers()
        self.wfile.write(HTML_PAGE.encode())

    def log_message(self, format, *args):
        pass


if __name__ == "__main__":
    server = HTTPServer(("127.0.0.1", PORT), Handler)
    url = f"http://127.0.0.1:{PORT}"
    print(f"Graph viewer at {url}")
    print("To dump data: pio device monitor | tee graph_dump.csv")
    print("  Then type DATA:GRAPH? and Ctrl+C when done.")
    print("Press Ctrl+C to stop viewer")
    threading.Timer(0.5, lambda: webbrowser.open(url)).start()
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\nStopped")
        server.server_close()
