<template>
  <div class="mqtt-demo">
    <!-- 顶部状态栏 -->
    <div class="status-bar" :class="{ connected: isConnected }">
      <div class="status-indicator">
        <span class="status-dot" :class="{ active: isConnected }"></span>
        <span class="status-text">{{ isConnected ? "已连接" : "未连接" }}</span>
      </div>
      <div class="connection-info" v-if="isConnected">
        <el-icon>
          <Connection />
        </el-icon>
        <span class="connection-text">{{ connection.host }}:{{ connection.port }}</span>
      </div>
    </div>

    <!-- 连接配置卡片（可折叠） -->
    <el-card class="config-card" shadow="never">
      <template #header>
        <div class="card-header clickable" @click="configCollapsed = !configCollapsed">
          <div class="header-left">
            <el-icon class="header-icon">
              <Setting />
            </el-icon>
            <span class="card-title">连接配置</span>
          </div>
          <el-icon class="collapse-icon" :class="{ collapsed: configCollapsed }">
            <ArrowDown />
          </el-icon>
        </div>
      </template>
      <el-collapse-transition>
        <div v-show="!configCollapsed">
          <el-form label-position="top" :model="connection">
            <el-row :gutter="16">
              <el-col :xs="24" :sm="12" :md="8">
                <el-form-item label="协议">
                  <el-select v-model="connection.protocol" :disabled="isConnected">
                    <el-option label="ws://" value="ws"></el-option>
                    <el-option label="wss://" value="wss"></el-option>
                  </el-select>
                </el-form-item>
              </el-col>
              <el-col :xs="24" :sm="12" :md="8">
                <el-form-item label="主机地址">
                  <el-input v-model="connection.host" :disabled="isConnected"></el-input>
                </el-form-item>
              </el-col>
              <el-col :xs="24" :sm="12" :md="8">
                <el-form-item label="端口">
                  <el-input v-model.number="connection.port" type="number" :disabled="isConnected"></el-input>
                </el-form-item>
              </el-col>
              <el-col :xs="24" :sm="12" :md="8">
                <el-form-item label="客户端 ID">
                  <el-input v-model="connection.clientId" :disabled="isConnected"></el-input>
                </el-form-item>
              </el-col>
              <el-col :xs="24" :sm="12" :md="8">
                <el-form-item label="用户名">
                  <el-input v-model="connection.username" :disabled="isConnected"></el-input>
                </el-form-item>
              </el-col>
              <el-col :xs="24" :sm="12" :md="8">
                <el-form-item label="密码">
                  <el-input v-model="connection.password" type="password" show-password
                    :disabled="isConnected"></el-input>
                </el-form-item>
              </el-col>
              <el-col :span="24">
                <div class="button-group">
                  <el-button type="primary" size="large" :disabled="isConnected" @click="createConnection"
                    :loading="btnLoadingType === 'connect'" color="#1976d2">
                    <el-icon>
                      <Connection />
                    </el-icon>
                    <span>{{ isConnected ? "已连接" : "连接" }}</span>
                  </el-button>
                  <el-button v-if="isConnected" size="large" @click="destroyConnection"
                    :loading="btnLoadingType === 'disconnect'" color="#424242">
                    <el-icon>
                      <Close />
                    </el-icon>
                    <span>断开</span>
                  </el-button>
                </div>
              </el-col>
            </el-row>
          </el-form>
        </div>
      </el-collapse-transition>
    </el-card>

    <!-- 灯光控制卡片 -->
    <el-card class="light-control-card" shadow="never">
      <template #header>
        <div class="card-header">
          <div class="header-left">
            <el-icon class="header-icon">
              <Sunny />
            </el-icon>
            <span class="card-title">灯光控制</span>
          </div>
          <div class="header-actions">
            <el-button size="small" @click="toggleAllLights(true)" :disabled="!isConnected" text>
              全开
            </el-button>
            <el-button size="small" @click="toggleAllLights(false)" :disabled="!isConnected" text>
              全关
            </el-button>
            <el-button size="small" @click="queryLightStatus" :disabled="!isConnected" text>
              刷新
            </el-button>
          </div>
        </div>
      </template>
      <div class="lights-grid">
        <div v-for="light in lights" :key="light.id" class="light-card"
          :class="{ active: light.state, disabled: !isConnected }" @click="toggleLight(light)">
          <div class="light-icon">{{ light.icon }}</div>
          <div class="light-info">
            <div class="light-name">{{ light.name }}</div>
            <div class="light-status">{{ light.state ? "开启" : "关闭" }}</div>
          </div>
          <el-switch v-model="light.state" size="large" :disabled="!isConnected" @click.stop
            @change="handleSingleLightControl(light)" />
        </div>
      </div>
    </el-card>

    <!-- 警报控制卡片 -->
    <el-card class="alarm-card" shadow="never">
      <template #header>
        <div class="card-header">
          <div class="header-left">
            <el-icon class="header-icon">
              <Bell />
            </el-icon>
            <span class="card-title">警报控制</span>
          </div>
          <div class="header-actions">
            <el-button size="small" @click="queryAlarmStatus" :disabled="!isConnected" text>
              刷新
            </el-button>
          </div>
        </div>
      </template>
      <div class="alarm-control" :class="{ active: alarmControl.state, disabled: !isConnected }" @click="toggleAlarm">
        <div class="alarm-icon" :class="{ active: alarmControl.state }">🔔</div>
        <div class="alarm-info">
          <div class="alarm-title">警报系统</div>
          <div class="alarm-status">{{ alarmControl.state ? "开启" : "关闭" }}</div>
        </div>
        <el-switch v-model="alarmControl.state" size="large" :disabled="!isConnected" @click.stop
          @change="handleAlarmControl" />
      </div>
    </el-card>

    <!-- 文件管理卡片 -->
    <el-card class="file-management-card" shadow="never">
      <template #header>
        <div class="card-header">
          <div class="header-left">
            <el-icon class="header-icon">
              <Folder />
            </el-icon>
            <span class="card-title">文件管理</span>
          </div>
          <el-button size="small" :disabled="!isConnected || fileOpLoading" @click="refreshFileList"
            :loading="fileOpLoading" text>
            <el-icon>
              <Refresh />
            </el-icon>
            <span class="btn-text">刷新</span>
          </el-button>
        </div>
      </template>

      <!-- 文件上传区域 -->
      <div class="file-upload-section">
        <div class="drop-zone" :class="{ 'drag-over': isDragOver, 'has-file': selectedFile, 'disabled': !isConnected }"
          @dragover.prevent="isConnected && (isDragOver = true)" @dragleave.prevent="isDragOver = false"
          @drop.prevent="isConnected && handleFileDrop($event)" @click="isConnected && triggerFileSelect()">
          <div v-if="selectedFile" class="file-preview">
            <div class="file-preview-content">
              <el-icon class="file-icon">
                <Document />
              </el-icon>
              <div class="file-info">
                <div class="file-name">{{ selectedFile.name }}</div>
                <div class="file-size">{{ formatFileSize(selectedFile.size) }}</div>
              </div>
            </div>
            <div class="file-actions">
              <el-button type="primary" size="large" :disabled="!isConnected || fileOpLoading" @click.stop="uploadFile"
                :loading="fileOpLoading">
                <el-icon>
                  <Upload />
                </el-icon>
                <span>上传文件</span>
              </el-button>
              <el-button size="large" @click.stop="selectedFile = null">
                <el-icon>
                  <Close />
                </el-icon>
                <span>取消</span>
              </el-button>
            </div>
          </div>
          <div v-else class="drop-hint">
            <el-icon class="upload-icon">
              <Upload />
            </el-icon>
            <p class="hint-title">{{ isConnected ? '点击选择文件' : '请先连接服务器' }}</p>
            <p class="hint-subtitle" v-if="isConnected">或拖拽文件到此处</p>
          </div>
        </div>
      </div>

      <!-- 文件列表 -->
      <div v-if="fileList.length > 0" class="file-list-section">
        <div class="section-title">
          <span>服务器文件</span>
          <span class="file-count">{{ fileList.length }} 个文件</span>
        </div>
        <div class="file-list">
          <div v-for="file in fileList" :key="file.name" class="file-item">
            <div class="file-item-info">
              <el-icon class="file-item-icon">
                <Document />
              </el-icon>
              <span class="file-item-name">{{ file.name }}</span>
            </div>
            <div class="file-item-actions">
              <el-button size="small" type="primary" :icon="Download" :disabled="!isConnected || fileOpLoading"
                @click="downloadFile(file.name)" circle />
              <el-button size="small" type="danger" :icon="Delete" :disabled="!isConnected || fileOpLoading"
                @click="deleteFile(file.name)" circle />
            </div>
          </div>
        </div>
      </div>
      <el-empty v-else description="暂无文件" :image-size="60" />
    </el-card>

    <!-- 消息日志卡片 -->
    <el-card class="message-card" shadow="never">
      <template #header>
        <div class="card-header">
          <div class="header-left">
            <el-icon class="header-icon">
              <ChatDotRound />
            </el-icon>
            <span class="card-title">消息日志</span>
          </div>
          <el-button size="small" :disabled="!receivedMessages" text @click="receivedMessages = ''">
            <el-icon>
              <Delete />
            </el-icon>
            <span class="btn-text">清空</span>
          </el-button>
        </div>
      </template>
      <div class="message-container">
        <el-input type="textarea" :rows="6" v-model="receivedMessages" readonly placeholder="等待接收消息..."
          class="message-textarea" />
      </div>
    </el-card>
  </div>
</template>

<script setup lang="ts">
import * as mqtt from "mqtt/dist/mqtt.min";
import { reactive, ref } from "vue";
import { ElNotification, ElMessageBox } from 'element-plus';
import { Download, Delete } from '@element-plus/icons-vue';

// ==================== 常量定义 ====================
const CHUNK_SIZE = 3072;

// ==================== 状态管理 ====================
const connection = reactive({
  protocol: "ws",
  host: "120.55.192.74",
  port: 8083,
  clientId: "emqx_vue3_" + Math.random().toString(16).substring(2, 8),
  username: "test",
  password: "123456",
  clean: true,
  connectTimeout: 30 * 1000,
  reconnectPeriod: 4000,
});

let client = ref<mqtt.MqttClient | null>(null);
const isConnected = ref(false);
const receivedMessages = ref("");
const btnLoadingType = ref("");
const configCollapsed = ref(false);

// 灯光和警报
const lights = reactive([
  { id: 1, name: "客厅", state: false, icon: "🛋️" },
  { id: 2, name: "餐厅", state: false, icon: "🍽️" },
  { id: 3, name: "卧室", state: false, icon: "🛏️" },
  { id: 4, name: "走廊", state: false, icon: "🚪" },
]);
const alarmControl = reactive({ state: false });

// 文件管理
const fileOpLoading = ref(false);
const selectedFile = ref<File | null>(null);
const fileList = ref<{ name: string }[]>([]);
const isDragOver = ref(false);
let downloadBuffers: Record<string, Uint8Array[]> = {};

// ==================== MQTT 连接管理 ====================
const createConnection = () => {
  try {
    btnLoadingType.value = "connect";
    const { protocol, host, port, ...options } = connection;
    const connectUrl = `${protocol}://${host}:${port}/mqtt`;
    client.value = mqtt.connect(connectUrl, options);

    client.value.on("connect", () => {
      isConnected.value = true;
      btnLoadingType.value = "";
      configCollapsed.value = true;
      ElNotification({ title: '成功', message: 'MQTT 连接成功', type: 'success', duration: 2000 });

      ["smart_home/light", "smart_home/alarm", "smart_home/file"].forEach(doSubscribe);
      refreshFileList();
      queryDeviceStatus(); // 连接成功后查询设备状态
    });

    client.value.on("error", (error) => {
      isConnected.value = false;
      btnLoadingType.value = "";
      console.error("连接错误:", error);
      ElNotification({ title: '错误', message: 'MQTT 连接失败', type: 'error', duration: 3000 });
    });

    client.value.on("message", handleMessage);
  } catch (error) {
    btnLoadingType.value = "";
    console.error("连接异常:", error);
  }
};

const destroyConnection = () => {
  if (!client.value) return;
  btnLoadingType.value = "disconnect";
  client.value.end(false, () => {
    isConnected.value = false;
    btnLoadingType.value = "";
    configCollapsed.value = false;
    ElNotification({ title: '通知', message: '已断开连接', type: 'info', duration: 2000 });
  });
};

const doSubscribe = (topic: string) => {
  client.value?.subscribe(topic, { qos: 1 }, (error) => {
    if (error) console.error(`订阅 ${topic} 失败:`, error);
  });
};

const doPublish = (topic: string, payload: string) => {
  client.value?.publish(topic, payload, { qos: 1 });
};

// ==================== 消息处理 ====================
const handleMessage = (topic: string, message: Buffer) => {
  const msgString = message.toString();
  const timestamp = new Date().toLocaleTimeString();
  receivedMessages.value = `[${timestamp}] [${topic}] ${msgString}\n${receivedMessages.value}`;

  if (topic === "smart_home/file") {
    try {
      const msgObj = JSON.parse(msgString);
      handleFileMessage(msgObj);
    } catch (e) {
      console.error("解析文件消息失败:", e);
    }
  } else if (topic === "smart_home/light") {
    try {
      const msgObj = JSON.parse(msgString);
      handleLightMessage(msgObj);
    } catch (e) {
      console.error("解析灯光消息失败:", e);
    }
  } else if (topic === "smart_home/alarm") {
    try {
      const msgObj = JSON.parse(msgString);
      handleAlarmMessage(msgObj);
    } catch (e) {
      console.error("解析报警消息失败:", e);
    }
  }
};

// ==================== 灯光和报警消息处理 ====================
const handleLightMessage = (msgObj: any) => {
  if (msgObj.command === "list_response" && msgObj.status === "ok") {
    // 更新灯光状态
    msgObj.lights.forEach((lightData: any) => {
      const light = lights.find(l => l.id === lightData.id);
      if (light) {
        light.state = lightData.state === "ON";
        // 可以扩展更多属性如亮度、色温等
      }
    });
  }
};

const handleAlarmMessage = (msgObj: any) => {
  if (msgObj.command === "list_response" && msgObj.status === "ok") {
    // 更新报警器状态
    alarmControl.state = msgObj.state === "ON";
  }
};

// ==================== 设备状态查询 ====================
const queryDeviceStatus = () => {
  // 查询灯光状态
  doPublish("smart_home/light", JSON.stringify({ command: "list" }));

  // 查询报警器状态
  doPublish("smart_home/alarm", JSON.stringify({ command: "list" }));
};

const queryLightStatus = () => {
  // 查询灯光状态
  doPublish("smart_home/light", JSON.stringify({ command: "list" }));
};

const queryAlarmStatus = () => {
  // 查询报警器状态
  doPublish("smart_home/alarm", JSON.stringify({ command: "list" }));
};

// ==================== 设备控制 ====================
const toggleLight = (light: { id: number; state: boolean }) => {
  if (!isConnected.value) return;
  light.state = !light.state;
  handleSingleLightControl(light);
};

const toggleAlarm = () => {
  if (!isConnected.value) return;
  alarmControl.state = !alarmControl.state;
  handleAlarmControl();
};

const handleSingleLightControl = (light: { id: number; state: boolean }) => {
  doPublish("smart_home/light", JSON.stringify({
    led: light.id,
    state: light.state ? "ON" : "OFF"
  }));
};

const handleAlarmControl = () => {
  doPublish("smart_home/alarm", JSON.stringify({
    state: alarmControl.state ? "ON" : "OFF"
  }));
};

const toggleAllLights = (state: boolean) => {
  lights.forEach(light => {
    if (light.state !== state) {
      light.state = state;
      handleSingleLightControl(light);
    }
  });
};

// ==================== 文件操作 ====================
const handleFileMessage = (msgObj: any) => {
  const handlers: Record<string, Function> = {
    list_response: handleListResponse,
    get_response: handleDownloadChunk,
    put_response: handleUploadResponse,
    delete_response: handleDeleteResponse,
  };
  handlers[msgObj.command]?.(msgObj);
};

const handleListResponse = (msgObj: any) => {
  fileOpLoading.value = false;
  if (msgObj.status === "ok") {
    fileList.value = (msgObj.files || []).map((name: string) => ({ name }));
  } else {
    ElNotification({ title: '错误', message: '获取文件列表失败', type: 'error', duration: 2000 });
  }
};

const handleUploadResponse = (msgObj: any) => {
  fileOpLoading.value = false;
  if (msgObj.status === "ok") {
    ElNotification({ title: '成功', message: '文件上传成功', type: 'success', duration: 2000 });
    selectedFile.value = null;
    refreshFileList();
  } else {
    ElNotification({ title: '失败', message: msgObj.message || '上传失败', type: 'error', duration: 3000 });
  }
};

const handleDeleteResponse = (msgObj: any) => {
  fileOpLoading.value = false;
  if (msgObj.status === "ok") {
    ElNotification({ title: '成功', message: '文件已删除', type: 'success', duration: 2000 });
    refreshFileList();
  } else {
    ElNotification({ title: '失败', message: '删除失败', type: 'error', duration: 2000 });
  }
};

const handleDownloadChunk = (msgObj: any) => {
  if (msgObj.status !== 'ok') {
    fileOpLoading.value = false;
    ElNotification({ title: '下载失败', message: msgObj.message || '文件不存在', type: 'error', duration: 3000 });
    delete downloadBuffers[msgObj.filename];
    return;
  }

  const filename = msgObj.filename;
  if (!downloadBuffers[filename]) downloadBuffers[filename] = [];

  const byteString = atob(msgObj.payload);
  const byteArray = new Uint8Array(byteString.length);
  for (let i = 0; i < byteString.length; i++) {
    byteArray[i] = byteString.charCodeAt(i);
  }
  downloadBuffers[filename].push(byteArray);

  if (msgObj.eof) {
    const parts = downloadBuffers[filename].map(u8 => u8.slice().buffer);
    const blob = new Blob(parts);
    const link = document.createElement('a');
    link.href = URL.createObjectURL(blob);
    link.download = filename;
    document.body.appendChild(link);
    link.click();
    document.body.removeChild(link);
    URL.revokeObjectURL(link.href);

    delete downloadBuffers[filename];
    fileOpLoading.value = false;
    ElNotification({ title: '成功', message: '文件下载完成', type: 'success', duration: 2000 });
  }
};

const refreshFileList = () => {
  if (!isConnected.value) return;
  fileOpLoading.value = true;
  doPublish("smart_home/file", JSON.stringify({ command: "list" }));
};

const uploadFile = async () => {
  if (!selectedFile.value) return;

  fileOpLoading.value = true;
  const file = selectedFile.value;
  const arrayBuffer = await file.arrayBuffer();
  const totalChunks = Math.ceil(arrayBuffer.byteLength / CHUNK_SIZE);

  for (let i = 0; i < totalChunks; i++) {
    const offset = i * CHUNK_SIZE;
    const chunk = arrayBuffer.slice(offset, offset + CHUNK_SIZE);
    const base64Chunk = btoa(String.fromCharCode(...new Uint8Array(chunk)));

    doPublish("smart_home/file", JSON.stringify({
      command: "put",
      filename: file.name,
      payload: base64Chunk,
      seq: i,
      eof: i === totalChunks - 1
    }));
  }
};

const downloadFile = (fileName: string) => {
  fileOpLoading.value = true;
  doPublish("smart_home/file", JSON.stringify({ command: "get", filename: fileName }));
};

const deleteFile = (fileName: string) => {
  ElMessageBox.confirm(`确定要删除文件 "${fileName}" 吗？`, '确认删除', {
    confirmButtonText: '删除',
    cancelButtonText: '取消',
    type: 'warning',
  }).then(() => {
    fileOpLoading.value = true;
    doPublish("smart_home/file", JSON.stringify({ command: "delete", filename: fileName }));
  }).catch(() => { });
};

// ==================== UI 辅助函数 ====================
const handleFileDrop = (e: DragEvent) => {
  isDragOver.value = false;
  if (e.dataTransfer?.files?.[0]) {
    selectedFile.value = e.dataTransfer.files[0];
  }
};

const triggerFileSelect = () => {
  const input = document.createElement('input');
  input.type = 'file';
  input.onchange = (e) => {
    const target = e.target as HTMLInputElement;
    if (target.files?.[0]) selectedFile.value = target.files[0];
  };
  input.click();
};

const formatFileSize = (bytes: number): string => {
  if (bytes === 0) return '0 Bytes';
  const k = 1024;
  const sizes = ['Bytes', 'KB', 'MB', 'GB'];
  const i = Math.floor(Math.log(bytes) / Math.log(k));
  return `${(bytes / Math.pow(k, i)).toFixed(2)} ${sizes[i]}`;
};
</script>

<style scoped src="./MqttDemo.css"></style>