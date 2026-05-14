const { ipcRenderer } = require('electron');

const btnHash = document.getElementById('btn-hash');
const outputText = document.getElementById('output-text');
const outputEncoding = document.getElementById('output-encoding');
const inputMode = document.getElementById('input-mode');
const textContainer = document.getElementById('input-text');
const fileContainer = document.getElementById('input-file-container');
const fileSelector = document.getElementById('file-selector');
const filePathDisplay = document.getElementById('file-path-display');

// DOM cho Progress Bar
const progressContainer = document.getElementById('progress-container');
const progressBar = document.getElementById('progress-bar');
const progressText = document.getElementById('progress-text');

let currentFilePath = "";
let currentAlgo = "sha512"; // Thuật toán mặc định

// Lắng nghe chọn Sidebar Thuật Toán
document.querySelectorAll('.algo-list li').forEach(li => {
    li.addEventListener('click', (e) => {
        document.querySelectorAll('.algo-list li').forEach(el => el.classList.remove('active'));
        e.target.classList.add('active');
        currentAlgo = e.target.getAttribute('data-algo');
    });
});

inputMode.addEventListener('change', (e) => {
    if (e.target.value === 'file') {
        textContainer.style.display = 'none';
        fileContainer.style.display = 'flex';
    } else {
        textContainer.style.display = 'block';
        fileContainer.style.display = 'none';
    }
});

fileSelector.addEventListener('change', (e) => {
    if (e.target.files.length > 0) {
        currentFilePath = e.target.files[0].path; 
        filePathDisplay.innerText = "Selected: " + currentFilePath;
    } else {
        currentFilePath = "";
        filePathDisplay.innerText = "";
    }
});

// Lắng nghe sự kiện % tiến độ gửi từ Main Process
ipcRenderer.on('hash-progress', (event, percentage) => {
    const pct = percentage.toFixed(1);
    progressBar.style.width = pct + '%';
    progressText.innerText = pct + '%';
});

btnHash.addEventListener('click', async () => {
    const isFileMode = (inputMode.value === 'file');
    const encoding = outputEncoding.value;
    let dataToSend = "";

    if (isFileMode) {
        if (!currentFilePath) {
            outputText.value = "Vui lòng chọn một file trước!";
            return;
        }
        dataToSend = currentFilePath;
    } else {
        dataToSend = textContainer.value;
    }

    // Hiển thị Progress Bar và Reset
    progressContainer.style.display = 'block';
    progressBar.style.width = '0%';
    progressText.innerText = '0%';
    outputText.value = "";
    btnHash.disabled = true;

    try {
        // Truyền currentAlgo vào
        const result = await ipcRenderer.invoke('hash-data', dataToSend, encoding, isFileMode, currentAlgo);
        outputText.value = result;
    } catch (err) {
        outputText.value = "Lỗi: " + err;
    } finally {
        btnHash.disabled = false;
        setTimeout(() => { progressContainer.style.display = 'none'; }, 2000); // Ẩn bar sau 2s
    }
});