# 智能家居 MQTT 控制面板

这是一个基于 [EMQX 的 Vue3 MQTT 示例](https://github.com/emqx/MQTT-Client-Examples/tree/master/mqtt-client-Vue3.js) 改进的智能家居控制面板项目。项目采用 Vue 3 + TypeScript + Element Plus 技术栈，通过 MQTT 协议实现与智能家居设备的实时通信和控制。

## 项目特性

- 🔌 实时设备连接状态显示
- 💡 智能照明控制（支持多房间灯光管理）
- 🚨 警报系统控制
- 📁 文件管理系统（支持上传/下载/删除）
- 📊 设备状态实时反馈
- 🎨 美观的 Element Plus UI 界面
- 📱 响应式设计，支持移动端

## 在线演示

访问 [在线演示](https://zhangwei43721.github.io/Smart-Home-MQTT-Control/) 查看效果

## 技术栈

- Vue 3 + TypeScript
- Element Plus UI 组件库
- MQTT.js 客户端
- Vite 构建工具
- GitHub Actions 自动部署

## 本地开发

### 环境要求

- Node.js 16.0 或更高版本
- npm 包管理器

### 安装依赖

```bash
npm install
```

### 开发模式

```bash
npm run dev
```

### 构建生产版本

```bash
npm run build
```

## MQTT 服务器配置

默认连接配置：
- WebSocket 地址：ws://test.io.com:8083/mqtt
- 用户名：test
- 密码：123456

> 注意：这是测试服务器，生产环境请使用自己的 MQTT 服务器。

## 主要功能模块

1. **设备连接管理**
   - 支持 ws/wss 协议
   - 实时连接状态显示
   - 自定义连接参数配置

2. **照明控制**
   - 支持多个房间的灯光控制
   - 一键全开/全关功能
   - 实时状态反馈

3. **警报系统**
   - 一键开启/关闭警报
   - 实时状态监控

4. **文件管理**
   - 支持文件上传/下载
   - 支持文件删除
   - 支持文件列表刷新
   - 文件传输进度显示

5. **消息日志**
   - 实时显示 MQTT 消息
   - 支持日志清空

## 部署说明

项目使用 GitHub Actions 自动部署到 GitHub Pages：
1. 推送代码到 main 分支会自动触发部署
2. 构建产物会发布到 gh-pages 分支
3. 可通过 GitHub Pages 访问应用

## 贡献指南

1. Fork 本仓库
2. 创建特性分支 (`git checkout -b feature/AmazingFeature`)
3. 提交改动 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 提交 Pull Request

## 致谢

- [EMQX](https://github.com/emqx) - 原始 Vue3 MQTT 示例项目
- [Element Plus](https://element-plus.org/) - UI 组件库
- [MQTT.js](https://github.com/mqttjs/MQTT.js) - MQTT 客户端库
