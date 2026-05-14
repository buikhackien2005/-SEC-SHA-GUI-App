const { app, BrowserWindow, ipcMain } = require('electron');
const path = require('path');

// Nạp lõi C++ siêu tốc của bạn vào đây!
const addon = require('../build/Release/sha512_addon.node');

function createWindow() {
    const win = new BrowserWindow({
        width: 1000,
        height: 700,
        autoHideMenuBar: true, // Giấu thanh menu mặc định cho đẹp
        webPreferences: {
            nodeIntegration: true,
            contextIsolation: false // Bật tính năng gọi Node.js trực tiếp từ giao diện
        }
    });

    win.loadFile('ui/index.html');
}

app.whenReady().then(createWindow);

ipcMain.handle('hash-data', async (event, inputData, outputEncoding, isFile, algo) => {
    try {
        // C++ giờ nhận thêm tham số algo và một hàm callback bắt % tiến độ
        const hexResult = await addon.hash(inputData, isFile, algo, (percentage) => {
            // Gửi % từ main process bắn ngược ra renderer process (giao diện)
            event.sender.send('hash-progress', percentage);
        });

        if (outputEncoding === 'hex_upper') return hexResult.toUpperCase();
        if (outputEncoding === 'base64') return Buffer.from(hexResult, 'hex').toString('base64');
        return hexResult; 
    } catch (err) {
        return "Lỗi nội bộ: " + err.message;
    }
});

app.on('window-all-closed', () => {
    if (process.platform !== 'darwin') app.quit();
});