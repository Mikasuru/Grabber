<div align="center">
  <h1>Kukuri Malware Project</h1>
  <p>
    <img src="https://img.shields.io/badge/Purpose-Educational-blue?style=for-the-badge"/>
    <img src="https://img.shields.io/badge/Language-C++-00599C?style=for-the-badge&logo=cplusplus"/>
    <img src="https://img.shields.io/badge/Environment-Windows-0078D6?style=for-the-badge&logo=windows"/>
  </p>
</div>

<div align="center">
  ⚠️ <strong>FOR EDUCATIONAL PURPOSES ONLY</strong> ⚠️<br>
  This project demonstrates malware behavior for science education. <br />The code might not perfect.
</div>

## 🎯 Project Goals

<table>
  <tr>
    <td>🔍 <b>Understanding</b></td>
    <td>Learn malware persistence mechanisms and Windows internals</td>
  </tr>
  <tr>
    <td>🌐 <b>Networking</b></td>
    <td>Study communication patterns and C2 infrastructure</td>
  </tr>
  <tr>
    <td>🛡️ <b>Defense</b></td>
    <td>Practice cybersecurity defense techniques</td>
  </tr>
</table>

## ⚙️ Features

- **Process Control**
  - Remote command execution
  - Process manipulation (crash/freeze)
  - System monitoring

- **System Access**
  - Network connection control
  - Volume manipulation
  - Screen capture
  - File system operations

## 🔒 Security Controls

- No data exfiltration
- No encryption features
- No propagation mechanisms

## 💻 Requirements

```
- Windows 10/11
- Visual Studio 2022
- C++20
- Administrator privileges
```

## 🚀 Installation

1. Clone repository
2. Open `Kukuri Helper.sln` in Visual Studio
3. Build solution in Release mode
4. Run as Administrator
5. Open `Server` folder
6. Run `bun Main.js`

## ⚡ Quick Start

```cpp
// Setup command & control
const string webhookUrl = "your-webhook-url";
Payload::SendMessage(webhookUrl, "Test message");

// Create embedded message
Payload::Embed embed;
embed.title = "Test Embed";
embed.description = "Embedded message";
Payload::SendEmbed(webhookUrl, embed);
```

## 🔧 Removal Tool

A removal script is provided to clean the system:
- Terminates malware processes
- Removes startup entries
- Cleans registry modifications
- Deletes temporary files
- Restores system settings

## ⚠️ Disclaimer

This code is published for educational purposes as part of a controlled classroom environment. Any use outside of authorized educational contexts is strictly prohibited.

<div align="center">
  <p>Made with ❤️ | By Mikasuru</p>
</div>
