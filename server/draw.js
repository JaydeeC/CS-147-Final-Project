const canvas = document.getElementById("canvas");
const ctx = canvas.getContext("2d");

let drawing = false;
let currentColor = document.getElementById("colorPicker").value;
let erasing = false;

document.getElementById("colorPicker").addEventListener("input", (e) => {
  currentColor = e.target.value;
  erasing = false;
});

document.getElementById("erase").addEventListener("click", () => {
  erasing = true;
});

document.getElementById("draw").addEventListener("click", () => {
  erasing = false;
  currentColor = document.getElementById("colorPicker").value;
});

document.getElementById("clear").addEventListener("click", () => {
  ctx.fillStyle = "#000000";
  ctx.fillRect(0, 0, canvas.width, canvas.height);
});

function startDraw(e) {
  drawing = true;
  draw(e);
}

function stopDraw() {
  drawing = false;
  ctx.beginPath();
}

function getPos(e) {
  const rect = canvas.getBoundingClientRect();

  const clientX = e.clientX !== undefined ? e.clientX : e.touches[0].clientX;
  const clientY = e.clientY !== undefined ? e.clientY : e.touches[0].clientY;

  return {
    x: clientX - rect.left,
    y: clientY - rect.top
  };
}

const PIXEL_SIZE = 10; 

function draw(e) {
  if (!drawing) return;

  const pos = getPos(e);

  const gx = Math.floor(pos.x / PIXEL_SIZE) * PIXEL_SIZE;
  const gy = Math.floor(pos.y / PIXEL_SIZE) * PIXEL_SIZE;

  ctx.fillStyle = erasing ? "#000000" : currentColor;
  ctx.fillRect(gx, gy, PIXEL_SIZE, PIXEL_SIZE);
}

canvas.addEventListener("mousedown", startDraw);
canvas.addEventListener("mousemove", draw);
canvas.addEventListener("mouseup", stopDraw);
canvas.addEventListener("mouseleave", stopDraw);

canvas.addEventListener("touchstart", (e) => {
  e.preventDefault();
  startDraw(e);
});
canvas.addEventListener("touchmove", (e) => {
  e.preventDefault();
  draw(e);
});
canvas.addEventListener("touchend", stopDraw);
canvas.addEventListener("touchcancel", stopDraw);


document.getElementById("send").addEventListener("click", uploadDrawing);

function uploadDrawing() {
  const small = document.createElement("canvas");
  small.width = 64;
  small.height = 32;

  const sctx = small.getContext("2d");
  sctx.drawImage(canvas, 0, 0, 64, 32);

  const img = sctx.getImageData(0, 0, 64, 32);
  const pixels = [];


  for (let i = 0; i < img.data.length; i += 4) {
    pixels.push([img.data[i], img.data[i + 1], img.data[i + 2]]);
  }

  const json = {
    type: "drawing",
    width: 64,
    height: 32,
    pixels: pixels
  };

  const status = document.getElementById("status");
  status.textContent = "Uploading...";

 fetch("http://localhost:3000/upload", {
    method: "POST",
    headers: {"Content-Type": "application/json"},
    body: JSON.stringify(json)
  })
    .then(() => status.textContent = "Uploaded!")
    .catch(() => status.textContent = "Upload failed");
}
