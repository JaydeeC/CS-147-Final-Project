from flask import Flask, request, jsonify
from flask_cors import CORS
import requests

app = Flask(__name__)
CORS(app)

IOT_HUB = "cs147group69iothub.azure-devices.net"
DEVICE_ID = "147esp32group69"

SAS_TOKEN = "SharedAccessSignature sr=cs147group69iothub.azure-devices.net%2Fdevices%2F147esp32group69&sig=VbWA9jEaUjlYbULr2N%2FbT4E1RzJzUu5FEMJlmxHZV7c%3D&se=60001764798811"

@app.post("/upload")
def upload():
    drawing = request.json

    url = f"https://{IOT_HUB}/devices/{DEVICE_ID}/messages/events?api-version=2021-04-12"

    headers = {
        "Authorization": SAS_TOKEN,
        "Content-Type": "application/json"
    }

    r = requests.post(url, json=drawing, headers=headers)

    if r.status_code == 204:
        return jsonify({"status": "ok"}), 200
    else:
        return jsonify({"status": "error", "code": r.status_code}), 500

app.run(port=3000)
