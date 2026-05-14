const { ipcRenderer } = require('electron');

const btnHash = document.getElementById('btn-hash');
const inputText = document.getElementById('input-text');
const outputText = document.getElementById('output-text');
const outputEncoding = document.getElementById('output-encoding');

btnHash.addEventListener('click', async () => {
    const text = inputText.value;
    const encoding = outputEncoding.value;
    
    // Gửi yêu cầu xuống file main.js (nơi chứa lõi C++)
    const result = await ipcRenderer.invoke('hash-data', text, encoding);
    
    // Hiển thị kết quả lên màn hình
    outputText.value = result;
});